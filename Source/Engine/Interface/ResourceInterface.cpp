#include "Engine_PCH.h"
#include "ResourceInterface.h"

#include "Resources\ResourceManager.h"

namespace Hail
{
	ResourceInterface* ResourceInterface::m_instance = nullptr;



	ResourceInterface::ResourceInterface(ResourceManager& resourceManager) : m_resourceManager(resourceManager)
	{
	}

	void ResourceInterface::LoadMaterialInstanceResource(GUID guid)
	{
		if (!m_instance)
			return;
		ResourceInterface& interface = *m_instance;

		interface.m_resourceManager.LoadMaterialResource(guid);
	}

	uint32 ResourceInterface::GetMaterialInstanceResourceHandle(GUID guid)
	{
		if (!m_instance || guid == GuidZero)
			return MAX_UINT;
		ResourceInterface& interface = *m_instance;

		return interface.m_resourceManager.GetMaterialInstanceHandle(guid);
	}


	void ResourceInterface::InitializeResourceInterface(ResourceManager& resourceManager)
	{
		if (m_instance)
			return;

		m_instance = new ResourceInterface(resourceManager);
	}

}

