#include "ResourceCompiler_PCH.h"
#include "ShaderCompiler.h"

#include "shaderc/shaderc.hpp"

#include "DebugMacros.h"

#include "Utility\FileSystem.h"
#include "Utility\StringUtility.h"
#include "Utility\InOutStream.h"

namespace Hail
{

	bool LocalCompileShaderInternalGLSL(shaderc_compiler_t shaderCCompiler, eShaderType type, const char* shaderName, GrowingArray<char>& shaderData, shaderc_compile_options_t compileOptions, MetaResource metaResource);
	bool LocalCompileShaderInternalHLSL(const char* relativePath, const char* shaderName);
	bool LocalCompileShaderInternalMETAL(const char* relativePath, const char* shaderName);
	bool LocalExportCompiledShader(const char* shaderName, const char* compiledShaderData, ShaderHeader shaderHeader, MetaResource metaResource);

	bool ShaderCompiler::CompileSpecificShader(const char* shaderName, eShaderType shaderType)
	{
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		WString64 extension{};
		WString64 wShaderName;
		FromConstCharToWChar(shaderName, wShaderName, 64);
		switch (shaderType)
		{
		case Hail::eShaderType::Compute:
			extension = L"cmp";
			break;
		case Hail::eShaderType::Vertex:
			extension = L"vert";
			break;
		case Hail::eShaderType::Amplification:
			extension = L"amp";
			break;
		case Hail::eShaderType::Mesh:
			extension = L"msh";
			break;
		case Hail::eShaderType::Fragment:
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
		GrowingArray<char> readShader(stream.GetFileSize(), 0);
		stream.Read(readShader.Data(), stream.GetFileSize());
		//readShader[memblockSize] = '\0';//null terminating the string
		//DEBUG_PRINT_CONSOLE_CONSTCHAR(readShader.Data());
		MetaResource metaResource;
		metaResource.SetSourcePath(filePath);
		const bool result = LocalCompileShaderInternalGLSL(compiler, shaderType, shaderName, readShader, compileOptions, metaResource);

		shaderc_compile_options_release(compileOptions);
		shaderc_compiler_release(compiler);
		return result;
	}

	void ShaderCompiler::Init(SHADERLANGUAGETARGET shaderCompilationTarget)
	{
		m_languageToCompileTo = shaderCompilationTarget;
	}


	bool LocalCompileShaderInternalGLSL(shaderc_compiler_t shaderCCompiler, eShaderType type, const char* shaderName, GrowingArray<char>& shaderData, shaderc_compile_options_t compileOptions, MetaResource metaResource)
	{
		shaderc_compilation_result_t compiledShader{};
		switch (type)
		{
		case eShaderType::Compute:
			break;
		case eShaderType::Vertex:
			compiledShader = shaderc_compile_into_spv(shaderCCompiler, shaderData.Data(), shaderData.Size(), shaderc_shader_kind::shaderc_glsl_vertex_shader, shaderName, "main", compileOptions);
			break;
		case eShaderType::Amplification:
			break;
		case eShaderType::Mesh:
			break;
		case eShaderType::Fragment:
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
			result = LocalExportCompiledShader(shaderName, compiledShaderBytes, header, metaResource);
		}
		shaderc_result_release(compiledShader);
		return result;
	}


	bool LocalCompileShaderInternalHLSL(ShaderCompiler& compiler, const char* relativePath, const char* shaderName)
	{
		return false;
	}


	bool LocalCompileShaderInternalMETAL(ShaderCompiler& compiler, const char* relativePath, const char* shaderName)
	{
		return false;
	}


	bool LocalExportCompiledShader(const char* shaderName, const char* compiledShaderData, ShaderHeader shaderHeader, MetaResource metaResource)
	{
		{
			FilePath filePath(SHADER_DIR_OUT);
			filePath.CreateFileDirectory();
		}
		FilePath outPath = StringL::Format("%s%s%s", SHADER_DIR_OUT, shaderName, ".shr");

		InOutStream readStream;
		if (readStream.OpenFile(outPath, FILE_OPEN_TYPE::READ, true))
		{
			metaResource.Deserialize(readStream);
		}
		metaResource.ConstructResourceAndID("", outPath);

		InOutStream outStream;
		if (!outStream.OpenFile(outPath, FILE_OPEN_TYPE::WRITE, true))
		{
			return false;
		}
		metaResource.Serialize(outStream);
		outStream.Write((char*)&shaderHeader, sizeof(shaderHeader));
		for (uint32_t i = 0; i < shaderHeader.sizeOfShaderData; i++)
		{
			outStream.Write(&compiledShaderData[i], 1);
		}
		outStream.CloseFile();
		return true;
	}


	eShaderType ShaderCompiler::CheckShaderType(const char* shaderExtension)
	{
		if (strcmp(shaderExtension, "vert") == 0)
		{
			return eShaderType::Vertex;
		}
		else if (strcmp(shaderExtension, "frag") == 0)
		{
			return eShaderType::Fragment;
		}
		return eShaderType::None;
	}
}