#pragma once

#include "Utility\FilePath.hpp"

namespace Hail
{
	class InOutStream;

	//Resource that gets a filewatcher imported from non engine resources
	class MetaResource : public SerializeableObjectCustom
	{
	public:
		MetaResource() = default;
		// Constructs a GUID as well as the relative fileobjects
		void ConstructResourceAndID(const FilePath& sourcePath, const FilePath& resourceProjectPath);
		void Serialize(InOutStream& stream) final;
		void Deserialize(InOutStream& stream) final;

		//The path of the original resource, so if a texture gets imported this is the source file
		const RelativeFilePath& GetSourceFilePath() const { return m_sourceFilePath; }
		//The path of the game resource
		const RelativeFilePath& GetProjectFilePath() const { return m_projectFilePath; }
		// The Global Unique Identifier of the resource
		const GUID& GetGUID() const { return m_ID; }
		const CommonFileData& GetFileData() const { return m_fileOwningCommonData; }
	private:
		GUID m_ID;
		RelativeFilePath m_sourceFilePath;
		RelativeFilePath m_projectFilePath;
		CommonFileData m_fileOwningCommonData;
	};
}