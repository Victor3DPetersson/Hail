#include "Engine_PCH.h"

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


void CreateGUID(Hail::GUID& guidToCreate)
{
#ifdef PLATFORM_WINDOWS
	UUID newId;
	UuidCreate(&newId);
	memcpy(&guidToCreate, &newId, sizeof(Hail::GUID));
#endif
}

void MetaResource::ConstructResourceAndID(const FilePath& sourcePath, const FilePath& resourceProjectPath)
{
	m_sourceFilePath = RelativeFilePath(sourcePath);
	m_projectFilePath = RelativeFilePath(resourceProjectPath);
	//Create GUID
	m_fileOwningCommonData = sourcePath.Object().GetFileData();
	CreateGUID(m_ID);
}

void MetaResource::Serialize(InOutStream& stream)
{
	if (m_ID == guidZero)
	{
		CreateGUID(m_ID);
	}
	if (!stream.IsWriting())
			return;
	stream.Write(&m_ID, sizeof(GUID));
	m_sourceFilePath.Serialize(stream);
	m_projectFilePath.Serialize(stream);
	stream.Write((char*)&m_fileOwningCommonData, sizeof(m_fileOwningCommonData), 1);
}

void MetaResource::Deserialize(InOutStream& stream)
{
	if (!stream.IsReading())
		return;
	stream.Read(&m_ID, sizeof(GUID));
	m_sourceFilePath.Deserialize(stream);
	m_projectFilePath.Deserialize(stream);
	stream.Read((char*)&m_fileOwningCommonData, sizeof(m_fileOwningCommonData), 1);
}
