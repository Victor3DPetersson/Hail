#include "Engine_PCH.h"
#include "VlkMaterialManager.h"

#include "Windows\VulkanInternal\VlkDevice.h"
#include "Windows\VulkanInternal\VlkSwapChain.h"
#include "Windows\VulkanInternal\VlkVertex_Descriptor.h"
#include "VlkBufferResource.h"
#include "Windows\VulkanInternal\VlkResourceManager.h"
#include "VlkTextureManager.h"
#include "Windows\VulkanInternal\VlkMaterialCreationUtils.h"
#include "VlkMaterial.h"

using namespace Hail;

namespace
{
	VkShaderModule CreateShaderModule(CompiledShader& shader, VlkDevice& device)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shader.header.sizeOfShaderData;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.compiledCode);
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			
			// TODO: Throw error and move this to the shader loading stage so we can have an early test for this.
			return nullptr;
		}
		return shaderModule;
	}

}

void VlkMaterialManager::Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain)
{
	MaterialManager::Init(renderingDevice, textureResourceManager, renderingResourceManager, swapChain);
}

void Hail::VlkMaterialManager::BindFrameBuffer(eMaterialType materialType, FrameBufferTexture* frameBufferToBindToMaterial)
{
	if (frameBufferToBindToMaterial)
	{
		m_passesFrameBufferTextures[(uint32)materialType] = (VlkFrameBufferTexture*)(frameBufferToBindToMaterial);
	}
	else
	{
		m_passesFrameBufferTextures[(uint32)materialType] = nullptr;
	}
}

bool VlkMaterialManager::InitMaterialInternal(Material* pMaterial, uint32 frameInFlight)
{
	const uint32 materialBaseType = (uint32)pMaterial->m_type;

	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	VlkMaterial* pVlkMat = (VlkMaterial*)pMaterial;

	ResourceValidator& materialDataValidator = pVlkMat->m_validator;
	if (materialDataValidator.GetIsResourceDirty())
	{
		if (!materialDataValidator.GetIsFrameDataDirty(frameInFlight))
		{
			//Add fatal assert here that something have gone wrong
			return false;
		}
	}

	pVlkMat->m_frameBufferTextures = m_passesFrameBufferTextures[materialBaseType];

	if (materialDataValidator.GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		if (!CreateMaterialTypeDescriptor(pMaterial))
		{
			// TODO assert
			return false;
		}

		// TODO: assert on !pMaterial->m_pTypeDescriptor

		// TODO: if reloading a material and the materials layout is different, this needs to be looked over
		if (!CreatePipelineLayout(pMaterial))
		{
			// TODO assert
			return false;
		}


		// Create the render pass
		if (!CreateRenderpass(*pVlkMat))
		{
			return false;
		}
	}
	if (!CreateFramebuffers(*pVlkMat, frameInFlight))
	{
		return false;
	}
	AllocateTypeDescriptors(*pVlkMat, frameInFlight);

	//for debugging, check so that once everyFrameDataIsDirty the validator is set to not dirty
	materialDataValidator.ClearFrameData(frameInFlight);

	if (materialDataValidator.GetFrameThatMarkedFrameDirty() != frameInFlight)
	{
		//Early out as below the resources are only created for the first frame in flight
		return true;
	}

	if (!CreateGraphicsPipeline(*pVlkMat))
	{
		return false;
	}
	return true;
}

struct VlkLayoutDescriptor
{
	VkShaderStageFlags flags;
	uint32_t bindingPoint;
	VkDescriptorType type;
	eDecorationType decorationType;
};

bool localCreateSetLayoutDescriptor(GrowingArray<VlkLayoutDescriptor> descriptors, VkDescriptorSetLayout& returnDescriptorLayoput, VlkDevice& device)
{
	//if (descriptors.Empty())
	//{
	//	// This material does not use any resources for this set
	//	returnDescriptorLayoput = VK_NULL_HANDLE;
	//	return true;
	//}
	//
	GrowingArray<VkDescriptorSetLayoutBinding>bindings(descriptors.Size());
	bindings.Fill();

	for (uint32_t i = 0; i < descriptors.Size(); i++)
	{
		VkDescriptorSetLayoutBinding descriptor{};
		descriptor.binding = descriptors[i].bindingPoint;
		descriptor.descriptorCount = 1;
		descriptor.descriptorType = descriptors[i].type;
		descriptor.pImmutableSamplers = nullptr;
		descriptor.stageFlags = descriptors[i].flags;
		bindings[i] = descriptor;
	}

	VkDescriptorSetLayoutCreateInfo layoutSetInfo{};
	layoutSetInfo.bindingCount = bindings.Size();
	layoutSetInfo.flags = 0;
	layoutSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutSetInfo.pNext = nullptr;
	layoutSetInfo.pBindings = bindings.Data();
	if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutSetInfo, nullptr, &returnDescriptorLayoput) != VK_SUCCESS)
	{
		return false;
#ifdef DEBUG
		throw std::runtime_error("failed to create final pass descriptor set layout!");
#endif
	}
	return true;
}

VkWriteDescriptorSet WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo& bufferInfo, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dstSet;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = &bufferInfo;
	return write;
}

VkWriteDescriptorSet WriteDescriptorSampler(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo& bufferInfo, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dstSet;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = &bufferInfo;
	return write;
}

bool ValidateDescriptorSamplerWrite(VkDescriptorImageInfo& descriptorToValidate)
{
	bool returnValue = true;
	returnValue |= descriptorToValidate.imageView != VK_NULL_HANDLE;
	returnValue |= descriptorToValidate.sampler != VK_NULL_HANDLE;
	return returnValue;
}
bool ValidateDescriptorBufferWrite(VkDescriptorBufferInfo& descriptorToValidate)
{
	bool returnValue = true;
	returnValue |= descriptorToValidate.buffer != VK_NULL_HANDLE;
	returnValue |= descriptorToValidate.range != 0u;
	return returnValue;
}

bool Hail::VlkMaterialManager::CreateRenderpass(VlkMaterial& vlkMaterial)
{
	VlkMaterialTypeDescriptor& typeDescriptor = *(VlkMaterialTypeDescriptor*)m_materialTypeDescriptors[(uint32)vlkMaterial.m_type];
	if (vlkMaterial.m_type == eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
	{
		VlkSwapChain* swapChain = (VlkSwapChain*)m_swapChain;
		vlkMaterial.m_ownsRenderpass = false;
		vlkMaterial.m_renderPass = swapChain->GetRenderPass();
		return true;
	}

	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	GrowingArray<VkAttachmentDescription> attachmentDescriptors(2);
	VkAttachmentReference colorAttachmentRef{};
	VkAttachmentReference depthAttachmentRef{};
	switch (vlkMaterial.m_type)
	{
		case Hail::eMaterialType::SPRITE:
		{
			VkAttachmentDescription colorAttachment{};
			VkAttachmentDescription depthAttachment{};

			colorAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetTextureFormat());
			depthAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetDepthFormat());
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(colorAttachment);

			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(depthAttachment);
		}
		break;

		case Hail::eMaterialType::MODEL3D:
		{
			VkAttachmentDescription colorAttachment{};
			VkAttachmentDescription depthAttachment{};
			colorAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetTextureFormat());
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(colorAttachment);

			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			depthAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetDepthFormat());
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(depthAttachment);
		}
		break;

		case Hail::eMaterialType::DEBUG_LINES2D:
		{
			VkAttachmentDescription colorAttachment{};
			VkAttachmentDescription depthAttachment{};

			colorAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetTextureFormat());
			depthAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetDepthFormat());
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			attachmentDescriptors.Add(colorAttachment);

			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(depthAttachment);
		}
		break;

		case Hail::eMaterialType::DEBUG_LINES3D:
		{
			VkAttachmentDescription colorAttachment{};
			VkAttachmentDescription depthAttachment{};

			colorAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetTextureFormat());
			depthAttachment.format = ToVkFormat(vlkMaterial.m_frameBufferTextures->GetDepthFormat());
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(colorAttachment);

			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(depthAttachment);
		}
		break;

		default:
		break;
	}
	//Todo set up dependencies depending on pass
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachmentDescriptors.Size();
	renderPassInfo.pAttachments = attachmentDescriptors.Data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (vkCreateRenderPass(device.GetDevice(), &renderPassInfo, nullptr, &vlkMaterial.m_renderPass) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}
	return true;
}

void localGetDescriptorsFromDecoration(
	const VectorOnStack<ShaderDecoration, 8>* decoration1,
	VkShaderStageFlags stageFlag1, 
	const VectorOnStack<ShaderDecoration, 8>* decoration2,
	VkShaderStageFlags stageFlag2, 
	VkDescriptorType descriptorType,
	GrowingArray<VlkLayoutDescriptor>& descriptorToFill)
{
	VectorOnStack<VlkLayoutDescriptor, 8> descriptors;

	for (int i = 0; i < decoration1->Size(); i++)
	{
		VlkLayoutDescriptor descriptor;
		descriptor.bindingPoint = (*decoration1)[i].m_bindingLocation;
		descriptor.type = descriptorType;
		descriptor.flags = stageFlag1;
		descriptor.decorationType = (*decoration1)[i].m_type;
		descriptors.Add(descriptor);
	}
	if (decoration2)
	{
		for (int i = 0; i < decoration2->Size(); i++)
		{
			const uint32 bindingPoint = (*decoration2)[i].m_bindingLocation;
			bool foundDescriptor = false;
			for (size_t iPreviousDescriptors = 0; iPreviousDescriptors < descriptors.Size(); iPreviousDescriptors++)
			{
				if (bindingPoint == descriptors[iPreviousDescriptors].bindingPoint)
				{
					descriptors[iPreviousDescriptors].flags |= stageFlag2;
					foundDescriptor = true;
					break;
				}
			}
			if (!foundDescriptor)
			{
				VlkLayoutDescriptor descriptor;
				descriptor.bindingPoint = bindingPoint;
				descriptor.type = descriptorType;
				descriptor.flags = stageFlag2;
				descriptor.decorationType = (*decoration2)[i].m_type;
				descriptors.Add(descriptor);
			}
		}
	}
	for (size_t i = 0; i < descriptors.Size(); i++)
	{
		descriptorToFill.Add(descriptors[i]);
	}
}

void localGetImageDescriptorsFromDecoration(
	const VectorOnStack<ShaderDecoration, 8>* decoration1,
	VkShaderStageFlags stageFlag1,
	const VectorOnStack<ShaderDecoration, 8>* decoration2,
	VkShaderStageFlags stageFlag2,
	VkDescriptorType descriptorType,
	GrowingArray<VlkLayoutDescriptor>& descriptorToFill)
{
	VectorOnStack<VlkLayoutDescriptor, 8> descriptors;

	for (int i = 0; i < decoration1->Size(); i++)
	{
		VlkLayoutDescriptor descriptor;
		descriptor.bindingPoint = (*decoration1)[i].m_bindingLocation;
		descriptor.type = descriptorType;
		descriptor.flags = stageFlag1;
		descriptor.decorationType = (*decoration1)[i].m_type;
		descriptors.Add(descriptor);
	}
	if (decoration2)
	{
		for (int i = 0; i < decoration2->Size(); i++)
		{
			const uint32 bindingPoint = (*decoration2)[i].m_bindingLocation;
			bool foundDescriptor = false;
			for (size_t iPreviousDescriptors = 0; iPreviousDescriptors < descriptors.Size(); iPreviousDescriptors++)
			{
				if (bindingPoint == descriptors[iPreviousDescriptors].bindingPoint)
				{
					descriptors[iPreviousDescriptors].flags |= stageFlag2;
					foundDescriptor = true;
					break;
				}
			}
			if (!foundDescriptor)
			{
				VlkLayoutDescriptor descriptor;
				descriptor.bindingPoint = bindingPoint;
				descriptor.type = descriptorType;
				descriptor.flags = stageFlag2;
				descriptor.decorationType = (*decoration2)[i].m_type;
				descriptors.Add(descriptor);
			}
		}
	}
	for (size_t i = 0; i < descriptors.Size(); i++)
	{
		descriptorToFill.Add(descriptors[i]);
	}
}

GrowingArray<VlkLayoutDescriptor> localGetSetLayoutDescription(eShaderType primaryShaderType, eDecorationSets domainToFetch, ReflectedShaderData* pMainData, ReflectedShaderData* pSecondaryData)
{
	GrowingArray<VlkLayoutDescriptor> typeSetDescriptors;
	VkShaderStageFlagBits flagBit = VK_SHADER_STAGE_ALL;
	VkShaderStageFlagBits secondaryFlagBit = VK_SHADER_STAGE_ALL;
	const SetDecorations& setDecorations = pMainData->m_setDecorations[domainToFetch];
	bool use2Shaders = false;
	if (primaryShaderType == eShaderType::Compute)
	{
		flagBit = VK_SHADER_STAGE_COMPUTE_BIT;
	}
	else if (primaryShaderType == eShaderType::Fragment)
	{
		flagBit = VK_SHADER_STAGE_FRAGMENT_BIT;
		secondaryFlagBit = VK_SHADER_STAGE_VERTEX_BIT;
		use2Shaders = true;
	}
	else if (primaryShaderType == eShaderType::Vertex)
	{
		flagBit = VK_SHADER_STAGE_VERTEX_BIT;
		secondaryFlagBit = VK_SHADER_STAGE_FRAGMENT_BIT;
		use2Shaders = true;
	}
	SetDecorations* pSecondarySetDecorations = nullptr;  pMainData->m_setDecorations[domainToFetch];

	if (use2Shaders)
	{
		pSecondarySetDecorations = &pSecondaryData->m_setDecorations[domainToFetch];
	}

	localGetDescriptorsFromDecoration(
		&setDecorations.m_uniformBuffers, flagBit,
		&pSecondarySetDecorations->m_uniformBuffers, secondaryFlagBit,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, typeSetDescriptors);

	localGetDescriptorsFromDecoration(
		&setDecorations.m_storageBuffers, flagBit,
		&pSecondarySetDecorations->m_storageBuffers, secondaryFlagBit,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, typeSetDescriptors);

	localGetImageDescriptorsFromDecoration(
		&setDecorations.m_sampledImages, flagBit,
		&pSecondarySetDecorations->m_sampledImages, secondaryFlagBit,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeSetDescriptors);

	return typeSetDescriptors;
}

bool Hail::VlkMaterialManager::CreateMaterialTypeDescriptor(Material* pMaterial)
{
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);

	VlkMaterial* pVlkMaterial = (VlkMaterial*)pMaterial;

	// First create the domain set layout, we should only get here if this has not already been created
	if (!pVlkMaterial->m_instanceSetLayout)
	{
		ReflectedShaderData* pSecondaryShaderData = nullptr;
		if (pMaterial->m_pShaders.Size() > 1)
		{
			pSecondaryShaderData = &pMaterial->m_pShaders[1]->reflectedShaderData;
		}
		GrowingArray<VlkLayoutDescriptor> materialSetLayoutDescriptors = localGetSetLayoutDescription((eShaderType)pMaterial->m_pShaders[0]->header.shaderType, InstanceDomain, &pMaterial->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
		if (!localCreateSetLayoutDescriptor(materialSetLayoutDescriptors, pVlkMaterial->m_instanceSetLayout, device))
		{
			return false;
		}
	}
	else
	{
		// TODO assert here
	}

	if (m_materialTypeDescriptors[(uint32)pMaterial->m_type])
	{
		// TODO: Check if the reflected shader data matches, otherwise assert
		pMaterial->m_pTypeDescriptor = m_materialTypeDescriptors[(uint32)pMaterial->m_type];
		return true;
	}

	m_materialTypeDescriptors[(uint32)pMaterial->m_type] = new VlkMaterialTypeDescriptor();
	VlkMaterialTypeDescriptor& typeDescriptor = *(VlkMaterialTypeDescriptor*)m_materialTypeDescriptors[(uint32)pMaterial->m_type];
	typeDescriptor.m_type = pMaterial->m_type;
	if (pMaterial->m_pShaders[0]->loadState == eShaderLoadState::LoadedToRAM)
	{
		const eShaderType shader1Type = (eShaderType)pMaterial->m_pShaders[0]->header.shaderType;

		ReflectedShaderData* pSecondaryShaderData = nullptr;
		if (pMaterial->m_pShaders.Size() > 1)
		{
			pSecondaryShaderData = &pMaterial->m_pShaders[1]->reflectedShaderData;
		}

		GrowingArray<VlkLayoutDescriptor> globalSetDescriptors = localGetSetLayoutDescription((eShaderType)pMaterial->m_pShaders[0]->header.shaderType, GlobalDomain, &pMaterial->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
		if (!localCreateSetLayoutDescriptor(globalSetDescriptors, typeDescriptor.m_globalSetLayout, device))
		{
			return false;
		}
		GrowingArray<VlkLayoutDescriptor> typeSetDescriptors = localGetSetLayoutDescription((eShaderType)pMaterial->m_pShaders[0]->header.shaderType, MaterialTypeDomain, &pMaterial->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
		if (!localCreateSetLayoutDescriptor(typeSetDescriptors, typeDescriptor.m_typeSetLayout, device))
		{
			return false;
		}
	}
	else
	{
		// TODO: Assert here
		return false;
	}

	if (typeDescriptor.m_typeSetLayout != VK_NULL_HANDLE)
	{
		VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();
		GrowingArray<VkDescriptorSetLayout> setLayouts(MAX_FRAMESINFLIGHT, typeDescriptor.m_typeSetLayout);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = vlkRenderingResources->m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = (uint32)MAX_FRAMESINFLIGHT;
		passAllocInfo.pSetLayouts = setLayouts.Data();

		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, typeDescriptor.m_typeDescriptors) != VK_SUCCESS)
		{
			//TODO ASSERT
			return false;
		}
	}

	if (typeDescriptor.m_globalSetLayout != VK_NULL_HANDLE)
	{
		VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();
		GrowingArray<VkDescriptorSetLayout> setLayouts(MAX_FRAMESINFLIGHT, typeDescriptor.m_globalSetLayout);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = vlkRenderingResources->m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = (uint32)MAX_FRAMESINFLIGHT;
		passAllocInfo.pSetLayouts = setLayouts.Data();

		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, typeDescriptor.m_globalDescriptors) != VK_SUCCESS)
		{
			//TODO ASSERT
			return false;
		}
	}

	pMaterial->m_pTypeDescriptor = m_materialTypeDescriptors[(uint32)pMaterial->m_type];
	return true;
}

bool Hail::VlkMaterialManager::CreatePipelineLayout(Material* pMaterial)
{

	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	//setup push constants
	VectorOnStack<VkPushConstantRange, 4> push_constants;

	VectorOnStack<ShaderDecoration, 8>* reflectedPushConstants = &pMaterial->m_pShaders[0]->reflectedShaderData.m_pushConstants;

	if ((eShaderType)pMaterial->m_pShaders[0]->header.shaderType == eShaderType::Fragment ||
		(eShaderType)pMaterial->m_pShaders[0]->header.shaderType == eShaderType::Amplification)
	{
		reflectedPushConstants = &pMaterial->m_pShaders[1]->reflectedShaderData.m_pushConstants;
	}
	for (size_t i = 0; i < (*reflectedPushConstants).Size(); i++)
	{
		const ShaderDecoration& reflectedPushConstant = (*reflectedPushConstants)[i];
		VkPushConstantRange pushConstant;
		//this push constant range starts at the beginning
		pushConstant.offset = 0;
		//this push constant range takes up the size of a MeshPushConstants struct
		pushConstant.size = reflectedPushConstant.m_byteSize; // Check if this length is correct 
		//this push constant range is accessible only in the vertex and compute shader
		if ((eShaderType)pMaterial->m_pShaders[0]->header.shaderType == eShaderType::Vertex)
		{
			pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if ((eShaderType)pMaterial->m_pShaders[0]->header.shaderType == eShaderType::Compute)
		{
			pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		push_constants.Add(pushConstant);
	}


	VlkMaterialTypeDescriptor& typeDescriptor = *(VlkMaterialTypeDescriptor*)m_materialTypeDescriptors[(uint32)pMaterial->m_type];
	VlkMaterial* material = (VlkMaterial*)pMaterial;
	VectorOnStack<VkDescriptorSetLayout, eDecorationSets::Count> layouts;
	if (typeDescriptor.m_globalSetLayout != VK_NULL_HANDLE)
	{
		layouts.Add(typeDescriptor.m_globalSetLayout);
	}
	if (typeDescriptor.m_typeSetLayout != VK_NULL_HANDLE)
	{
		layouts.Add(typeDescriptor.m_typeSetLayout);
	}
	if (((VlkMaterial*)pMaterial)->m_instanceSetLayout != VK_NULL_HANDLE)
	{
		layouts.Add(material->m_instanceSetLayout);
	}

	VkPipelineLayoutCreateInfo passPipelineLayoutInfo{};
	passPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	passPipelineLayoutInfo.setLayoutCount = layouts.Size();
	passPipelineLayoutInfo.pSetLayouts = layouts.Data();
	passPipelineLayoutInfo.pushConstantRangeCount = push_constants.Size();
	passPipelineLayoutInfo.pPushConstantRanges = push_constants.Data();

	if (vkCreatePipelineLayout(device.GetDevice(), &passPipelineLayoutInfo, nullptr, &material->m_pipelineLayout) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}
	return true;
}

bool Hail::VlkMaterialManager::CreateGraphicsPipeline(VlkMaterial& vlkMaterial)
{
	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	//TODO: make a wireframe toggle for materials
	const bool isWireFrame = vlkMaterial.m_type == eMaterialType::DEBUG_LINES2D || vlkMaterial.m_type == eMaterialType::DEBUG_LINES3D;

	bool renderDepth = false;
	VkVertexInputBindingDescription vertexBindingDescription = VkVertexInputBindingDescription();
	GrowingArray<VkVertexInputAttributeDescription> vertexAttributeDescriptions(1);
	switch (vlkMaterial.m_type)
	{
	case eMaterialType::SPRITE:
		vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::SPRITE);
		vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::SPRITE);
		break;
	case eMaterialType::FULLSCREEN_PRESENT_LETTERBOX:
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;
		attributeDescription.location = 0;
		attributeDescription.format = VK_FORMAT_R32_UINT;
		attributeDescription.offset = 0;
		vertexAttributeDescriptions.Add(attributeDescription);
	}
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = sizeof(uint32_t);
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	break;
	case eMaterialType::MODEL3D:
		renderDepth = true;
		vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::MODEL);
		vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::MODEL);
		break;
	case eMaterialType::DEBUG_LINES2D:
	case eMaterialType::DEBUG_LINES3D:
		vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::SPRITE);
		vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::SPRITE);
		break;
	default:
		break;
	}

	// TODO: assert if sahders is not 2
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	for (size_t i = 0; i < vlkMaterial.m_pShaders.Size(); i++)
	{
		if ((eShaderType)vlkMaterial.m_pShaders[i]->header.shaderType == eShaderType::Vertex)
		{
			vertShaderModule = CreateShaderModule(*vlkMaterial.m_pShaders[i], device);
		}
		else if ((eShaderType)vlkMaterial.m_pShaders[i]->header.shaderType == eShaderType::Fragment)
		{
			fragShaderModule = CreateShaderModule(*vlkMaterial.m_pShaders[i], device);
		}
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkDynamicState dynamicStates[2] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = isWireFrame ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	const glm::uvec2 resolution = vlkMaterial.m_frameBufferTextures->GetResolution();
	const VkExtent2D passExtent = { resolution.x, resolution.y };
	viewport.width = (float)passExtent.width;
	viewport.height = (float)passExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = passExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = isWireFrame ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = CreateColorBlendAttachment(vlkMaterial);

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = renderDepth;
	depthStencil.depthWriteEnable = renderDepth;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = vlkMaterial.m_pipelineLayout;
	pipelineInfo.renderPass = vlkMaterial.m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.Size());
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.Data();

	if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vlkMaterial.m_pipeline) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}

	vkDestroyShaderModule(device.GetDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device.GetDevice(), vertShaderModule, nullptr);
	return true;
}

void Hail::VlkMaterialManager::AllocateTypeDescriptors(VlkMaterial& vlkMaterial, uint32 frameInFlight)
{
	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();
	GrowingArray< VkWriteDescriptorSet> setWrites(2);
	uint32_t bufferSize = 0;
	//m_descriptorImageInfo.imageView = VK_NULL_HANDLE;
	//m_descriptorImageInfo.sampler = VK_NULL_HANDLE;

	ReflectedShaderData* pSecondaryShaderData = nullptr;
	if (vlkMaterial.m_pShaders.Size() > 1)
	{
		pSecondaryShaderData = &vlkMaterial.m_pShaders[1]->reflectedShaderData;
	}

	VlkMaterialTypeDescriptor& typeDescriptor = *(VlkMaterialTypeDescriptor*)m_materialTypeDescriptors[(uint32)vlkMaterial.m_type];

	GrowingArray<VlkLayoutDescriptor> globalDescriptors = localGetSetLayoutDescription(
		(eShaderType)vlkMaterial.m_pShaders[0]->header.shaderType,
		GlobalDomain, &vlkMaterial.m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);

	VectorOnStack<VkDescriptorBufferInfo, 8> bufferDescriptorInfos;
	VectorOnStack<VkDescriptorImageInfo, 8> imageDescriptorInfos;

	for (size_t i = 0; i < globalDescriptors.Size(); i++)
	{
		VlkLayoutDescriptor& descriptor = globalDescriptors[i];
		if (descriptor.decorationType == eDecorationType::UniformBuffer)
		{
			VkDescriptorBufferInfo& bufferDescriptorInfo = bufferDescriptorInfos.Add();
			VlkBufferObject* vlkBuffer = (VlkBufferObject*)m_renderingResourceManager->GetBuffer(GlobalDomain, eBufferType::uniform, descriptor.bindingPoint);
			bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
			bufferDescriptorInfo.offset = vlkBuffer->GetProperties().offset;
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			setWrites.Add(WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, typeDescriptor.m_globalDescriptors[frameInFlight], bufferDescriptorInfo, descriptor.bindingPoint));
			
		}
		else if (descriptor.decorationType == eDecorationType::ShaderStorageBuffer)
		{
			VkDescriptorBufferInfo& bufferDescriptorInfo = bufferDescriptorInfos.Add();
			VlkBufferObject* vlkBuffer = (VlkBufferObject*)m_renderingResourceManager->GetBuffer(GlobalDomain, eBufferType::structured, descriptor.bindingPoint);
			bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
			bufferDescriptorInfo.offset = vlkBuffer->GetProperties().offset;
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			setWrites.Add(WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, typeDescriptor.m_globalDescriptors[frameInFlight], bufferDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::SampledImage)
		{
			VkDescriptorImageInfo& imageDescriptorInfo = imageDescriptorInfos.Add();
			VlkTextureResource* vlkTexture = (VlkTextureResource*)m_textureManager->GetEngineTexture(GlobalDomain, descriptor.bindingPoint, frameInFlight);
			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = vlkTexture->GetVlkTextureData().textureImageView;
			imageDescriptorInfo.sampler = vlkRenderingResources->m_pointTextureSampler;
			setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_globalDescriptors[frameInFlight], imageDescriptorInfo, descriptor.bindingPoint));
		}
	}
	GrowingArray<VlkLayoutDescriptor> materialDescriptors = localGetSetLayoutDescription(
		(eShaderType)vlkMaterial.m_pShaders[0]->header.shaderType,
		MaterialTypeDomain, &vlkMaterial.m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
	for (size_t i = 0; i < materialDescriptors.Size(); i++)
	{
		VlkLayoutDescriptor& descriptor = materialDescriptors[i];
		if (descriptor.decorationType == eDecorationType::UniformBuffer)
		{
			VkDescriptorBufferInfo& bufferDescriptorInfo = bufferDescriptorInfos.Add();
			VlkBufferObject* vlkBuffer = (VlkBufferObject*)m_renderingResourceManager->GetBuffer(MaterialTypeDomain, eBufferType::uniform, descriptor.bindingPoint);
			bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
			bufferDescriptorInfo.offset = vlkBuffer->GetProperties().offset;
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			setWrites.Add(WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, typeDescriptor.m_typeDescriptors[frameInFlight], bufferDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::ShaderStorageBuffer)
		{
			VkDescriptorBufferInfo& bufferDescriptorInfo = bufferDescriptorInfos.Add();
			VlkBufferObject* vlkBuffer = (VlkBufferObject*)m_renderingResourceManager->GetBuffer(MaterialTypeDomain, eBufferType::structured, descriptor.bindingPoint);
			bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
			bufferDescriptorInfo.offset = vlkBuffer->GetProperties().offset;
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			setWrites.Add(WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, typeDescriptor.m_typeDescriptors[frameInFlight], bufferDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::SampledImage)
		{
			VkDescriptorImageInfo& imageDescriptorInfo = imageDescriptorInfos.Add();
			VlkTextureResource* vlkTexture = (VlkTextureResource*)m_textureManager->GetEngineTexture(MaterialTypeDomain, descriptor.bindingPoint, frameInFlight);
			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = vlkTexture->GetVlkTextureData().textureImageView;
			imageDescriptorInfo.sampler = vlkRenderingResources->m_pointTextureSampler;
			setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_typeDescriptors[frameInFlight], imageDescriptorInfo, descriptor.bindingPoint));
		}
	}
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	if (setWrites.Size() > 0)
	{
		vkUpdateDescriptorSets(device.GetDevice(), setWrites.Size(), setWrites.Data(), 0, nullptr);
	}
}

bool VlkMaterialManager::CreateFramebuffers(VlkMaterial& vlkMaterial, uint32 frameInFlight)
{
	if (vlkMaterial.m_type == eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
	{
		VlkSwapChain* swapChain = (VlkSwapChain*)m_swapChain;
		vlkMaterial.m_ownsFrameBuffer = false;

		vlkMaterial.m_frameBuffer[frameInFlight] = swapChain->GetFrameBuffer(frameInFlight);
		vlkMaterial.m_renderPass = swapChain->GetRenderPass();
	}
	else
	{
		VlkDevice& device = *(VlkDevice*)(m_renderDevice);
		FrameBufferTextureData colorTexture = vlkMaterial.m_frameBufferTextures->GetTextureImage(frameInFlight);
		FrameBufferTextureData depthTexture = vlkMaterial.m_frameBufferTextures->GetDepthTextureImage(frameInFlight);
		VkImageView attachments[2] = { colorTexture.imageView, depthTexture.imageView };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vlkMaterial.m_renderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = vlkMaterial.m_frameBufferTextures->GetResolution().x;
		framebufferInfo.height = vlkMaterial.m_frameBufferTextures->GetResolution().y;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &vlkMaterial.m_frameBuffer[frameInFlight]) != VK_SUCCESS)
		{
			//TODO ASSERT
			return false;
		}
	}
	return true;
}


bool VlkMaterialManager::InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight, bool isDefaultMaterialInstance)
{
	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();

	VlkMaterial* pMat = (VlkMaterial*)m_materials[(uint8)instance.m_materialType][instance.m_materialIndex];

	//VlkPassData& passData = pMat->m_passData;
	ResourceValidator& validator = isDefaultMaterialInstance ? GetDefaultMaterialValidator(instance.m_materialType) : m_materialsInstanceValidationData[instance.m_instanceIdentifier];
	
	if (validator.GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		VkInternalMaterialDescriptorSet setAllocLayouts{};
		setAllocLayouts.descriptors[0] = VK_NULL_HANDLE;
		setAllocLayouts.descriptors[1] = VK_NULL_HANDLE;
 		GrowingArray<VkDescriptorSetLayout> setLayouts(MAX_FRAMESINFLIGHT, pMat->m_instanceSetLayout);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = vlkRenderingResources->m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = (uint32_t)MAX_FRAMESINFLIGHT;
		passAllocInfo.pSetLayouts = setLayouts.Data();
		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, setAllocLayouts.descriptors) != VK_SUCCESS)
		{
			//TODO add assert
			return false;
		}
		instance.m_gpuResourceInstance = pMat->m_instanceDescriptors.Size();
		pMat->m_instanceDescriptors.Add(setAllocLayouts);
	}


	ReflectedShaderData* pSecondaryShaderData = nullptr;
	if (pMat->m_pShaders.Size() > 1)
	{
		pSecondaryShaderData = &pMat->m_pShaders[1]->reflectedShaderData;
	}


	GrowingArray<VlkLayoutDescriptor> instanceDescriptors = localGetSetLayoutDescription(
		(eShaderType)pMat->m_pShaders[0]->header.shaderType,
		InstanceDomain, &pMat->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);

	GrowingArray<VkWriteDescriptorSet> descriptorWrites(instanceDescriptors.Size());

	GrowingArray<VkDescriptorImageInfo> imageDescriptorInfos;

	uint32 textureIndex = 0;
	for (size_t i = 0; i < instanceDescriptors.Size(); i++)
	{
		VlkLayoutDescriptor& descriptor = instanceDescriptors[i];
		if (descriptor.decorationType == eDecorationType::UniformBuffer)
		{
			// ASSERT as I do not have buffer support yet in Set 2
		}
		else if (descriptor.decorationType == eDecorationType::ShaderStorageBuffer)
		{
			// ASSERT as I do not have buffer support yet in Set 2
		}
		else if (descriptor.decorationType == eDecorationType::SampledImage)
		{

			VlkTextureResource* pVlkTexture = instance.m_textureHandles[textureIndex] != MAX_UINT ? 
				(VlkTextureResource*)m_textureManager->GetTexture(instance.m_textureHandles[textureIndex]) : 
				(VlkTextureResource*)m_textureManager->GetDefaultTexture();

			VkDescriptorImageInfo& imageDescriptorInfo = imageDescriptorInfos.Add();

			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = pVlkTexture->GetVlkTextureData().textureImageView;
			imageDescriptorInfo.sampler = vlkRenderingResources->m_linearTextureSampler;

			VkWriteDescriptorSet& writeDescriptor = descriptorWrites.Add();

			writeDescriptor = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				pMat->m_instanceDescriptors[instance.m_gpuResourceInstance].descriptors[frameInFlight],
				imageDescriptorInfo, descriptor.bindingPoint);

			textureIndex++;

		}
	}
	vkUpdateDescriptorSets(device.GetDevice(), (uint32)descriptorWrites.Size(), descriptorWrites.Data(), 0, nullptr);

	return true;
}

void Hail::VlkMaterialManager::ClearMaterialInternal(Material* pMaterial, uint32 frameInFlight)
{
	pMaterial->m_validator.MarkResourceAsDirty(frameInFlight);
	if (pMaterial->m_validator.GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		pMaterial->CleanupResource(*m_renderDevice);
	}
	pMaterial->CleanupResourceFrameData(*m_renderDevice, frameInFlight);
}

Material* Hail::VlkMaterialManager::CreateUnderlyingMaterial()
{
	return new VlkMaterial();
}
