//Interface for the entire engine
#pragma once
#include "Types.h"

namespace Hail
{
	class ResourceManager;

	class ResourceInterface
	{
	public:
		explicit ResourceInterface(ResourceManager& resourceManager);

		static void LoadMaterialInstanceResource(GUID guid);
		static uint32 GetMaterialInstanceResourceHandle(GUID guid);

		static void InitializeResourceInterface(ResourceManager& resourceManager);

		static eResourceState GetMaterialResourceState(GUID guid);

	private:
		static ResourceInterface* m_instance;
		ResourceManager& m_resourceManager;
	};
}

