#include "ResourceCompiler_PCH.h"
#include "ShaderCompiler.h"

#include "shaderc/shaderc.hpp"

#include "DebugMacros.h"

#include "Utility\FileSystem.h"
#include "Utility\StringUtility.h"
#include "Utility\InOutStream.h"

namespace Hail
{

	bool LocalCompileShaderInternalGLSL(shaderc_compiler_t shaderCCompiler, eShaderStage type, const char* shaderName, GrowingArray<char>& shaderData, shaderc_compile_options_t compileOptions, MetaResource metaResource);
	bool LocalCompileShaderInternalHLSL(const char* relativePath, const char* shaderName);
	bool LocalCompileShaderInternalMETAL(const char* relativePath, const char* shaderName);
	bool LocalExportCompiledShader(const char* shaderName, const char* compiledShaderData, ShaderHeader shaderHeader, MetaResource metaResource);

	// TODO: add an error enum instead of a false statement
	bool localCheckForIncludes(GrowingArray<FileObject>& includesAdded, GrowingArray<char>& shaderDataToFill, const FilePath& newPathToCheck)
	{
		if (!newPathToCheck.IsValid())
			return false;

		FileObject fileObjectToCheck = newPathToCheck.Object();
		for (size_t i = 0; i < includesAdded.Size(); i++)
		{
			if (fileObjectToCheck == includesAdded[i])
				return false;
		}

		InOutStream stream;
		if (!stream.OpenFile(newPathToCheck, FILE_OPEN_TYPE::READ, false))
		{
			return false;
		}
		GrowingArray<char> readShader(stream.GetFileSize(), 0);
		stream.Read(readShader.Data(), stream.GetFileSize());
		stream.CloseFile();

		uint32 offsetToAddFrom = 0u;
		for (uint32 i = 0; i < readShader.Size(); i++)
		{
			if (readShader[i] != '#')
				continue;

			StringL line;
			bool bFoundEndLine = false;
			
			uint32 currentCharacterIndex = i;
			while (bFoundEndLine == false)
			{
				if (readShader[currentCharacterIndex] == '\n')
				{
					bFoundEndLine = true;
				}
				line += readShader[currentCharacterIndex];
				currentCharacterIndex++;
				if (currentCharacterIndex == readShader.Size())
					break;
			}
			H_ASSERT(bFoundEndLine);

			if (int includeStart = StringContainsAndEndsAtIndex(line.Data(), "#include ") != -1)
			{
				uint32 lengthOfEntireLine = line.Length();
				// remove "#include "";
				line = line.Data() + includeStart + 9u;
				H_ASSERT(line[line.Length() - 1u] == '\n');

				if (line[0] == '"')
					return false;
				if (line[line.Length() - 2u] != '"')
					return false;


				H_ASSERT(i >= offsetToAddFrom);

				// Add what is before the include to the outShaderData
				uint32 lengthToAdd = i - offsetToAddFrom;
				uint32 previousLength = shaderDataToFill.Size();
				if (lengthToAdd)
				{
					// TODO: gör ett test case i en shader där jag lägger till kod emellan två includes
					shaderDataToFill.AddN(lengthToAdd);
					memcpy(shaderDataToFill.Data() + previousLength, readShader.Data() + (offsetToAddFrom - 1u), lengthToAdd);
				}

				// make sure that the rest of the code gets added
				offsetToAddFrom += lengthOfEntireLine + lengthToAdd;
				// Remove new line and character
				line.RemoveCharsFromBack(2u);
				StringLW lineW = line;
				FilePath includePath = FilePath::GetShaderResourceDirectory();
				includePath = includePath + lineW.Data();
				if (!localCheckForIncludes(includesAdded, shaderDataToFill, includePath))
					return false;

				includesAdded.Add(includePath.Object());
				i += lengthOfEntireLine - 1u;
			}
		}
		// no more includes, add the rest of the code
		uint32 lengthToAdd = (readShader.Size() - offsetToAddFrom);
		uint32 previousLength = shaderDataToFill.Size();
		shaderDataToFill.AddN(lengthToAdd + 1u);
		memcpy(shaderDataToFill.Data() + previousLength, readShader.Data() + offsetToAddFrom, lengthToAdd);
		shaderDataToFill[shaderDataToFill.Size() - 1u] = '\n';
		return true;
	}

	bool ShaderCompiler::CompileSpecificShader(const char* shaderName, eShaderStage shaderType)
	{
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		WString64 extension{};
		WString64 wShaderName;
		FromConstCharToWChar(shaderName, wShaderName, 64);
		H_ASSERT(shaderType != eShaderStage::None, "incorrect shader type");
		switch (shaderType)
		{
		case Hail::eShaderStage::Compute:
			extension = L"cmp";
			break;
		case Hail::eShaderStage::Vertex:
			extension = L"vert";
			break;
		case Hail::eShaderStage::Amplification:
			extension = L"amp";
			break;
		case Hail::eShaderStage::Mesh:
			extension = L"mesh";
			break;
		case Hail::eShaderStage::Fragment:
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
		shaderc_compile_options_set_target_env(compileOptions, shaderc_target_env::shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

		InOutStream stream;
		if (!stream.OpenFile(filePath, FILE_OPEN_TYPE::READ, false))
		{
			return false;
		}
		GrowingArray<char> readShader(stream.GetFileSize());
		stream.CloseFile();

		GrowingArray<FileObject> includesAdded;
		if (!localCheckForIncludes(includesAdded, readShader, filePath))
			return false;

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


	bool LocalCompileShaderInternalGLSL(shaderc_compiler_t shaderCCompiler, eShaderStage type, const char* shaderName, GrowingArray<char>& shaderData, shaderc_compile_options_t compileOptions, MetaResource metaResource)
	{
		shaderc_compilation_result_t compiledShader{};
		switch (type)
		{
		case eShaderStage::Compute:
			compiledShader = shaderc_compile_into_spv(shaderCCompiler, shaderData.Data(), shaderData.Size(), shaderc_shader_kind::shaderc_glsl_compute_shader, shaderName, "main", compileOptions);
			break;
		case eShaderStage::Vertex:
			compiledShader = shaderc_compile_into_spv(shaderCCompiler, shaderData.Data(), shaderData.Size(), shaderc_shader_kind::shaderc_glsl_vertex_shader, shaderName, "main", compileOptions);
			break;
		case eShaderStage::Amplification:
			break;
		case eShaderStage::Mesh:
			compiledShader = shaderc_compile_into_spv(shaderCCompiler, shaderData.Data(), shaderData.Size(), shaderc_shader_kind::shaderc_glsl_mesh_shader, shaderName, "main", compileOptions);
			break;
		case eShaderStage::Fragment:
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
			StringL line;
			uint32 currentLineNumb = 1u;
			for (uint32 i = 0; i < shaderData.Size(); i++)
			{
				line += StringL::Format("%u: ", currentLineNumb);
				bool bFoundEndLine = false;

				uint32 currentCharacterIndex = i;
				while (bFoundEndLine == false)
				{
					if (shaderData[currentCharacterIndex] == '\n')
					{
						bFoundEndLine = true;
					}
					else
					{
						line += shaderData[currentCharacterIndex];
						currentCharacterIndex++;
						if (currentCharacterIndex == shaderData.Size())
							break;
					}
				}
				Debug_PrintConsoleConstChar(line.Data());
				currentLineNumb++;
				i = currentCharacterIndex;
				line = StringL();
			}

			H_ERROR(shaderc_result_get_error_message(compiledShader));
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
		metaResource.ConstructResourceAndID("", outPath, metaResource.GetGUID());

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


	eShaderStage ShaderCompiler::CheckShaderType(const char* shaderExtension)
	{
		if (strcmp(shaderExtension, "vert") == 0)
		{
			return eShaderStage::Vertex;
		}
		else if (strcmp(shaderExtension, "frag") == 0)
		{
			return eShaderStage::Fragment;
		}
		else if (strcmp(shaderExtension, "mesh") == 0)
		{
			return eShaderStage::Mesh;
		}
		return eShaderStage::None;
	}
}