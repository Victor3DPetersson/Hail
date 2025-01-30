#include "Engine_PCH.h"
#include "VlkMaterialManager.h"

#include "Windows\VulkanInternal\VlkDevice.h"
#include "Windows\VulkanInternal\VlkSwapChain.h"
#include "Windows\VulkanInternal\VlkVertex_Descriptor.h"
#include "VlkBufferResource.h"
#include "Windows\VulkanInternal\VlkResourceManager.h"
#include "VlkTextureManager.h"
#include "VlkMaterial.h"
#include "Rendering\RenderContext.h"

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

	VkPipelineColorBlendAttachmentState LocalCreateBlendMode(eBlendMode blendMode)
	{
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = (blendMode == eBlendMode::Translucent || blendMode == eBlendMode::Additive) ? VK_TRUE : VK_FALSE;
		switch (blendMode)
		{
		case Hail::eBlendMode::None:
		case Hail::eBlendMode::Cutout:
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			break;
		case Hail::eBlendMode::Translucent:
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			break;
		case Hail::eBlendMode::Additive:
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			break;
		default:
			break;
		}
		return colorBlendAttachment;
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
		else if (primaryShaderType == eShaderType::Mesh)
		{
			flagBit = VK_SHADER_STAGE_MESH_BIT_EXT;
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

	VkWriteDescriptorSet WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* pBufferInfo, uint32_t binding)
	{
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;

		write.dstBinding = binding;
		write.dstSet = dstSet;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pBufferInfo = pBufferInfo;
		return write;
	}

	VkWriteDescriptorSet WriteDescriptorSampler(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* pBufferInfo, uint32_t binding)
	{
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;

		write.dstBinding = binding;
		write.dstSet = dstSet;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pImageInfo = pBufferInfo;
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

	struct DescriptorInfos
	{
		VectorOnStack<VkDescriptorBufferInfo, 8> bufferDescriptorInfos; 
		VectorOnStack<VkDescriptorImageInfo, 8> imageDescriptorInfos;
	};

	GrowingArray<VkWriteDescriptorSet> localGetGlobalPipelineDescriptor(DescriptorInfos& outDescriptorInfos, const VlkPipeline& vlkPipeline, ReflectedShaderData* pSecondaryShaderData, VlkMaterialTypeObject& typeDescriptor, 
		TextureManager* pTextureResourceManager, RenderingResourceManager* pRenderingResourceManager, uint32 frameInFlight)
	{
		GrowingArray<VkWriteDescriptorSet> outSetWrites(8);
		GrowingArray<VlkLayoutDescriptor> globalDescriptors = localGetSetLayoutDescription(
			(eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType,
			GlobalDomain, &vlkPipeline.m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
		VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)pRenderingResourceManager)->GetRenderingResources();

		// TODO flytta ut globala descriptors till en egen funktion.
		for (size_t i = 0; i < globalDescriptors.Size(); i++)
		{
			VlkLayoutDescriptor& descriptor = globalDescriptors[i];
			if (descriptor.decorationType == eDecorationType::UniformBuffer || descriptor.decorationType == eDecorationType::ShaderStorageBuffer)
			{
				VkDescriptorBufferInfo& bufferDescriptorInfo = outDescriptorInfos.bufferDescriptorInfos.Add();
				eBufferType bufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? eBufferType::structured : eBufferType::uniform;
				VlkBufferObject* vlkBuffer = (VlkBufferObject*)pRenderingResourceManager->GetGlobalBuffer(GlobalDomain, bufferType, descriptor.bindingPoint);
				bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
				bufferDescriptorInfo.offset = vlkBuffer->GetProperties().offset;
				bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();

				VkDescriptorType vkBufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				outSetWrites.Add(WriteDescriptorBuffer(vkBufferType, typeDescriptor.m_globalDescriptors[frameInFlight], &bufferDescriptorInfo, descriptor.bindingPoint));

			}
			else if (descriptor.decorationType == eDecorationType::SampledImage)
			{
				VkDescriptorImageInfo& imageDescriptorInfo = outDescriptorInfos.imageDescriptorInfos.Add();
				VlkTextureView* vlkTexture = (VlkTextureView*)pTextureResourceManager->GetEngineTextureView(GlobalDomain, descriptor.bindingPoint, frameInFlight);
				imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageDescriptorInfo.imageView = vlkTexture->GetVkImageView();
				imageDescriptorInfo.sampler = vlkRenderingResources->m_pointTextureSampler;
				outSetWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_globalDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
			}
		}
		return outSetWrites;
	}
}

void VlkMaterialManager::Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain)
{
	MaterialManager::Init(renderingDevice, textureResourceManager, renderingResourceManager, swapChain);
}



void Hail::VlkMaterialManager::BindPipelineToContext(Pipeline* pPipeline, RenderContext* pRenderContext)
{
	H_ASSERT(pPipeline, "Must have a valid pipeline");
	VlkPipeline& vlkPipeline = *(VlkPipeline*)pPipeline;
	uint32_t bufferSize = 0;

	const uint32 frameInFlight = m_swapChain->GetFrameInFlight();

	ReflectedShaderData* pSecondaryShaderData = nullptr;
	if (vlkPipeline.m_pShaders.Size() > 1)
	{
		pSecondaryShaderData = &vlkPipeline.m_pShaders[1]->reflectedShaderData;
	}

	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();
	VlkMaterialTypeObject& typeDescriptor = *(VlkMaterialTypeObject*)vlkPipeline.m_pTypeDescriptor;

	if (typeDescriptor.m_bBoundTypeData[frameInFlight])
		return;

	DescriptorInfos descriptorInfos{};

	GrowingArray< VkWriteDescriptorSet> setWrites = localGetGlobalPipelineDescriptor(descriptorInfos, vlkPipeline, pSecondaryShaderData, typeDescriptor, m_textureManager, m_renderingResourceManager, frameInFlight);

	GrowingArray<VlkLayoutDescriptor> materialDescriptors = localGetSetLayoutDescription(
		(eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType, MaterialTypeDomain, &vlkPipeline.m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);

	for (size_t i = 0; i < materialDescriptors.Size(); i++)
	{
		VlkLayoutDescriptor& descriptor = materialDescriptors[i];
		if (descriptor.decorationType == eDecorationType::UniformBuffer || descriptor.decorationType == eDecorationType::ShaderStorageBuffer)
		{
			VkDescriptorBufferInfo& bufferDescriptorInfo = descriptorInfos.bufferDescriptorInfos.Add();
			VlkBufferObject* vlkBuffer = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? (VlkBufferObject*)pRenderContext->GetBoundStructuredBufferAtSlot(descriptor.bindingPoint) 
				: (VlkBufferObject*)pRenderContext->GetBoundUniformBufferAtSlot(descriptor.bindingPoint);

			H_ASSERT(vlkBuffer, "Nothing bound to the context slot.");

			bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
			bufferDescriptorInfo.offset = vlkBuffer->GetProperties().offset;
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			VkDescriptorType vkBufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setWrites.Add(WriteDescriptorBuffer(vkBufferType, typeDescriptor.m_typeDescriptors[frameInFlight], &bufferDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::SampledImage)
		{
			VkDescriptorImageInfo& imageDescriptorInfo = descriptorInfos.imageDescriptorInfos.Add();
			VlkTextureView* vlkTexture = (VlkTextureView*)pRenderContext->GetBoundTextureAtSlot(descriptor.bindingPoint);

			H_ASSERT(vlkTexture, "Nothing bound to the texture context slot.");

			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = vlkTexture->GetVkImageView();
			imageDescriptorInfo.sampler = vlkRenderingResources->m_pointTextureSampler;
			setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_typeDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
		}
	}
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	if (setWrites.Size() > 0)
	{
		vkUpdateDescriptorSets(device.GetDevice(), setWrites.Size(), setWrites.Data(), 0, nullptr);
	}
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

	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	VlkMaterial* pVlkMat = (VlkMaterial*)pMaterial;
	VlkPipeline* pVlkPipeline = (VlkPipeline*)pMaterial->m_pPipeline;

	const uint32 materialBaseType = (uint32)pVlkPipeline->m_type;

	ResourceValidator& materialDataValidator = pVlkMat->m_validator;
	if (materialDataValidator.GetIsResourceDirty())
	{
		if (!materialDataValidator.GetIsFrameDataDirty(frameInFlight))
		{
			//Add fatal assert here that something have gone wrong
			return false;
		}
	}

	pVlkPipeline->m_frameBufferTextures = m_passesFrameBufferTextures[materialBaseType];

	if (materialDataValidator.GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		if (!pVlkMat->m_instanceSetLayout)
		{
			ReflectedShaderData* pSecondaryShaderData = nullptr;
			if (pVlkPipeline->m_pShaders.Size() > 1)
			{
				pSecondaryShaderData = &pVlkPipeline->m_pShaders[1]->reflectedShaderData;
			}
			GrowingArray<VlkLayoutDescriptor> materialSetLayoutDescriptors = localGetSetLayoutDescription((eShaderType)pVlkPipeline->m_pShaders[0]->header.shaderType, InstanceDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
			if (!localCreateSetLayoutDescriptor(materialSetLayoutDescriptors, pVlkMat->m_instanceSetLayout, device))
			{
				return false;
			}
		}
		else
		{
			// TODO assert here
		}
		if (!CreateMaterialTypeObject(pVlkPipeline))
		{
			// TODO assert
			return false;
		}

		// TODO: assert on !pMaterial->m_pTypeDescriptor

		// TODO: if reloading a material and the materials layout is different, this needs to be looked over
		if (!CreatePipelineLayout(*pVlkPipeline, pVlkMat))
		{
			// TODO assert
			return false;
		}

		// Create the render pass
		if (!CreateRenderpass(*pVlkPipeline, false))
		{
			return false;
		}
	}
	if (!CreateFramebuffers(*pVlkPipeline, frameInFlight))
	{
		return false;
	}
	AllocateTypeDescriptors(*pVlkPipeline, frameInFlight);

	//for debugging, check so that once everyFrameDataIsDirty the validator is set to not dirty
	materialDataValidator.ClearFrameData(frameInFlight);

	if (materialDataValidator.GetFrameThatMarkedFrameDirty() != frameInFlight)
	{
		//Early out as below the resources are only created for the first frame in flight
		return true;
	}

	if (!CreateGraphicsPipeline(*pVlkPipeline))
	{
		return false;
	}
	return true;
}

bool Hail::VlkMaterialManager::InitMaterialPipelineInternal(MaterialPipeline* pMaterialPipeline, uint32 frameInFlight)
{

	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	VlkMaterialPipeline& vlkMatPipeline = *(VlkMaterialPipeline*)pMaterialPipeline;
	VlkPipeline& vlkPipeline = *(VlkPipeline*)pMaterialPipeline->m_pPipeline;
	const uint32 materialBaseType = (uint32)vlkPipeline.m_type;

	ResourceValidator& materialDataValidator = vlkMatPipeline.m_validator;
	if (materialDataValidator.GetIsResourceDirty())
	{
		if (!materialDataValidator.GetIsFrameDataDirty(frameInFlight))
		{
			H_ASSERT(false, "validator is in a broken state, check the logic dumb dumb.");
			return false;
		}
	}

	if (vlkPipeline.m_bUseTypeRenderPasses)
		vlkPipeline.m_frameBufferTextures = m_passesFrameBufferTextures[(uint32)vlkPipeline.m_typeRenderPass];

	if (materialDataValidator.GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		if (!CreateMaterialTypeObject(&vlkPipeline))
		{
			H_ASSERT(false, "Failed to create pipeline");
			return false;
		}

		// TODO: if reloading a material and the materials layout is different, this needs to be looked over
		if (!CreatePipelineLayout(vlkPipeline, nullptr))
		{
			// TODO assert
			return false;
		}

		//TODO: Updatera interfacet på denna funktionen
		if (!CreateRenderpass(vlkPipeline, true))
		{
			return false;
		}

	}
	if (!CreateFramebuffers(vlkPipeline, frameInFlight))
	{
		return false;
	}

	//for debugging, check so that once everyFrameDataIsDirty the validator is set to not dirty
	materialDataValidator.ClearFrameData(frameInFlight);

	if (materialDataValidator.GetFrameThatMarkedFrameDirty() != frameInFlight)
	{
		//Early out as below the resources are only created for the first frame in flight
		return true;
	}

	if (vlkPipeline.m_bIsCompute)
	{
		// TODO: Create compute pipeline
	}
	else
	{
		if (!CreateGraphicsPipeline(vlkPipeline))
		{
			return false;
		}
	}

	return true;
}

bool Hail::VlkMaterialManager::CreateRenderpass(VlkPipeline& vlkPipeline, bool bSpecialCase)
{
	if (vlkPipeline.m_bIsCompute)
		return true;

	VlkMaterialTypeObject& typeDescriptor = *(VlkMaterialTypeObject*)m_MaterialTypeObjects[(uint32)vlkPipeline.m_type];
	if (vlkPipeline.m_typeRenderPass == eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
	{
		VlkSwapChain* swapChain = (VlkSwapChain*)m_swapChain;
		vlkPipeline.m_ownsRenderpass = false;
		vlkPipeline.m_renderPass = swapChain->GetRenderPass();
		return true;
	}

	// TODO: If vlkPipeline.m_typeRenderPass == Custom, do something weird with that shit :) 

	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	GrowingArray<VkAttachmentDescription> attachmentDescriptors(2);
	VkAttachmentReference colorAttachmentRef{};
	VkAttachmentReference depthAttachmentRef{};
	switch (vlkPipeline.m_typeRenderPass)
	{
		case Hail::eMaterialType::SPRITE:
		{
			VkAttachmentDescription colorAttachment{};
			VkAttachmentDescription depthAttachment{};

			colorAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetTextureFormat());
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = bSpecialCase ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(colorAttachment);

			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = bSpecialCase ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			depthAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetDepthFormat());
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
			colorAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetTextureFormat());
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

			depthAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetDepthFormat());
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
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

			colorAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetTextureFormat());
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = bSpecialCase ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDescriptors.Add(colorAttachment);

			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = bSpecialCase ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			depthAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetDepthFormat());
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

			colorAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetTextureFormat());
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

			depthAttachment.format = ToVkFormat(vlkPipeline.m_frameBufferTextures->GetDepthFormat());
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
	if (vkCreateRenderPass(device.GetDevice(), &renderPassInfo, nullptr, &vlkPipeline.m_renderPass) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}
	return true;
}

bool Hail::VlkMaterialManager::CreateMaterialTypeObject(Pipeline* pPipeline)
{
	VlkPipeline* pVlkPipeline = (VlkPipeline*)pPipeline;

	if (pVlkPipeline->m_bUseTypePasses && m_MaterialTypeObjects[(uint32)pVlkPipeline->m_type])
	{
		// TODO: Check if the reflected shader data matches, otherwise assert
		pVlkPipeline->m_pTypeDescriptor = m_MaterialTypeObjects[(uint32)pVlkPipeline->m_type];
		return true;
	}

	MaterialTypeObject** pTypeObjectToCreate = pVlkPipeline->m_bUseTypePasses ? &m_MaterialTypeObjects[(uint32)pVlkPipeline->m_type] : &pVlkPipeline->m_pTypeDescriptor;
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	(*pTypeObjectToCreate) = new VlkMaterialTypeObject();
	VlkMaterialTypeObject& typeDescriptor = *(VlkMaterialTypeObject*)(*pTypeObjectToCreate);
	typeDescriptor.m_type = pVlkPipeline->m_type;
	if (pVlkPipeline->m_pShaders[0]->loadState == eShaderLoadState::LoadedToRAM)
	{
		const eShaderType shader1Type = (eShaderType)pVlkPipeline->m_pShaders[0]->header.shaderType;

		ReflectedShaderData* pSecondaryShaderData = nullptr;
		if (pVlkPipeline->m_pShaders.Size() > 1)
		{
			pSecondaryShaderData = &pVlkPipeline->m_pShaders[1]->reflectedShaderData;
		}

		GrowingArray<VlkLayoutDescriptor> globalSetDescriptors = localGetSetLayoutDescription((eShaderType)pVlkPipeline->m_pShaders[0]->header.shaderType, GlobalDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
		if (!localCreateSetLayoutDescriptor(globalSetDescriptors, typeDescriptor.m_globalSetLayout, device))
		{
			return false;
		}
		GrowingArray<VlkLayoutDescriptor> typeSetDescriptors = localGetSetLayoutDescription((eShaderType)pVlkPipeline->m_pShaders[0]->header.shaderType, MaterialTypeDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
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

	return true;
}


bool Hail::VlkMaterialManager::CreatePipelineLayout(VlkPipeline& vlkPipeline, VlkMaterial* pMaterial)
{
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	//setup push constants
	VectorOnStack<VkPushConstantRange, 4> push_constants;

	VectorOnStack<ShaderDecoration, 8>* reflectedPushConstants = &vlkPipeline.m_pShaders[0]->reflectedShaderData.m_pushConstants;

	if ((eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType == eShaderType::Fragment ||
		(eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType == eShaderType::Amplification)
	{
		reflectedPushConstants = &vlkPipeline.m_pShaders[1]->reflectedShaderData.m_pushConstants;
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
		if ((eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType == eShaderType::Vertex)
		{
			pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if ((eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType == eShaderType::Compute)
		{
			pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		else if ((eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType == eShaderType::Mesh)
		{
			pushConstant.stageFlags = VK_SHADER_STAGE_MESH_BIT_NV;
		}
		push_constants.Add(pushConstant);
	}

	MaterialTypeObject* pTypeObject = vlkPipeline.m_bUseTypePasses ? m_MaterialTypeObjects[(uint32)vlkPipeline.m_type] : vlkPipeline.m_pTypeDescriptor;
	VlkMaterialTypeObject& typeDescriptor = *(VlkMaterialTypeObject*)pTypeObject;
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
	if (material && material->m_instanceSetLayout != VK_NULL_HANDLE)
	{
		layouts.Add(material->m_instanceSetLayout);
	}

	VkPipelineLayoutCreateInfo passPipelineLayoutInfo{};
	passPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	passPipelineLayoutInfo.setLayoutCount = layouts.Size();
	passPipelineLayoutInfo.pSetLayouts = layouts.Data();
	passPipelineLayoutInfo.pushConstantRangeCount = push_constants.Size();
	passPipelineLayoutInfo.pPushConstantRanges = push_constants.Data();

	if (vkCreatePipelineLayout(device.GetDevice(), &passPipelineLayoutInfo, nullptr, &vlkPipeline.m_pipelineLayout) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}
	return true;
}

bool Hail::VlkMaterialManager::CreateGraphicsPipeline(VlkPipeline& vlkPipeline)
{
	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	//TODO: make a wireframe toggle for materials
	const bool isWireFrame = vlkPipeline.m_type == eMaterialType::DEBUG_LINES2D || vlkPipeline.m_type == eMaterialType::DEBUG_LINES3D;

	bool renderDepth = false;
	VkVertexInputBindingDescription vertexBindingDescription = VkVertexInputBindingDescription();
	GrowingArray<VkVertexInputAttributeDescription> vertexAttributeDescriptions(1);
	switch (vlkPipeline.m_type)
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

	H_ASSERT(vlkPipeline.m_pShaders.Size() == 2, "Insufficient amount of shaders in the material");
	VkShaderModule vertShaderModule;
	bool bUsesMeshShaders = false;
	VkShaderModule fragShaderModule;
	for (size_t i = 0; i < vlkPipeline.m_pShaders.Size(); i++)
	{
		if ((eShaderType)vlkPipeline.m_pShaders[i]->header.shaderType == eShaderType::Vertex)
		{
			vertShaderModule = CreateShaderModule(*vlkPipeline.m_pShaders[i], device);
		}
		else if ((eShaderType)vlkPipeline.m_pShaders[i]->header.shaderType == eShaderType::Fragment)
		{
			fragShaderModule = CreateShaderModule(*vlkPipeline.m_pShaders[i], device);
		}
		else if ((eShaderType)vlkPipeline.m_pShaders[i]->header.shaderType == eShaderType::Mesh)
		{
			vertShaderModule = CreateShaderModule(*vlkPipeline.m_pShaders[i], device);
			bUsesMeshShaders = true;
		}

	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = bUsesMeshShaders ? VK_SHADER_STAGE_MESH_BIT_EXT : VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	//l vertexInput, inputAssembly, vertexBindingDescriptions and vertexAttributes

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
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.Size());
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.Data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = isWireFrame ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	const glm::uvec2 resolution = vlkPipeline.m_frameBufferTextures->GetResolution();
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
	rasterizer.rasterizerDiscardEnable = VK_FALSE; // Good to turn off for debugging fragment shader bottlenecks
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

	VkPipelineColorBlendAttachmentState colorBlendAttachment = LocalCreateBlendMode(vlkPipeline.m_blendMode);

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
	pipelineInfo.pVertexInputState = bUsesMeshShaders ? nullptr : &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = bUsesMeshShaders ? nullptr : &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = vlkPipeline.m_pipelineLayout;
	pipelineInfo.renderPass = vlkPipeline.m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vlkPipeline.m_pipeline) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}

	vkDestroyShaderModule(device.GetDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device.GetDevice(), vertShaderModule, nullptr);
	return true;
}

void Hail::VlkMaterialManager::AllocateTypeDescriptors(VlkPipeline& vlkPipeline, uint32 frameInFlight)
{
	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();
	uint32_t bufferSize = 0;

	ReflectedShaderData* pSecondaryShaderData = nullptr;
	if (vlkPipeline.m_pShaders.Size() > 1)
	{
		pSecondaryShaderData = &vlkPipeline.m_pShaders[1]->reflectedShaderData;
	}

	VlkMaterialTypeObject& typeDescriptor = vlkPipeline.m_bUseTypePasses ? *(VlkMaterialTypeObject*)m_MaterialTypeObjects[(uint32)vlkPipeline.m_type] : *(VlkMaterialTypeObject*)vlkPipeline.m_pTypeDescriptor;

	DescriptorInfos descriptorInfos{};
	GrowingArray< VkWriteDescriptorSet> setWrites = localGetGlobalPipelineDescriptor(descriptorInfos, vlkPipeline, pSecondaryShaderData, typeDescriptor, m_textureManager, m_renderingResourceManager, frameInFlight);

	GrowingArray<VlkLayoutDescriptor> materialDescriptors = localGetSetLayoutDescription(
		(eShaderType)vlkPipeline.m_pShaders[0]->header.shaderType,
		MaterialTypeDomain, &vlkPipeline.m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
	for (size_t i = 0; i < materialDescriptors.Size(); i++)
	{
		VlkLayoutDescriptor& descriptor = materialDescriptors[i];
		if (descriptor.decorationType == eDecorationType::UniformBuffer || descriptor.decorationType == eDecorationType::ShaderStorageBuffer)
		{
			VkDescriptorBufferInfo& bufferDescriptorInfo = descriptorInfos.bufferDescriptorInfos.Add();
			eBufferType bufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? eBufferType::structured : eBufferType::uniform;
			VlkBufferObject* vlkBuffer = (VlkBufferObject*)m_renderingResourceManager->GetGlobalBuffer(MaterialTypeDomain, bufferType, descriptor.bindingPoint);
			bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
			bufferDescriptorInfo.offset = vlkBuffer->GetProperties().offset;
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			VkDescriptorType vkBufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setWrites.Add(WriteDescriptorBuffer(vkBufferType, typeDescriptor.m_typeDescriptors[frameInFlight], &bufferDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::SampledImage)
		{
			VkDescriptorImageInfo& imageDescriptorInfo = descriptorInfos.imageDescriptorInfos.Add();
			VlkTextureView* view = (VlkTextureView*)m_textureManager->GetEngineTextureView(MaterialTypeDomain, descriptor.bindingPoint, frameInFlight);
			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = view->GetVkImageView();
			imageDescriptorInfo.sampler = vlkRenderingResources->m_pointTextureSampler;
			setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_typeDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
		}
	}
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	if (setWrites.Size() > 0)
	{
		vkUpdateDescriptorSets(device.GetDevice(), setWrites.Size(), setWrites.Data(), 0, nullptr);
	}
	typeDescriptor.m_bBoundTypeData[frameInFlight] = true;

}

bool VlkMaterialManager::CreateFramebuffers(VlkPipeline& vlkPipeline, uint32 frameInFlight)
{
	if (vlkPipeline.m_bIsCompute)
		return true;


	if (vlkPipeline.m_typeRenderPass == eMaterialType::FULLSCREEN_PRESENT_LETTERBOX)
	{
		VlkSwapChain* swapChain = (VlkSwapChain*)m_swapChain;
		vlkPipeline.m_ownsFrameBuffer = false;

		vlkPipeline.m_frameBuffer[frameInFlight] = swapChain->GetFrameBuffer(frameInFlight);
		vlkPipeline.m_renderPass = swapChain->GetRenderPass();
	}
	else
	{
		VlkDevice& device = *(VlkDevice*)(m_renderDevice);
		VkImageView colorTexture = ((VlkTextureView*)vlkPipeline.m_frameBufferTextures->GetColorTextureView(frameInFlight))->GetVkImageView();
		VkImageView depthTexture = ((VlkTextureView*)vlkPipeline.m_frameBufferTextures->GetDepthTextureView(frameInFlight))->GetVkImageView();
		VkImageView attachments[2] = { colorTexture, depthTexture };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vlkPipeline.m_renderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = vlkPipeline.m_frameBufferTextures->GetResolution().x;
		framebufferInfo.height = vlkPipeline.m_frameBufferTextures->GetResolution().y;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &vlkPipeline.m_frameBuffer[frameInFlight]) != VK_SUCCESS)
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
	VlkPipeline* pVlkPipeline = (VlkPipeline*)pMat->m_pPipeline;

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
	if (pVlkPipeline->m_pShaders.Size() > 1)
	{
		pSecondaryShaderData = &pVlkPipeline->m_pShaders[1]->reflectedShaderData;
	}


	GrowingArray<VlkLayoutDescriptor> instanceDescriptors = localGetSetLayoutDescription(
		(eShaderType)pVlkPipeline->m_pShaders[0]->header.shaderType,
		InstanceDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);

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

			VlkTextureView* pVlkTexture = instance.m_textureHandles[textureIndex] != MAX_UINT ?
				(VlkTextureView*)m_textureManager->GetTextureView(instance.m_textureHandles[textureIndex]) :
				(VlkTextureView*)m_textureManager->GetDefaultTexture().m_pView;

			VkDescriptorImageInfo& imageDescriptorInfo = imageDescriptorInfos.Add();

			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = pVlkTexture->GetVkImageView();
			imageDescriptorInfo.sampler = vlkRenderingResources->m_linearTextureSampler;

			VkWriteDescriptorSet& writeDescriptor = descriptorWrites.Add();

			writeDescriptor = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				pMat->m_instanceDescriptors[instance.m_gpuResourceInstance].descriptors[frameInFlight],
				&imageDescriptorInfo, descriptor.bindingPoint);

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
	Material* pMaterial = new VlkMaterial();
	pMaterial->m_pPipeline = CreateUnderlyingPipeline();
	return pMaterial;
}

MaterialPipeline* Hail::VlkMaterialManager::CreateUnderlyingMaterialPipeline()
{
	MaterialPipeline* pMaterialPipe = new VlkMaterialPipeline();
	pMaterialPipe->m_pPipeline = CreateUnderlyingPipeline();
	return pMaterialPipe;
}

Pipeline* Hail::VlkMaterialManager::CreateUnderlyingPipeline()
{
	return new VlkPipeline();
}
