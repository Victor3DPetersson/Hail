#include "ResourceCompiler_PCH.h"
#include "ShaderCompiler.h"

#include "shaderc/shaderc.hpp"

#include "DebugMacros.h"

#include "Utility\FileSystem.h"
#include "Utility\StringUtility.h"
#include "Utility\InOutStream.h"

namespace Hail
{

	void ShaderCompiler::CompileAllShaders()
	{
#ifdef NDEBUG
		shaderc_compiler_t compiler = shaderc_compiler_initialize();




		shaderc_compiler_release(compiler);
#endif // DEBUG
	}

	bool CompileShaderInternalGLSL(shaderc_compiler_t shaderCCompiler, SHADERTYPE type, const char* shaderName, GrowingArray<char>& shaderData, shaderc_compile_options_t compileOptions);
	bool CompileShaderInternalHLSL(const char* relativePath, const char* shaderName);
	bool CompileShaderInternalMETAL(const char* relativePath, const char* shaderName);
	void ExportCompiledShader(const char* shaderName, const char* compiledShaderData, ShaderHeader shaderHeader);

	bool ShaderCompiler::CompileAndExportAllRequiredShaders(const char** requiredShaders, uint32_t numberOfRequiredShaders)
	{
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		RecursiveFileIterator pathToShow = RecursiveFileIterator(SHADER_DIR_IN);
		String256 fileName;
		GrowingArray<FilePath> shadersToCompile = GrowingArray<FilePath>(numberOfRequiredShaders);
		while (pathToShow.IterateOverFolder()) 
		{
			FilePath currentPath = pathToShow.GetCurrentPath();
			if (currentPath.IsFile())
			{
				for (uint32_t i = 0; i < numberOfRequiredShaders; i++)
				{
					FromWCharToConstChar(currentPath.Object().Name(), fileName, 256);
					if (StringCompare(requiredShaders[i], fileName) 
						&& StringCompare(currentPath.Object().Extension(), L"frag")
						|| StringCompare(currentPath.Object().Extension(), L"vert"))
					{
						//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
						shadersToCompile.Add(currentPath);
					}
				}
			}
		}
		if (shadersToCompile.Size() < numberOfRequiredShaders)
		{
			return false;
		}


		shaderc_compile_options_t compileOptions = shaderc_compile_options_initialize();
		shaderc_compile_options_set_target_env(compileOptions, shaderc_target_env::shaderc_target_env_vulkan, 0);
		for (uint32_t i = 0; i < numberOfRequiredShaders; i++)
		{
			InOutStream stream;
			if (!stream.OpenFile(shadersToCompile[i], FILE_OPEN_TYPE::READ, false))
			{
				continue;
			}
			GrowingArray<char> readShader;
			readShader.InitAndFill(stream.GetFileSize());
			stream.Read(readShader.Data(), stream.GetFileSize());
			String64 shaderName;
			FromWCharToConstChar(shadersToCompile[i].Object().Name(), shaderName, 64);
			String64 extension;
			FromWCharToConstChar(shadersToCompile[i].Object().Extension(), extension, 64);
			if (!CompileShaderInternalGLSL(compiler, CheckShaderType(extension), shaderName.Data(), readShader, compileOptions))
			{
				shaderc_compile_options_release(compileOptions);
				shaderc_compiler_release(compiler);
				return false;
			}

		}
		shaderc_compile_options_release(compileOptions);

		shaderc_compiler_release(compiler);
		return true;
	}

	bool ShaderCompiler::CompileSpecificShader(const char* shaderName, SHADERTYPE shaderType)
	{
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		WString64 extension{};
		WString64 wShaderName;
		FromConstCharToWChar(shaderName, wShaderName, 64);
		switch (shaderType)
		{
		case Hail::SHADERTYPE::COMPUTE:
			extension = L"cmp";
			break;
		case Hail::SHADERTYPE::VERTEX:
			extension = L"vert";
			break;
		case Hail::SHADERTYPE::CONTROL:
			extension = L"ctrl";
			break;
		case Hail::SHADERTYPE::EVALUATION:
			extension = L"evl";
			break;
		case Hail::SHADERTYPE::FRAGMENT:
			extension = L"frag";
			break;
		default:
			break;
		}
		FilePath filePath{};
		RecursiveFileIterator pathToShow = RecursiveFileIterator(SHADER_DIR_IN);
		
		while(pathToShow.IterateOverFolderRecursively())
		{
			FilePath currentPath = pathToShow.GetCurrentPath();
			if (currentPath.IsFile())
			{
				if (StringCompare(wShaderName, currentPath.Object().Name()))
				{
					//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
					if (StringCompare(currentPath.Object().Extension(), extension))
					{
						filePath = currentPath;
						break;
					}
				}
			}
		}
		if (filePath.IsEmpty())
		{
			return false;
		}

		shaderc_compile_options_t compileOptions = shaderc_compile_options_initialize();
		shaderc_compile_options_set_target_env(compileOptions, shaderc_target_env::shaderc_target_env_vulkan, 0);

		InOutStream stream;
		if (!stream.OpenFile(filePath, FILE_OPEN_TYPE::READ, false))
		{
			return false;
		}
		GrowingArray<char> readShader;
		readShader.InitAndFill(stream.GetFileSize());
		stream.Read(readShader.Data(), stream.GetFileSize());
		//readShader[memblockSize] = '\0';//null terminating the string
		//DEBUG_PRINT_CONSOLE_CONSTCHAR(readShader.Data());

		const bool result = CompileShaderInternalGLSL(compiler, shaderType, shaderName, readShader, compileOptions);

		shaderc_compile_options_release(compileOptions);
		shaderc_compiler_release(compiler);
		return result;
	}


	bool ShaderCompiler::IsShaderCompiled(const char* relativePath, const char* shaderName)
	{
		return false;
	}


	void ShaderCompiler::Init(SHADERLANGUAGETARGET shaderCompilationTarget)
	{
		m_languageToCompileTo = shaderCompilationTarget;
	}


	bool CompileShaderInternalGLSL(shaderc_compiler_t shaderCCompiler, SHADERTYPE type, const char* shaderName, GrowingArray<char>& shaderData, shaderc_compile_options_t compileOptions)
	{
		shaderc_compilation_result_t compiledShader{};
		switch (type)
		{
		case SHADERTYPE::COMPUTE:
			break;
		case SHADERTYPE::VERTEX:
			compiledShader = shaderc_compile_into_spv(shaderCCompiler, shaderData.Data(), shaderData.Size(), shaderc_shader_kind::shaderc_glsl_vertex_shader, shaderName, "main", compileOptions);
			break;
		case SHADERTYPE::CONTROL:
			break;
		case SHADERTYPE::EVALUATION:
			break;
		case SHADERTYPE::FRAGMENT:
			compiledShader = shaderc_compile_into_spv(shaderCCompiler, shaderData.Data(), shaderData.Size(), shaderc_shader_kind::shaderc_glsl_fragment_shader, shaderName, "main", compileOptions);
			break;
		default:
			break;
		}
		bool result = false;
		uint32_t numberOfErrors = shaderc_result_get_num_errors(compiledShader);
		shaderc_compilation_status status = shaderc_result_get_compilation_status(compiledShader);
		if (numberOfErrors > 0 && status != shaderc_compilation_status_success)
		{
			Debug_PrintConsoleConstChar(shaderc_result_get_error_message(compiledShader));
		}
		else
		{
			ShaderHeader header;
			const char* compiledShaderBytes = shaderc_result_get_bytes(compiledShader);
			header.sizeOfShaderData = shaderc_result_get_length(compiledShader);
			header.shaderType = static_cast<uint32_t>(type);
			ExportCompiledShader(shaderName, compiledShaderBytes, header);
			result = true;
		}
		shaderc_result_release(compiledShader);
		return result;
	}


	bool CompileShaderInternalHLSL(ShaderCompiler& compiler, const char* relativePath, const char* shaderName)
	{
		return false;
	}


	bool CompileShaderInternalMETAL(ShaderCompiler& compiler, const char* relativePath, const char* shaderName)
	{
		return false;
	}


	void ExportCompiledShader(const char* shaderName, const char* compiledShaderData, ShaderHeader shaderHeader)
	{
		Debug_PrintConsoleString256(String256::Format("\nExporting Shader:\n%s:", shaderName));
		Debug_PrintConsoleString256(String256::Format("Shader Size:%i:%s", shaderHeader.sizeOfShaderData, "\n"));
		{
			FilePath filePath(SHADER_DIR_OUT);
			filePath.CreateFileDirectory();
		}
		FilePath outPath = String256::Format("%s%s%s", SHADER_DIR_OUT, shaderName, ".shr");

		InOutStream outStream;
		if (!outStream.OpenFile(outPath, FILE_OPEN_TYPE::WRITE, true))
		{
			return;
		}
		outStream.Write((char*)&shaderHeader, sizeof(shaderHeader));
		for (uint32_t i = 0; i < shaderHeader.sizeOfShaderData; i++)
		{
			outStream.Write(&compiledShaderData[i], 1);
			std::cout << compiledShaderData[i];
		}
		std::cout << std::endl;
		outStream.CloseFile();
	}


	SHADERTYPE ShaderCompiler::CheckShaderType(const char* shaderExtension)
	{
		if (strcmp(shaderExtension, ".vert") == 0)
		{
			return SHADERTYPE::VERTEX;
		}
		else if (strcmp(shaderExtension, ".frag") == 0)
		{
			return SHADERTYPE::FRAGMENT;
		}
		return SHADERTYPE();
	}
}