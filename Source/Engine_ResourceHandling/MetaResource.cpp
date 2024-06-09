#include "ResourceCompiler_PCH.h"

#include "MetaResource.h"
#include "Utility\InOutStream.h"

#include "Reflection\Reflection.h"

#ifdef PLATFORM_WINDOWS

#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")
//#elif PLATFORM_OSX//.... more to be added

#endif

using namespace Hail;

namespace Hail::Reflection
{
	DEFINE_TYPE_CUSTOM_SERIALIZER(MetaResource)
}


void LocalCreateGUID(Hail::GUID& guidToCreate)
{
#ifdef PLATFORM_WINDOWS
	UUID newId;
	UuidCreate(&newId);
	memcpy(&guidToCreate, &newId, sizeof(Hail::GUID));
#endif
}

void MetaResource::ConstructResourceAndID(const FilePath& sourcePath, const FilePath& resourceProjectPath)
{
	// TODO assert on paths if they not valid

	if (m_sourceFilePath.Empty() && sourcePath.IsValid())
	{
		m_sourceFilePath = RelativeFilePath(sourcePath);
		m_sourceFileOwningCommonData = sourcePath.Object().GetFileData();
	}
	else if (!sourcePath.IsEmpty() && sourcePath.IsValid())
	{
		FilePath sourceFilePath = m_sourceFilePath.GetFilePath();
		if (sourcePath != sourceFilePath)
		{
			m_sourceFilePath = RelativeFilePath(sourcePath);
		}
	}

	if (!m_sourceFilePath.Empty())
	{
		m_sourceFileOwningCommonData = m_sourceFilePath.GetFilePath().Object().GetFileData();
	}

	if (m_projectFilePath.Empty())
		m_projectFilePath = RelativeFilePath(resourceProjectPath);
	else
	{
		FilePath projectFilePath = m_projectFilePath.GetFilePath();
		if (resourceProjectPath != projectFilePath)
			m_projectFilePath = RelativeFilePath(resourceProjectPath);
	}
	//Create GUID
	if (m_ID == GuidZero)
	{
		LocalCreateGUID(m_ID);
	}
	// TODO: assert if no project name
	m_name = m_projectFilePath.GetName();

}

void Hail::MetaResource::SetSourcePath(const FilePath& sourcePath)
{
	m_sourceFilePath = RelativeFilePath(sourcePath);
	m_sourceFileOwningCommonData = sourcePath.Object().GetFileData();
}

void Hail::MetaResource::SetProjectPath(const FilePath& sourcePath)
{
	m_projectFilePath = RelativeFilePath(sourcePath);
	m_name = m_projectFilePath.GetName();
}

void MetaResource::Serialize(InOutStream& stream)
{
	if (m_ID == GuidZero)
	{
		LocalCreateGUID(m_ID);
	}
	if (!stream.IsWriting())
			return;
	stream.Write(&m_ID, sizeof(GUID));
	m_sourceFilePath.Serialize(stream);
	m_projectFilePath.Serialize(stream);
	stream.Write((char*)&m_sourceFileOwningCommonData, sizeof(m_sourceFileOwningCommonData), 1);
}

void MetaResource::Deserialize(InOutStream& stream)
{
	if (!stream.IsReading())
		return;
	stream.Read(&m_ID, sizeof(GUID));
	m_sourceFilePath.Deserialize(stream);
	m_projectFilePath.Deserialize(stream);
	stream.Read((char*)&m_sourceFileOwningCommonData, sizeof(m_sourceFileOwningCommonData), 1);
	m_name = m_projectFilePath.GetName();
}
