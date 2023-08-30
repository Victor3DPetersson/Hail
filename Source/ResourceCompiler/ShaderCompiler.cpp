#include "ResourceCompiler_PCH.h"
#include "ShaderCompiler.h"

#include "shaderc/shaderc.hpp"

#include <filesystem>

#include "DebugMacros.h"

#include <iostream>
#include <fstream>

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
		std::filesystem::path pathToShow{ SHADER_DIR_IN };
		Debug_PrintConsoleString256(String256(pathToShow.string().c_str()));
		GrowingArray<std::filesystem::directory_entry> shadersToCompile = GrowingArray<std::filesystem::directory_entry>(numberOfRequiredShaders);
		for (auto& entry : std::filesystem::directory_iterator(pathToShow)) {
			const auto filenameStr = entry.path().filename().replace_extension().string();
			if (entry.is_directory())
			{
				//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tdir:  ", filenameStr));
			}
			else if (entry.is_regular_file())
			{
				for (uint32_t i = 0; i < numberOfRequiredShaders; i++)
				{
					if (strcmp(requiredShaders[i], filenameStr.c_str()) == 0)
					{
						//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
						const String64 extension = entry.path().extension().string().c_str();
						if (extension == String64(".frag") || extension == String64(".vert"))
						{
							shadersToCompile.Add(entry);
						}
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
			const String256 shaderName = shadersToCompile[i].path().filename().replace_extension().string();
			const String64 shaderExtension = shadersToCompile[i].path().extension().string();

			uint32_t memblockSize = 0;
			std::ifstream stream(shadersToCompile[i].path().string().c_str());
			if (!stream)
			{
				continue;
			}
			GrowingArray<char> readShader;
			std::string line;
			stream.seekg(0);
			char character = 1;
			uint32_t sizeofchar = sizeof(char);
			while (stream.peek() != EOF)
			{
				stream.read(&character, sizeof(char));
				memblockSize++;
			}
			stream.clear();
			stream.seekg(0);
			readShader.InitAndFill(memblockSize);
			stream.read(readShader.Data(), memblockSize);
			//readShader[memblockSize] = '\0';//null terminating the string
			//DEBUG_PRINT_CONSOLE_CONSTCHAR(readShader.Data());

			if (!CompileShaderInternalGLSL(compiler, CheckShaderType(shaderExtension), shaderName.Data(), readShader, compileOptions))
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
		std::filesystem::path pathToShow{ SHADER_DIR_IN };
		Debug_PrintConsoleString256(String256(pathToShow.string().c_str()));
		String64 extension{};
		switch (shaderType)
		{
		case Hail::SHADERTYPE::COMPUTE:
			extension = ".cmp";
			break;
		case Hail::SHADERTYPE::VERTEX:
			extension = ".vert";
			break;
		case Hail::SHADERTYPE::CONTROL:
			extension = ".ctrl";
			break;
		case Hail::SHADERTYPE::EVALUATION:
			extension = ".evl";
			break;
		case Hail::SHADERTYPE::FRAGMENT:
			extension = ".frag";
			break;
		default:
			break;
		}
		String256 filePath{};
		
		for (auto& entry : std::filesystem::directory_iterator(pathToShow)) {
			const auto filenameStr = entry.path().filename().replace_extension().string();
			if (entry.is_directory())
			{
				//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tdir:  ", filenameStr));
			}
			else if (entry.is_regular_file())
			{
				if (strcmp(shaderName, filenameStr.c_str()) == 0)
				{
					//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
					const String64 fileExtension = entry.path().extension().string().c_str();
					if (fileExtension == extension)
					{
						filePath = entry.path().string();
						break;
					}
				}
			}
		}
		if (filePath.Empty())
		{
			return false;
		}

		shaderc_compile_options_t compileOptions = shaderc_compile_options_initialize();
		shaderc_compile_options_set_target_env(compileOptions, shaderc_target_env::shaderc_target_env_vulkan, 0);

		uint32_t memblockSize = 0;
		std::ifstream stream(filePath);
		if (!stream)
		{
			return false;
		}
		GrowingArray<char> readShader;
		std::string line;
		stream.seekg(0);
		char character = 1;
		uint32_t sizeofchar = sizeof(char);
		while (stream.peek() != EOF)
		{
			stream.read(&character, sizeof(char));
			memblockSize++;
		}
		stream.clear();
		stream.seekg(0);
		readShader.InitAndFill(memblockSize);
		stream.read(readShader.Data(), memblockSize);
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

		if (std::filesystem::exists(SHADER_DIR_OUT) == false)
		{
			std::filesystem::create_directory(SHADER_DIR_OUT);
		}

		String256 outPath = String256::Format("%s%s%s", SHADER_DIR_OUT, shaderName, ".shr");

		std::ofstream outStream(outPath.Data(), std::ios::out | std::ios::binary);

		outStream.write((char*)&shaderHeader, sizeof(shaderHeader));
		for (uint32_t i = 0; i < shaderHeader.sizeOfShaderData; i++)
		{
			outStream.write(&compiledShaderData[i], 1);
			std::cout << compiledShaderData[i];
		}
		std::cout << std::endl;
		outStream.close();
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