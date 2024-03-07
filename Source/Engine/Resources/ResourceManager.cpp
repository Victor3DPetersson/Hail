#include "Engine_PCH.h"
#include "ResourceManager.h"
#include "glm\geometric.hpp"
#include "MathUtils.h"
#include "Renderer.h"
#include "RenderCommands.h"
#include "Timer.h"

#include "ResourceRegistry.h"
#include "HailEngine.h"

#ifdef PLATFORM_WINDOWS
#include "windows\VulkanInternal\VlkResourceManager.h"
#include "windows\VulkanInternal\VlkSwapChain.h"
#include "Resources\Vulkan\VlkTextureManager.h"
#include "Resources\Vulkan\VlkMaterialManager.h"

#endif

namespace Hail
{
	Mesh CreateUnitCube();
	Mesh CreateUnitSphere();
	Mesh CreateUnitCylinder();
	void BuildCylinderTopCap(float topRadius, float height, uint32_t sliceCount, GrowingArray<VertexModel>& meshVertices, GrowingArray<uint32_t>& meshIndeces);
	void BuildCylinderBottomCap(float bottomRadius, float height, uint32_t sliceCount, GrowingArray<VertexModel>& meshVertices, GrowingArray<uint32_t>& meshIndeces);
}


Hail::ResourceManager::ResourceManager()
{
	m_unitCube = CreateUnitCube();
	m_unitSphere = CreateUnitSphere();
	m_unitCylinder = CreateUnitCylinder();

#ifdef PLATFORM_WINDOWS
	m_textureManager = new VlkTextureResourceManager();
	m_materialManager = new VlkMaterialManager();
	m_renderingResourceManager = new VlkRenderingResourceManager();
	m_swapChain = new VlkSwapChain();
#endif

}

bool Hail::ResourceManager::InitResources(RenderingDevice* renderingDevice)
{
	m_renderDevice = renderingDevice;
	m_swapChain->Init(m_renderDevice);
	m_textureManager->Init(m_renderDevice);
	m_renderingResourceManager->Init(m_renderDevice, m_swapChain);
	m_materialManager->Init(m_renderDevice, m_textureManager, m_renderingResourceManager, m_swapChain );
	m_mainPassFrameBufferTexture = m_textureManager->FrameBufferTexture_Create("MainRenderPass", m_swapChain->GetRenderTargetResolution(), TEXTURE_FORMAT::R8G8B8A8_UINT, TEXTURE_DEPTH_FORMAT::D16_UNORM);

	//Temp, needs to be more data driven
	for (uint32_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		if(!m_materialManager->InitMaterial(MATERIAL_TYPE::SPRITE, m_mainPassFrameBufferTexture, false, i))
		{
			return false;
		}
		if(!m_materialManager->InitMaterial(MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX, m_swapChain->GetFrameBufferTexture(), false, i))
		{
			return false;
		}
		if(!m_materialManager->InitMaterial(MATERIAL_TYPE::MODEL3D, m_mainPassFrameBufferTexture, false, i))
		{
			return false;
		}
		if (!m_materialManager->InitMaterial(MATERIAL_TYPE::DEBUG_LINES2D, m_mainPassFrameBufferTexture, false, i))
		{
			return false;
		}
		if (!m_materialManager->InitMaterial(MATERIAL_TYPE::DEBUG_LINES3D, m_mainPassFrameBufferTexture, false, i))
		{
			return false;
		}
	}
	m_materialManager->InitDefaultMaterialInstances();

	return true;
}

void Hail::ResourceManager::ClearAllResources()
{
	m_textureManager->ClearAllResources();
	m_materialManager->ClearAllResources();
	m_renderingResourceManager->ClearAllResources();
	m_mainPassFrameBufferTexture->ClearResources(m_renderDevice);
}

void Hail::ResourceManager::SetTargetResolution(eResolutions targetResolution)
{
	m_swapChain->SetTargetResolution(ResolutionFromEnum(targetResolution));
	m_targetResolution = targetResolution;
}

void Hail::ResourceManager::SetWindowResolution(eResolutions targetResolution)
{
	m_swapChain->SetWindowResolution(ResolutionFromEnum(targetResolution));
}

void Hail::ResourceManager::SetReloadOfAllResources()
{
	if (m_reloadEverything || m_reloadAllTextures)
	{
		return;
	}
	m_reloadEverything = true;
	m_reloadFrameCounter = 0;
}

void Hail::ResourceManager::SetReloadOfAllTextures()
{
	if (m_reloadEverything || m_reloadAllTextures)
	{
		return;
	}
	m_reloadAllTextures = true;
	m_reloadFrameCounter = 0;
}

void Hail::ResourceManager::ReloadResources()
{
	if (!m_reloadEverything && !m_reloadAllTextures)
	{
		return;
	}

	m_frameInFlightIndex = m_swapChain->GetFrameInFlight();
	if (m_reloadEverything)
	{
		m_textureManager->ReloadAllTextures(m_frameInFlightIndex);
		m_materialManager->ReloadAllMaterials(m_frameInFlightIndex);
	}
	if (m_reloadAllTextures)
	{
		m_textureManager->ReloadAllTextures(m_frameInFlightIndex);
		m_materialManager->ReloadAllMaterialInstances(m_frameInFlightIndex);
	}

	if (++m_reloadFrameCounter == MAX_FRAMESINFLIGHT)
	{
		m_reloadEverything = false;
		m_reloadAllTextures = false;
	}

}

void Hail::ResourceManager::LoadMaterialResource(GUID guid)
{
	if (guid == guidZero)
		return;

	ResourceRegistry& reg = GetResourceRegistry();
	if (!reg.GetIsResourceImported(ResourceType::Material, guid))
		return;

	SerializeableMaterialInstance matData = m_materialManager->LoadMaterialSerializeableInstance(reg.GetProjectPath(ResourceType::Material, guid));

	//TODO: make this more data driven

	MaterialInstance instance;
	instance.m_id = guid;
	instance.m_textureHandles[0] = INVALID_TEXTURE_HANDLE;

	if (matData.m_textureHandles[0] != guidZero)
		instance.m_textureHandles[0] = m_textureManager->LoadTexture(matData.m_textureHandles[0]);

	uint32 instanceID = m_materialManager->CreateInstance(matData.m_baseMaterialType, instance);
	bool result = true;
	for (uint32_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
	{
		result &= m_materialManager->InitMaterialInstance(instanceID, i);
	}

	if (!result)
	{
		// TODO:: assert / make it in to an invalid material
		result = true; // for debugging atm
	}
}

Hail::uint32 Hail::ResourceManager::GetMaterialInstanceHandle(GUID guid) const
{
	return m_materialManager->GetMaterialInstanceHandle(guid);
}

void Hail::ResourceManager::UpdateRenderBuffers(RenderCommandPool& renderPool, Timer* timer)
{
	const GrowingArray<TextureResource>& textures = m_textureManager->GetTexturesCommonData();

	//Sprites___

	const uint32_t numberOfSprites = renderPool.spriteCommands.Size();
	const glm::vec2 renderResolution = m_swapChain->GetRenderTargetResolution();
	for (size_t sprite = 0; sprite < numberOfSprites; sprite++)
	{
		const RenderCommand_Sprite& spriteCommand = renderPool.spriteCommands[sprite];
		//Get sprite texture size and sort with material and everything here to the correct place. 
		const MaterialInstance& materialInstance = m_materialManager->GetMaterialInstance(spriteCommand.materialInstanceID, MATERIAL_TYPE::SPRITE);
		const TextureResource& texture = materialInstance.m_textureHandles[0] != INVALID_TEXTURE_HANDLE ? textures[materialInstance.m_textureHandles[0]] : m_textureManager->GetDefaultTextureCommonData();
		const float textureAspectRatio = (float)texture.m_compiledTextureData.header.width / (float)texture.m_compiledTextureData.header.height;
		

		glm::vec2 spriteScale = spriteCommand.transform.GetScale();
		const glm::vec2 spriteSizeMultiplier = spriteCommand.bSizeRelativeToRenderTarget ? glm::vec2(1.0, 1.0) : spriteCommand.transform.GetScale();
		if (spriteCommand.bSizeRelativeToRenderTarget)
		{
			spriteScale.x *= textureAspectRatio;
		}
		else
		{
			const glm::vec2 scaleMultiplier = glm::vec2(texture.m_compiledTextureData.header.width, texture.m_compiledTextureData.header.height) / renderResolution.y;
			spriteScale = (spriteScale * 2.0f) * scaleMultiplier;
		}
		
		const glm::vec2 spritePosition = spriteCommand.transform.GetPosition();
		SpriteInstanceData spriteInstance{};

		spriteInstance.position_scale = { spritePosition.x, spritePosition.y, spriteScale.x, spriteScale.y };
		spriteInstance.uvTR_BL = spriteCommand.uvTR_BL;
		spriteInstance.color = spriteCommand.color;
		spriteInstance.pivot_rotation_padding = { spriteCommand.pivot.x, spriteCommand.pivot.y, spriteCommand.transform.GetRotationRad(), 0.0f };
		spriteInstance.sizeMultiplier_effectData_padding = { spriteSizeMultiplier.x, spriteSizeMultiplier.y, 0, 0 };
		m_spriteInstanceData.Add(spriteInstance);
	}
	m_renderingResourceManager->MapMemoryToBuffer(BUFFERS::SPRITE_INSTANCE_BUFFER, m_spriteInstanceData.Data(), sizeof(SpriteInstanceData) * m_spriteInstanceData.Size());
	
	//Debug Lines___
	//TODO: Add proper support for 3D lines and sort this list
	m_numberOf2DDebugLines = renderPool.debugLineCommands.Size();
	for (size_t iDebugLine = 0; iDebugLine < m_numberOf2DDebugLines; iDebugLine++)
	{
		const RenderCommand_DebugLine& debugLine = renderPool.debugLineCommands[iDebugLine];

		DebugLineData line; 
		line.posAndIs2D = glm::vec4(debugLine.pos1.x, debugLine.pos1.y, debugLine.pos1.z, debugLine.bIs2D ? 0.0f : 1.0f);
		line.color = debugLine.color1;
		m_debugLineData.Add(line);

		line.posAndIs2D = glm::vec4(debugLine.pos2.x, debugLine.pos2.y, debugLine.pos2.z, debugLine.bIs2D ? 0.0f : 1.0f);
		line.color = debugLine.color2;
		m_debugLineData.Add(line);
	}
	m_renderingResourceManager->MapMemoryToBuffer(BUFFERS::DEBUG_LINE_INSTANCE_BUFFER, m_debugLineData.Data(), sizeof(DebugLineData) * m_debugLineData.Size());

	//Common GPU buffers___

	const float deltaTime = timer->GetDeltaTime();
	const float totalTime = timer->GetTotalTime();

	PerFrameUniformBuffer perFrameData{};

	perFrameData.totalTime_horizonLevel.x = totalTime;
	perFrameData.totalTime_horizonLevel.y = 0.0f;
	perFrameData.mainRenderResolution = m_swapChain->GetRenderTargetResolution();
	perFrameData.mainWindowResolution = m_swapChain->GetSwapChainResolution();
	m_renderingResourceManager->MapMemoryToBuffer(BUFFERS::PER_FRAME_DATA, &perFrameData, sizeof(perFrameData));

	TutorialUniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(1.0f) + totalTime * 0.15f, glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = Transform3D::GetMatrix(renderPool.camera3D.GetTransform());
	ubo.proj = glm::perspective(glm::radians(renderPool.camera3D.GetFov()),
		(float)(perFrameData.mainRenderResolution.x) / (float)(perFrameData.mainRenderResolution.y), 
		renderPool.camera3D.GetNear(), renderPool.camera3D.GetFar());
	ubo.proj[1][1] *= -1;
	m_renderingResourceManager->MapMemoryToBuffer(BUFFERS::TUTORIAL, &ubo, sizeof(ubo));

	PerCameraUniformBuffer perCameraData{};
	perCameraData.proj = ubo.proj;
	perCameraData.view = ubo.view;
	m_renderingResourceManager->MapMemoryToBuffer(BUFFERS::PER_CAMERA_DATA, &perCameraData, sizeof(perCameraData));
}

void Hail::ResourceManager::ClearFrameData()
{
	m_spriteInstanceData.Clear();
	m_debugLineData.Clear();
}

void Hail::ResourceManager::SetSwapchainTargetResolution(glm::uvec2 targetResolution)
{
	m_swapChain->SetTargetResolution(targetResolution);
}

Hail::Mesh Hail::CreateUnitCube()
{
	Mesh mesh{};
	float w = 50;
	float h = 50;
	float d = 50;

	//Create Vertices
	const int amountOfVertices = 24;
	mesh.vertices.Prepare(24);
	mesh.vertices.Fill();
	VertexModel v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24;

	glm::vec3 toBinormal;
	//Back
	v1.pos = { -w, -h, -d };
	v1.color = { 0.5, 0.5, 0, 1.0 };
	v1.texCoord1 = { 0, 1 };
	v1.norm = { 0, 0, -1 };
	v1.tangent = { 1, 0, 0 };
	toBinormal = { v1.norm.x, v1.norm.y, v1.norm.z };
	toBinormal = glm::cross(toBinormal, { v1.tangent.x, v1.tangent.y, v1.tangent.z });
	v1.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v2.pos = { -w, h, -d };
	v2.color = { 0.5, 0.5, 0, 1.0 };
	v2.texCoord1 = { 0, 0 };
	v2.norm = { 0, 0, -1 };
	v2.tangent = { 1, 0, 0 };
	toBinormal = { v2.norm.x, v2.norm.y, v2.norm.z };
	toBinormal = glm::cross(toBinormal, { v2.tangent.x, v2.tangent.y, v2.tangent.z });
	v2.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v3.pos = { w, h, -d };
	v3.color = { 0.5, 0.5, 0, 1.0 };
	v3.texCoord1 = { 1, 0 };
	v3.norm = { 0, 0, -1 };
	v3.tangent = { 1, 0, 0 };
	toBinormal = { v3.norm.x, v3.norm.y, v3.norm.z };
	toBinormal = glm::cross(toBinormal, { v3.tangent.x, v3.tangent.y, v3.tangent.z });
	v3.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v4.pos = { w, -h, -d };
	v4.color = { 0.5, 0.5, 0, 1.0 };
	v4.texCoord1 = { 1, 1 };
	v4.norm = { 0, 0, -1 };
	v4.tangent = { 1, 0, 0 };
	toBinormal = { v4.norm.x, v4.norm.y, v4.norm.z };
	toBinormal = glm::cross(toBinormal, { v4.tangent.x, v4.tangent.y, v4.tangent.z });
	v4.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	//Front
	v5.pos = { -w, -h, d };
	v5.color = { 0.5, 0.5, 1.0, 1.0 };
	v5.texCoord1 = { 1, 1 };
	v5.norm = { 0, 0, 1 };
	v5.tangent = { -1, 0, 0 };
	toBinormal = { v5.norm.x, v5.norm.y, v5.norm.z };
	toBinormal = glm::cross(toBinormal, { v5.tangent.x, v5.tangent.y, v5.tangent.z });
	v5.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v6.pos = { w, -h, d };
	v6.color = { 0.5, 0.5, 1.0, 1.0 };
	v6.texCoord1 = { 0, 1 };
	v6.norm = { 0, 0, 1 };
	v6.tangent = { -1, 0, 0 };
	toBinormal = { v6.norm.x, v6.norm.y, v6.norm.z };
	toBinormal = glm::cross(toBinormal, { v6.tangent.x, v6.tangent.y, v6.tangent.z });
	v6.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v7.pos = { w, h, d };
	v7.color = { 0.5, 0.5, 1.0, 1.0 };
	v7.texCoord1 = { 0, 0 };
	v7.norm = { 0, 0, 1 };
	v7.tangent = { -1, 0, 0 };
	toBinormal = { v7.norm.x, v7.norm.y, v7.norm.z };
	toBinormal = glm::cross(toBinormal, { v7.tangent.x, v7.tangent.y, v7.tangent.z });
	v7.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v8.pos = { -w, h, d };
	v8.color = { 0.5, 0.5, 1.0, 1.0 };
	v8.texCoord1 = { 1, 0 };
	v8.norm = { 0, 0, 1 };
	v8.tangent = { -1, 0, 0 };
	toBinormal = { v8.norm.x, v8.norm.y, v8.norm.z };
	toBinormal = glm::cross(toBinormal, { v8.tangent.x, v8.tangent.y, v8.tangent.z });
	v8.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	//Top
	v9.pos = { -w, h, -d };
	v9.color = { 0.5, 1.0, 0.5, 1.0 };
	v9.texCoord1 = { 0, 1 };
	v9.norm = { 0, 1, 0 };
	v9.tangent = { 1, 0, 0 };
	toBinormal = { v9.norm.x, v9.norm.y, v9.norm.z };
	toBinormal = glm::cross(toBinormal, { v9.tangent.x, v9.tangent.y, v9.tangent.z });
	v9.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v10.pos = { -w, h, d };
	v10.color = { 0.5, 1.0, 0.5, 1.0 };
	v10.texCoord1 = { 0, 0 };
	v10.norm = { 0, 1, 0 };
	v10.tangent = { 1, 0, 0 };
	toBinormal = { v10.norm.x, v10.norm.y, v10.norm.z };
	toBinormal = glm::cross(toBinormal, { v10.tangent.x, v10.tangent.y, v10.tangent.z });
	v10.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v11.pos = { w, h, d };
	v11.color = { 0.5, 1.0, 0.5, 1.0 };
	v11.texCoord1 = { 1, 0 };
	v11.norm = { 0, 1, 0 };
	v11.tangent = { 1, 0, 0 };
	toBinormal = { v11.norm.x, v11.norm.y, v11.norm.z };
	toBinormal = glm::cross(toBinormal, { v11.tangent.x, v11.tangent.y, v11.tangent.z });
	v11.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v12.pos = { w, h, -d };
	v12.color = { 0.5, 1.0, 0.5, 1.0 };
	v12.texCoord1 = { 1, 1 };
	v12.norm = { 0, 1, 0 };
	v12.tangent = { 1, 0, 0 };
	toBinormal = { v12.norm.x, v12.norm.y, v12.norm.z };
	toBinormal = glm::cross(toBinormal, { v12.tangent.x, v12.tangent.y, v12.tangent.z });
	v12.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	//Bot
	v13.pos = { -w, -h, -d };
	v13.color = { 0.5, 0, 0.5, 1.0 };
	v13.texCoord1 = { 1, 1 };
	v13.norm = { 0, -1, 0 };
	v13.tangent = { -1, 0, 0 };
	toBinormal = { v13.norm.x, v13.norm.y, v13.norm.z };
	toBinormal = glm::cross(toBinormal, { v13.tangent.x, v13.tangent.y, v13.tangent.z });
	v13.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v14.pos = { w, -h, -d };
	v14.color = { 0.5, 0, 0.5, 1.0 };
	v14.texCoord1 = { 0, 1 };
	v14.norm = { 0, -1, 0 };
	v14.tangent = { -1, 0, 0 };
	toBinormal = { v14.norm.x, v14.norm.y, v14.norm.z };
	toBinormal = glm::cross(toBinormal, { v14.tangent.x, v14.tangent.y, v14.tangent.z });
	v14.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v15.pos = { w, -h, d };
	v15.color = { 0.5, 0, 0.5, 1.0 };
	v15.texCoord1 = { 0, 0 };
	v15.norm = { 0, -1, 0 };
	v15.tangent = { -1, 0, 0 };
	toBinormal = { v15.norm.x, v15.norm.y, v15.norm.z };
	toBinormal = glm::cross(toBinormal, { v15.tangent.x, v15.tangent.y, v15.tangent.z });
	v15.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v16.pos = { -w, -h, d };
	v16.color = { 0.5, 0, 0.5, 1.0 };
	v16.texCoord1 = { 1, 0 };
	v16.norm = { 0, -1, 0 };
	v16.tangent = { -1, 0, 0 };
	toBinormal = { v16.norm.x, v16.norm.y, v16.norm.z };
	toBinormal = glm::cross(toBinormal, { v16.tangent.x, v16.tangent.y, v16.tangent.z });
	v16.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	//Left
	v17.pos = { -w, -h, d };
	v17.color = { 0, 0.5, 0.5, 1.0 };
	v17.texCoord1 = { 0, 1 };
	v17.norm = { -1, 0, 0 };
	v17.tangent = { 0, 0, -1 };
	toBinormal = { v17.norm.x, v17.norm.y, v17.norm.z };
	toBinormal = glm::cross(toBinormal, { v17.tangent.x, v17.tangent.y, v17.tangent.z });
	v17.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v18.pos = { -w, h, d };
	v18.color = { 0, 0.5, 0.5, 1.0 };
	v18.texCoord1 = { 0, 0 };
	v18.norm = { -1, 0, 0 };
	v18.tangent = { 0, 0, -1 };
	toBinormal = { v18.norm.x, v18.norm.y, v18.norm.z };
	toBinormal = glm::cross(toBinormal, { v18.tangent.x, v18.tangent.y, v18.tangent.z });
	v18.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v19.pos = { -w, h, -d };
	v19.color = { 0, 0.5, 0.5, 1.0 };
	v19.texCoord1 = { 1, 0 };
	v19.norm = { -1, 0, 0 };
	v19.tangent = { 0, 0, -1 };
	toBinormal = { v19.norm.x, v19.norm.y, v19.norm.z };
	toBinormal = glm::cross(toBinormal, { v19.tangent.x, v19.tangent.y, v19.tangent.z });
	v19.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v20.pos = { -w, -h, -d };
	v20.color = { 0, 0.5, 0.5, 1.0 };
	v20.texCoord1 = { 1, 1 };
	v20.norm = { -1, 0, 0 };
	v20.tangent = { 0, 0, -1 };
	toBinormal = { v20.norm.x, v20.norm.y, v20.norm.z };
	toBinormal = glm::cross(toBinormal, { v20.tangent.x, v20.tangent.y, v20.tangent.z });
	v20.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	//Right
	v21.pos = { w, -h, -d };
	v21.color = { 1.0, 0.5, 0.5, 1.0 };
	v21.texCoord1 = { 0, 1 };
	v21.norm = { 1, 0, 0 };
	v21.tangent = { 0, 0, 1 };
	toBinormal = { v21.norm.x, v21.norm.y, v21.norm.z };
	toBinormal = glm::cross(toBinormal, { v21.tangent.x, v21.tangent.y, v21.tangent.z });
	v21.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v22.pos = { w, h, -d };
	v22.color = { 1.0, 0.5, 0.5, 1.0 };
	v22.texCoord1 = { 0, 0 };
	v22.norm = { 1, 0, 0 };
	v22.tangent = { 0, 0, 1 };
	toBinormal = { v22.norm.x, v22.norm.y, v22.norm.z };
	toBinormal = glm::cross(toBinormal, { v22.tangent.x, v22.tangent.y, v22.tangent.z });
	v22.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v23.pos = { w, h, d };
	v23.color = { 1.0, 0.5, 0.5, 1.0 };
	v23.texCoord1 = { 1, 0 };
	v23.norm = { 1, 0, 0 };
	v23.tangent = { 0, 0, 1 };
	toBinormal = { v23.norm.x, v23.norm.y, v23.norm.z };
	toBinormal = glm::cross(toBinormal, { v23.tangent.x, v23.tangent.y, v23.tangent.z });
	v23.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	v24.pos = { w, -h, d };
	v24.color = { 1.0, 0.5, 0.5, 1.0 };
	v24.texCoord1 = { 1, 1 };
	v24.norm = { 1, 0, 0 };
	v24.tangent = { 0, 0, 1 };
	toBinormal = { v24.norm.x, v24.norm.y, v24.norm.z };
	toBinormal = glm::cross(toBinormal, { v24.tangent.x, v24.tangent.y, v24.tangent.z });
	v24.biTangent = { toBinormal.x, toBinormal.y, toBinormal.z };

	mesh.vertices[0] = (v1);
	mesh.vertices[1] = (v2);
	mesh.vertices[2] = (v3);
	mesh.vertices[3] = (v4);
	mesh.vertices[4] = (v5);
	mesh.vertices[5] = (v6);
	mesh.vertices[6] = (v7);
	mesh.vertices[7] = (v8);
	mesh.vertices[8] = (v9);
	mesh.vertices[9] = (v10);
	mesh.vertices[10] = (v11);
	mesh.vertices[11] = (v12);
	mesh.vertices[12] = (v13);
	mesh.vertices[13] = (v14);
	mesh.vertices[14] = (v15);
	mesh.vertices[15] = (v16);
	mesh.vertices[16] = (v17);
	mesh.vertices[17] = (v18);
	mesh.vertices[18] = (v19);
	mesh.vertices[19] = (v20);
	mesh.vertices[20] = (v21);
	mesh.vertices[21] = (v22);
	mesh.vertices[22] = (v23);
	mesh.vertices[23] = (v24);

	mesh.indices =
	{
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};

	mesh.min = { -50.0, -50.0, -50.0 };
	mesh.max = { 50.0, 50.0, 50.0 };
	return mesh;
}

Hail::Mesh Hail::CreateUnitSphere()
{
	Mesh mesh;
	float radius = 50.0f;
	uint32_t sliceCount = 12;
	uint32_t stackCount = 12;

	float phiStep = Math::PIf / stackCount;
	float thetaStep = 2.0f * Math::PIf / sliceCount;

	mesh.vertices;
	mesh.vertices.Prepare(40);
	VertexModel topVertex;
	topVertex.pos = { 0, radius, 0 };
	topVertex.color = { 1.0, 1.0,1.0,1.0 };
	topVertex.texCoord1 = { 0, 1 };
	mesh.vertices.Add(topVertex);

	for (uint32_t i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;
		for (uint32_t j = 0; j <= sliceCount; j++) {
			float theta = j * thetaStep;
			glm::vec3 p =
			{
				(radius * sinf(phi) * cosf(theta)),
				(radius * cosf(phi)),
				(radius * sinf(phi) * sinf(theta))
			};
			glm::vec3 tangent =
			{
				-radius * sinf(phi) * sinf(theta),
				0,
				radius * sinf(phi) * cosf(theta)
			};
			glm::normalize(tangent);
			glm::vec3 n = { p.x, p.y, p.z };
			glm::normalize(n);
			glm::vec3 binormal;
			binormal = glm::cross(tangent, n);
			glm::normalize(binormal);
			glm::vec2 uv = { theta / (Math::PIf * 2), phi / Math::PIf };

			VertexModel vertex;
			vertex.pos = p;
			vertex.color = { p.x * 1.0, p.y * 1.0 , p.z * 1.0 , 1.0 };
			vertex.texCoord1 = uv;
			vertex.norm = { n.x, n.y, n.z };
			vertex.biTangent = { binormal.x, binormal.y, binormal.z };;
			vertex.tangent = { tangent.x, tangent.y, tangent.z };;
			mesh.vertices.Add(vertex);
		}
	}
	VertexModel botVertex;
	botVertex.pos = { 0, -radius, 0 };
	botVertex.color = { 0, 0,0,1.0 };
	botVertex.texCoord1 = { 0, 1 };
	botVertex.norm = { 0, -1, 0 };
	botVertex.tangent = { 1, 0, 0 };
	glm::vec3 tangent = { 1, 0, 0 };
	glm::vec3 binormal = glm::cross(tangent, { 0, -1, 0 });
	botVertex.biTangent = { binormal.x, binormal.y, binormal.z };

	mesh.vertices.Add(botVertex);

	
	mesh.indices.Prepare(mesh.vertices.Size() * 3);

	for (uint32_t i = 1; i <= sliceCount; i++) {
		mesh.indices.Add(0);
		mesh.indices.Add(i + 1);
		mesh.indices.Add(i);
	}
	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount - 2; i++) {
		for (uint32_t j = 0; j < sliceCount; j++) {
			mesh.indices.Add(baseIndex + i * ringVertexCount + j);
			mesh.indices.Add(baseIndex + i * ringVertexCount + j + 1);
			mesh.indices.Add(baseIndex + (i + 1) * ringVertexCount + j);

			mesh.indices.Add(baseIndex + (i + 1) * ringVertexCount + j);
			mesh.indices.Add(baseIndex + i * ringVertexCount + j + 1);
			mesh.indices.Add(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}
	uint32_t southPoleIndex = mesh.vertices.Size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (uint32_t i = 0; i < sliceCount; i++) {
		mesh.indices.Add(southPoleIndex);
		mesh.indices.Add(baseIndex + i);
		mesh.indices.Add(baseIndex + i + 1);
	}
	mesh.min = { -50.0, -50.0, -50.0 };
	mesh.max = { 50.0, 50.0, 50.0 };
	return mesh;
}

Hail::Mesh Hail::CreateUnitCylinder()
{
	Mesh mesh;
	float bottomRadius, topRadius, height;
	uint32_t sliceCount, stackCount;
	bottomRadius = 50.0;
	topRadius = bottomRadius;
	height = 100.0;
	sliceCount = 16;
	stackCount = 8;
	float stackHeight = height / (float)stackCount;
	float radiusStep = (topRadius - bottomRadius) / stackCount;
	uint32_t ringCount = stackCount + 1;

	//Vertices
	GrowingArray<VertexModel>& vertices = mesh.vertices;
	vertices.Prepare(40);
	for (uint32_t i = 0; i < ringCount + 1; i++)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		float dTheta = 2.0f * Math::PIf / sliceCount;
		for (uint32_t j = 0; j <= sliceCount; j++)
		{
			VertexModel vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.pos = { r * c, y, r * s };
			vertex.texCoord1.x = (float)j / sliceCount;
			vertex.texCoord1.y = 1.0f - (float)i / stackCount;
			vertex.color = { r * c * 1.0, y * 1.0, r * s * 1.0, 1.0 };
			vertex.tangent = { -s, 0.0f, c };
			float dr = bottomRadius - topRadius;
			vertex.biTangent = { dr * c, -height, dr * s };

			glm::vec3 normal = { vertex.tangent.x,vertex.tangent.y,vertex.tangent.z };

			normal = glm::cross(normal, { vertex.biTangent.x,vertex.biTangent.y,vertex.biTangent.z });
			vertex.norm = { normal.x, normal.y, normal.z };

			vertices.Add(vertex);
		}
	}

	//Indices
	GrowingArray<uint32_t>& vertexIndices = mesh.indices;
	vertexIndices.Prepare(vertices.Size() * 3);
	uint32_t ringVertexCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount; i++)
	{
		for (uint32_t j = 0; j < sliceCount; j++)
		{
			vertexIndices.Add(i * ringVertexCount + j);
			vertexIndices.Add((i + 1) * ringVertexCount + j);
			vertexIndices.Add((i + 1) * ringVertexCount + j + 1);

			vertexIndices.Add(i * ringVertexCount + j);
			vertexIndices.Add((i + 1) * ringVertexCount + j + 1);
			vertexIndices.Add(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(topRadius, height, sliceCount, vertices, vertexIndices);
	BuildCylinderBottomCap(bottomRadius, height, sliceCount, vertices, vertexIndices);
	mesh.min = { -50.0, -50.0, -50.0 };
	mesh.max = { 50.0, 50.0, 50.0 };
	return mesh;
}

void Hail::BuildCylinderTopCap(float topRadius, float height, uint32_t sliceCount, GrowingArray<VertexModel>& meshVertices, GrowingArray<uint32_t>& meshIndeces)
{
	uint32_t baseIndex = (uint32_t)meshVertices.Size();

	float y = 0.5f * height;
	float dTheta = 2.0f * Math::PIf / sliceCount;
	for (uint32_t i = 0; i <= sliceCount; i++)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;
		VertexModel vertex;
		vertex.pos = { x, y, z };
		vertex.texCoord1 = { u,v };
		vertex.color = { x * 1.0 * y, 1.0 * y, z * 1.0 * y, 1.0 };
		vertex.tangent = { 0,0,1 };
		vertex.biTangent = { 1,0,0 };
		vertex.norm = { 0,1,0 };

		meshVertices.Add(vertex);
	}
	//Cap Center Vertex
	VertexModel topMidVertex;
	topMidVertex.pos = { 0.0f, y, 0.0f };
	topMidVertex.texCoord1 = { 0.5f, 0.5f };
	topMidVertex.color = { 1.0 * y, 1.0 * y, 1.0 * y,1.0 };
	topMidVertex.tangent = { 0,0,1 };
	topMidVertex.biTangent = { 1,0,0 };
	topMidVertex.norm = { 0,1,0 };
	meshVertices.Add(topMidVertex);
	uint32_t centerIndex = (uint32_t)meshVertices.Size() - 1;

	for (uint32_t i = 0; i < sliceCount; i++)
	{
		meshIndeces.Add(centerIndex);
		meshIndeces.Add(baseIndex + i + 1);
		meshIndeces.Add(baseIndex + i);
	}

}

void Hail::BuildCylinderBottomCap(float bottomRadius, float height, uint32_t sliceCount, GrowingArray<VertexModel>& meshVertices, GrowingArray<uint32_t>& meshIndeces)
{
	uint32_t baseIndex = (uint32_t)meshVertices.Size();

	float y = -0.5f * height;
	float dTheta = 2.0f * Math::PIf / sliceCount;
	for (uint32_t i = 0; i <= sliceCount; i++)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;
		VertexModel vertex;
		vertex.pos = { x, y, z };
		vertex.texCoord1 = { u,v };
		vertex.color = { x * 1.0 * y, 0 , z * 1.0 * y, 1.0 };
		vertex.tangent = { 0,0,1 };
		vertex.biTangent = { -1,0,0 };
		vertex.norm = { 0,-1,0 };
		meshVertices.Add(vertex);
	}
	//Cap Center Vertex
	VertexModel botMiddleVertex;
	botMiddleVertex.pos = { 0.0f, y, 0.0f };
	botMiddleVertex.texCoord1 = { 0.5f, 0.5f };
	botMiddleVertex.color = { 1.0 * y, 0, 1.0 * y ,1.0 };
	botMiddleVertex.tangent = { 0,0,1 };
	botMiddleVertex.biTangent = { -1,0,0 };
	botMiddleVertex.norm = { 0,-1,0 };
	meshVertices.Add(botMiddleVertex);
	uint32_t centerIndex = (uint32_t)meshVertices.Size() - 1;

	for (uint32_t i = 0; i < sliceCount; i++)
	{
		meshIndeces.Add(centerIndex);
		meshIndeces.Add(baseIndex + i);
		meshIndeces.Add(baseIndex + i + 1);
	}
}