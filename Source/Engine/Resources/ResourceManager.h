#pragma once

#include "MeshResources.h"
#include "TextureManager.h"
#include "ShaderManager.h"

#ifdef PLATFORM_WINDOWS
#include "windows/VulkanInternal/VlkResourceManager.h"
#endif


namespace Hail
{
	class RenderingDevice;
	class ResourceManager
	{
	public:
		ResourceManager();
		bool InitResources(RenderingDevice* renderingDevice);
		ShaderManager& GetShaderManager() { return m_shaderManager; }
		TextureManager& GetTextureManager() { return m_textureManager; }


		void LoadTextureResource(GUID guid);
		void LoadTextureResource(String256 name);
		void LoadMaterialResource(GUID guid);
		void LoadMaterialResource(String256 name);


#ifdef PLATFORM_WINDOWS
		VlkResourceManager& GetVulkanResources() { return m_platformResourceManager; }
#endif

		Mesh m_unitCube;
		Mesh m_unitSphere;
		Mesh m_unitCylinder;

	private:

		ShaderManager m_shaderManager;
		TextureManager m_textureManager;
		RenderingDevice* m_renderDevice;

#ifdef PLATFORM_WINDOWS
		VlkResourceManager m_platformResourceManager;
#endif

		GrowingArray<Material> m_materials;
	};
}

