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
#include "Resources_Materials\ShaderBufferList.h"

using namespace Hail;

namespace
{
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
		const SetDecoration* decoration1,
		VkShaderStageFlags stageFlag1,
		const SetDecoration* decoration2,
		VkShaderStageFlags stageFlag2,
		VkDescriptorType descriptorType,
		GrowingArray<VlkLayoutDescriptor>& descriptorToFill)
	{
		VectorOnStack<VlkLayoutDescriptor, 16> descriptors;

		for (int i = 0; i < decoration1->m_indices.Size(); i++)
		{
			VlkLayoutDescriptor descriptor;
			const ShaderDecoration& decoration = decoration1->m_decorations[decoration1->m_indices[i]];
			descriptor.bindingPoint = decoration.m_bindingLocation;
			descriptor.type = descriptorType;
			descriptor.flags = stageFlag1;
			descriptor.decorationType = decoration.m_type;
			descriptors.Add(descriptor);
		}
		if (decoration2)
		{
			for (int i = 0; i < decoration2->m_indices.Size(); i++)
			{
				const ShaderDecoration& decoration = decoration2->m_decorations[decoration2->m_indices[i]];
				const uint32 bindingPoint = decoration.m_bindingLocation;
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
					descriptor.decorationType = decoration.m_type;
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
		const SetDecoration* decoration1,
		VkShaderStageFlags stageFlag1,
		const SetDecoration* decoration2,
		VkShaderStageFlags stageFlag2,
		VkDescriptorType descriptorType,
		GrowingArray<VlkLayoutDescriptor>& descriptorToFill,
		bool bCheckIfSampledOrStorageImage)
	{
		VectorOnStack<VlkLayoutDescriptor, 16> descriptors;

		for (int i = 0; i < decoration1->m_indices.Size(); i++)
		{
			VlkLayoutDescriptor descriptor;
			const ShaderDecoration& decoration = decoration1->m_decorations[decoration1->m_indices[i]];
			descriptor.bindingPoint = decoration.m_bindingLocation;
			descriptor.type = descriptorType;
			descriptor.flags = stageFlag1;
			descriptor.decorationType = decoration.m_type;
			if (bCheckIfSampledOrStorageImage)
			{
				// If the image is write in any way it is a storage image
				if (decoration.m_accessQualifier != eShaderAccessQualifier::ReadOnly)
				{
					descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				}
			}
			descriptors.Add(descriptor);
		}
		if (decoration2)
		{
			for (int i = 0; i < decoration2->m_indices.Size(); i++)
			{
				const ShaderDecoration& decoration = decoration2->m_decorations[decoration2->m_indices[i]];
				const uint32 bindingPoint = decoration.m_bindingLocation;
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
					descriptor.decorationType = decoration.m_type;
					if (bCheckIfSampledOrStorageImage)
					{
						// If the image is write in any way it is a storage image
						if (decoration.m_accessQualifier != eShaderAccessQualifier::ReadOnly)
						{
							descriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
						}
					}
					descriptors.Add(descriptor);
				}
			}
		}
		for (size_t i = 0; i < descriptors.Size(); i++)
		{
			descriptorToFill.Add(descriptors[i]);
		}
	}

	GrowingArray<VlkLayoutDescriptor> localGetSetLayoutDescription(eShaderStage primaryShaderType, eDecorationSets domainToFetch, ReflectedShaderData* pMainData, ReflectedShaderData* pSecondaryData)
	{
		GrowingArray<VlkLayoutDescriptor> typeSetDescriptors;
		VkShaderStageFlagBits flagBit = VK_SHADER_STAGE_ALL;
		VkShaderStageFlagBits secondaryFlagBit = VK_SHADER_STAGE_ALL;
		bool bUse2Shaders = false;
		if (primaryShaderType == eShaderStage::Compute)
		{
			flagBit = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		else if (primaryShaderType == eShaderStage::Fragment)
		{
			flagBit = VK_SHADER_STAGE_FRAGMENT_BIT;
			secondaryFlagBit = VK_SHADER_STAGE_VERTEX_BIT;
			bUse2Shaders = true;
		}
		else if (primaryShaderType == eShaderStage::Vertex)
		{
			flagBit = VK_SHADER_STAGE_VERTEX_BIT;
			secondaryFlagBit = VK_SHADER_STAGE_FRAGMENT_BIT;
			bUse2Shaders = true;
		}
		else if (primaryShaderType == eShaderStage::Mesh)
		{
			flagBit = VK_SHADER_STAGE_MESH_BIT_EXT;
			secondaryFlagBit = VK_SHADER_STAGE_FRAGMENT_BIT;
			bUse2Shaders = true;
		}
		H_ASSERT(bUse2Shaders ? (pSecondaryData != nullptr) : (pSecondaryData == nullptr));
		pMainData->m_setDecorations[domainToFetch];

		localGetDescriptorsFromDecoration(
			&pMainData->m_setDecorations[domainToFetch][(uint32)eDecorationType::UniformBuffer], flagBit,
			bUse2Shaders ? &pSecondaryData->m_setDecorations[domainToFetch][(uint32)eDecorationType::UniformBuffer] : nullptr, secondaryFlagBit,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, typeSetDescriptors);

		localGetDescriptorsFromDecoration(
			&pMainData->m_setDecorations[domainToFetch][(uint32)eDecorationType::ShaderStorageBuffer], flagBit,
			bUse2Shaders ? &pSecondaryData->m_setDecorations[domainToFetch][(uint32)eDecorationType::ShaderStorageBuffer] : nullptr, secondaryFlagBit,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, typeSetDescriptors);

		localGetImageDescriptorsFromDecoration(
			&pMainData->m_setDecorations[domainToFetch][(uint32)eDecorationType::SampledImage], flagBit,
			bUse2Shaders ? &pSecondaryData->m_setDecorations[domainToFetch][(uint32)eDecorationType::SampledImage] : nullptr, secondaryFlagBit,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeSetDescriptors, false);

		localGetImageDescriptorsFromDecoration(
			&pMainData->m_setDecorations[domainToFetch][(uint32)eDecorationType::Image], flagBit,
			bUse2Shaders ? &pSecondaryData->m_setDecorations[domainToFetch][(uint32)eDecorationType::Image] : nullptr, secondaryFlagBit,
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, typeSetDescriptors, true);

		localGetImageDescriptorsFromDecoration(
			&pMainData->m_setDecorations[domainToFetch][(uint32)eDecorationType::Sampler], flagBit,
			bUse2Shaders ? &pSecondaryData->m_setDecorations[domainToFetch][(uint32)eDecorationType::Sampler] : nullptr, secondaryFlagBit,
			VK_DESCRIPTOR_TYPE_SAMPLER, typeSetDescriptors, false);

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
		VectorOnStack<VkDescriptorImageInfo, 8> sampledImageDescriptorInfos;
		VectorOnStack<VkDescriptorImageInfo, 8> imageDescriptorInfos;
		VectorOnStack<VkDescriptorImageInfo, 8> samplerDescriptorInfos;
	};

	GrowingArray<VkWriteDescriptorSet> localGetGlobalPipelineDescriptor(DescriptorInfos& outDescriptorInfos, const VlkPipeline& vlkPipeline, ReflectedShaderData* pSecondaryShaderData, VlkMaterialTypeObject& typeDescriptor, 
		TextureManager* pTextureResourceManager, RenderingResourceManager* pRenderingResourceManager, uint32 frameInFlight)
	{
		GrowingArray<VkWriteDescriptorSet> outSetWrites(8);
		GrowingArray<VlkLayoutDescriptor> globalDescriptors = localGetSetLayoutDescription(
			(eShaderStage)vlkPipeline.m_pShaders[0]->header.shaderType,
			GlobalDomain, &vlkPipeline.m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
		VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)pRenderingResourceManager)->GetRenderingResources();

		for (size_t i = 0; i < globalDescriptors.Size(); i++)
		{
			VlkLayoutDescriptor& descriptor = globalDescriptors[i];
			if (descriptor.decorationType == eDecorationType::UniformBuffer || descriptor.decorationType == eDecorationType::ShaderStorageBuffer)
			{
				VkDescriptorBufferInfo& bufferDescriptorInfo = outDescriptorInfos.bufferDescriptorInfos.Add();
				eBufferType bufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? eBufferType::structured : eBufferType::uniform;
				VlkBufferObject* vlkBuffer = (VlkBufferObject*)pRenderingResourceManager->GetGlobalBuffer(GlobalDomain, bufferType, descriptor.bindingPoint);
				bufferDescriptorInfo.buffer = vlkBuffer->GetBuffer(frameInFlight);
				bufferDescriptorInfo.offset = 0u;
				bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();

				VkDescriptorType vkBufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				outSetWrites.Add(WriteDescriptorBuffer(vkBufferType, typeDescriptor.m_globalDescriptors[frameInFlight], &bufferDescriptorInfo, descriptor.bindingPoint));

			}
			else if (descriptor.decorationType == eDecorationType::SampledImage)
			{
				VkDescriptorImageInfo& imageDescriptorInfo = outDescriptorInfos.sampledImageDescriptorInfos.Add();
				VlkTextureView* vlkTexture = (VlkTextureView*)pTextureResourceManager->GetEngineTextureView(GlobalDomain, descriptor.bindingPoint, frameInFlight);
				imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageDescriptorInfo.imageView = vlkTexture->GetVkImageView();
				VlkSamplerObject* vlkSampler = (VlkSamplerObject*)pRenderingResourceManager->GetGlobalSampler(GlobalSamplers::Point);
				imageDescriptorInfo.sampler = vlkSampler->GetInternalSampler();
				outSetWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_globalDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
			}
			else if (descriptor.decorationType == eDecorationType::Sampler)
			{
				uint32 samplerBindingPoint = descriptor.bindingPoint - (uint32)eGlobalUniformBuffers::count;
				VlkSamplerObject* vlkSampler = (VlkSamplerObject*)pRenderingResourceManager->GetGlobalSampler((GlobalSamplers)samplerBindingPoint);
				VkDescriptorImageInfo& samplerDescriptorInfo = outDescriptorInfos.samplerDescriptorInfos.Add();
				samplerDescriptorInfo.sampler = vlkSampler->GetInternalSampler();
				outSetWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_SAMPLER, typeDescriptor.m_globalDescriptors[frameInFlight], &samplerDescriptorInfo, descriptor.bindingPoint));
			}
			else if (descriptor.decorationType == eDecorationType::Image)
			{
				VlkTextureView* vlkTexture = (VlkTextureView*)pTextureResourceManager->GetEngineTextureView(GlobalDomain, descriptor.bindingPoint, frameInFlight);
				VkDescriptorImageInfo& imageDescriptorInfo = outDescriptorInfos.imageDescriptorInfos.Add();
				imageDescriptorInfo.imageView = vlkTexture->GetVkImageView();
				imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				outSetWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, typeDescriptor.m_globalDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
			}
		}
		return outSetWrites;
	}
}

void VlkMaterialManager::Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain)
{
	MaterialManager::Init(renderingDevice, textureResourceManager, renderingResourceManager, swapChain);
}

void Hail::VlkMaterialManager::UpdateCustomPipelineDescriptors(Pipeline* pPipeline, RenderContext* pRenderContext)
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
	VlkMaterialTypeObject& typeDescriptor = *(VlkMaterialTypeObject*)vlkPipeline.m_pTypeObject;

	// Only Custom pipelines without a bounds global material and global set should be setting a manual pipeline 
	if (typeDescriptor.m_bBoundTypeData[frameInFlight])
		return;

	H_ASSERT(pPipeline->m_type == eMaterialType::CUSTOM, "Only custom pipelines should use this function");

	DescriptorInfos descriptorInfos{};

	GrowingArray<VkWriteDescriptorSet> setWrites = localGetGlobalPipelineDescriptor(descriptorInfos, vlkPipeline, pSecondaryShaderData, typeDescriptor, m_textureManager, m_renderingResourceManager, frameInFlight);

	GrowingArray<VlkLayoutDescriptor> materialDescriptors = localGetSetLayoutDescription(
		(eShaderStage)vlkPipeline.m_pShaders[0]->header.shaderType, MaterialTypeDomain, &vlkPipeline.m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);

	// Lägg till mera data för dem nya data typerna
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
			bufferDescriptorInfo.offset = 0u; // TODO: Använd view property här
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			VkDescriptorType vkBufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setWrites.Add(WriteDescriptorBuffer(vkBufferType, typeDescriptor.m_typeDescriptors[frameInFlight], &bufferDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::SampledImage)
		{
			VkDescriptorImageInfo& imageDescriptorInfo = descriptorInfos.sampledImageDescriptorInfos.Add();
			VlkTextureView* vlkTexture = (VlkTextureView*)pRenderContext->GetBoundTextureAtSlot(descriptor.bindingPoint);

			H_ASSERT(vlkTexture, "Nothing bound to the texture context slot.");

			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = vlkTexture->GetVkImageView();
			imageDescriptorInfo.sampler = ((VlkSamplerObject*)m_renderingResourceManager->GetGlobalSampler(GlobalSamplers::Point))->GetInternalSampler();
			setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_typeDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));

		}
		else if (descriptor.decorationType == eDecorationType::Image)
		{
			VkDescriptorImageInfo& imageDescriptorInfo = descriptorInfos.imageDescriptorInfos.Add();
			VlkTextureView* vlkTextureView = (VlkTextureView*)pRenderContext->GetBoundTextureAtSlot(descriptor.bindingPoint);

			H_ASSERT(vlkTextureView, "Nothing bound to the texture context slot.");

			imageDescriptorInfo.imageLayout = ((VlkTextureResource*)vlkTextureView->GetProps().pTextureToView)->GetVlkTextureData().imageLayout;
			imageDescriptorInfo.imageView = vlkTextureView->GetVkImageView();
			if (vlkTextureView->GetProps().accessQualifier != eShaderAccessQualifier::ReadOnly)
			{
				H_ASSERT(imageDescriptorInfo.imageLayout == VK_IMAGE_LAYOUT_GENERAL);
				setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, typeDescriptor.m_typeDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
			}
			else
			{
				H_ASSERT(imageDescriptorInfo.imageLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
				setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, typeDescriptor.m_typeDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
			}
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

	if (materialDataValidator.GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		if (!pVlkMat->m_instanceSetLayout)
		{
			ReflectedShaderData* pSecondaryShaderData = nullptr;
			if (pVlkPipeline->m_pShaders.Size() > 1)
			{
				pSecondaryShaderData = &pVlkPipeline->m_pShaders[1]->reflectedShaderData;
			}
			GrowingArray<VlkLayoutDescriptor> materialSetLayoutDescriptors = localGetSetLayoutDescription((eShaderStage)pVlkPipeline->m_pShaders[0]->header.shaderType, InstanceDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
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
		if (!CreatePipelineLayout(*pVlkPipeline, pVlkMat))
		{
			// TODO assert
			return false;
		}
	}
	AllocateTypeDescriptors(*pVlkPipeline, frameInFlight);

	//for debugging, check so that once everyFrameDataIsDirty the validator is set to not dirty
	materialDataValidator.ClearFrameData(frameInFlight);

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

	}

	//for debugging, check so that once everyFrameDataIsDirty the validator is set to not dirty
	materialDataValidator.ClearFrameData(frameInFlight);

	return true;
}

bool Hail::VlkMaterialManager::CreateMaterialTypeObject(Pipeline* pPipeline)
{
	VlkPipeline* pVlkPipeline = (VlkPipeline*)pPipeline;

	if (pVlkPipeline->m_bUseTypePasses && m_MaterialTypeObjects[(uint32)pVlkPipeline->m_type])
	{
		// TODO: Check if the reflected shader data matches, otherwise assert
		pVlkPipeline->m_pTypeObject = m_MaterialTypeObjects[(uint32)pVlkPipeline->m_type];
		return true;
	}

	MaterialTypeObject** pTypeObjectToCreate = pVlkPipeline->m_bUseTypePasses ? &m_MaterialTypeObjects[(uint32)pVlkPipeline->m_type] : &pVlkPipeline->m_pTypeObject;
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	(*pTypeObjectToCreate) = new VlkMaterialTypeObject();
	VlkMaterialTypeObject& typeDescriptor = *(VlkMaterialTypeObject*)(*pTypeObjectToCreate);
	typeDescriptor.m_type = pVlkPipeline->m_type;
	if (pVlkPipeline->m_pShaders[0]->loadState == eShaderLoadState::LoadedToRAM)
	{
		const eShaderStage shader1Type = (eShaderStage)pVlkPipeline->m_pShaders[0]->header.shaderType;
		(*pTypeObjectToCreate)->m_boundResources.Add();
		ReflectedShaderData* pSecondaryShaderData = nullptr;
		if (pVlkPipeline->m_pShaders.Size() > 1)
		{
			pSecondaryShaderData = &pVlkPipeline->m_pShaders[1]->reflectedShaderData;
			(*pTypeObjectToCreate)->m_boundResources.Add();
		}

		GrowingArray<VlkLayoutDescriptor> globalSetDescriptors = localGetSetLayoutDescription((eShaderStage)pVlkPipeline->m_pShaders[0]->header.shaderType, GlobalDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
		if (!localCreateSetLayoutDescriptor(globalSetDescriptors, typeDescriptor.m_globalSetLayout, device))
		{
			return false;
		}
		GrowingArray<VlkLayoutDescriptor> typeSetDescriptors = localGetSetLayoutDescription((eShaderStage)pVlkPipeline->m_pShaders[0]->header.shaderType, MaterialTypeDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);
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
	H_ASSERT(vlkPipeline.m_pushConstants.Empty());
	for (size_t i = 0; i < vlkPipeline.m_pShaders.Size(); i++)
	{
		const VectorOnStack<ShaderDecoration, 8>& reflectedPushConstants = vlkPipeline.m_pShaders[i]->reflectedShaderData.m_pushConstants;
		
		for (uint32 iPushConstant = 0u; iPushConstant < reflectedPushConstants.Size(); iPushConstant++)
		{
			const ShaderDecoration& reflectedPushConstant = reflectedPushConstants[iPushConstant];
			vlkPipeline.m_pushConstants.Add(reflectedPushConstant);
			VkPushConstantRange pushConstant;
			//this push constant range starts at the beginning
			pushConstant.offset = 0;
			//this push constant range takes up the size of a MeshPushConstants struct
			pushConstant.size = reflectedPushConstant.m_byteSize; // Check if this length is correct 
			//this push constant range is accessible only in the vertex and compute shader

			eShaderStage shaderType = (eShaderStage)vlkPipeline.m_pShaders[i]->header.shaderType;

			if (shaderType == eShaderStage::Vertex)
			{
				pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			}
			else if (shaderType == eShaderStage::Compute)
			{
				pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			}
			else if (shaderType == eShaderStage::Mesh)
			{
				pushConstant.stageFlags = VK_SHADER_STAGE_MESH_BIT_NV;
			}
			else if (shaderType == eShaderStage::Fragment)
			{
				pushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			push_constants.Add(pushConstant);
		}
	}

	MaterialTypeObject* pTypeObject = vlkPipeline.m_bUseTypePasses ? m_MaterialTypeObjects[(uint32)vlkPipeline.m_type] : vlkPipeline.m_pTypeObject;
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

void Hail::VlkMaterialManager::AllocateTypeDescriptors(VlkPipeline& vlkPipeline, uint32 frameInFlight)
{
	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();
	uint32_t bufferSize = 0;

	ReflectedShaderData* pSecondaryShaderData = nullptr;
	if (vlkPipeline.m_pShaders.Size() > 1)
	{
		pSecondaryShaderData = &vlkPipeline.m_pShaders[1]->reflectedShaderData;
	}

	VlkMaterialTypeObject& typeDescriptor = vlkPipeline.m_bUseTypePasses ? *(VlkMaterialTypeObject*)m_MaterialTypeObjects[(uint32)vlkPipeline.m_type] : *(VlkMaterialTypeObject*)vlkPipeline.m_pTypeObject;

	DescriptorInfos descriptorInfos{};
	GrowingArray< VkWriteDescriptorSet> setWrites = localGetGlobalPipelineDescriptor(descriptorInfos, vlkPipeline, pSecondaryShaderData, typeDescriptor, m_textureManager, m_renderingResourceManager, frameInFlight);

	GrowingArray<VlkLayoutDescriptor> materialDescriptors = localGetSetLayoutDescription(
		(eShaderStage)vlkPipeline.m_pShaders[0]->header.shaderType,
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
			bufferDescriptorInfo.offset = 0u;
			bufferDescriptorInfo.range = vlkBuffer->GetBufferSize();
			VkDescriptorType vkBufferType = descriptor.decorationType == eDecorationType::ShaderStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setWrites.Add(WriteDescriptorBuffer(vkBufferType, typeDescriptor.m_typeDescriptors[frameInFlight], &bufferDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::SampledImage)
		{
			VkDescriptorImageInfo& imageDescriptorInfo = descriptorInfos.sampledImageDescriptorInfos.Add();
			VlkTextureView* view = (VlkTextureView*)m_textureManager->GetEngineTextureView(MaterialTypeDomain, descriptor.bindingPoint, frameInFlight);
			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = view->GetVkImageView();
			imageDescriptorInfo.sampler = ((VlkSamplerObject*)m_renderingResourceManager->GetGlobalSampler(GlobalSamplers::Point))->GetInternalSampler();
			setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, typeDescriptor.m_typeDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
		}
		else if (descriptor.decorationType == eDecorationType::Image)
		{
			VlkTextureView* view = (VlkTextureView*)m_textureManager->GetEngineTextureView(MaterialTypeDomain, descriptor.bindingPoint, frameInFlight);
			VkDescriptorImageInfo& imageDescriptorInfo = descriptorInfos.imageDescriptorInfos.Add();
			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = view->GetVkImageView();
			setWrites.Add(WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, typeDescriptor.m_typeDescriptors[frameInFlight], &imageDescriptorInfo, descriptor.bindingPoint));
		}
		else
		{
			H_ASSERT(false, "Incorrect shader descriptor setup");
		}
	}
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	if (setWrites.Size() > 0)
	{
		vkUpdateDescriptorSets(device.GetDevice(), setWrites.Size(), setWrites.Data(), 0, nullptr);
	}
	typeDescriptor.m_bBoundTypeData[frameInFlight] = true;

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
		(eShaderStage)pVlkPipeline->m_pShaders[0]->header.shaderType,
		InstanceDomain, &pVlkPipeline->m_pShaders[0]->reflectedShaderData, pSecondaryShaderData);

	GrowingArray<VkWriteDescriptorSet> descriptorWrites(instanceDescriptors.Size());

	GrowingArray<VkDescriptorImageInfo> imageDescriptorInfos;

	uint32 textureIndex = 0;
	for (size_t i = 0; i < instanceDescriptors.Size(); i++)
	{
		VlkLayoutDescriptor& descriptor = instanceDescriptors[i];
		if (descriptor.decorationType == eDecorationType::SampledImage)
		{

			VlkTextureView* pVlkTexture = instance.m_textureHandles[textureIndex] != MAX_UINT ?
				(VlkTextureView*)m_textureManager->GetTextureView(instance.m_textureHandles[textureIndex]) :
				(VlkTextureView*)m_textureManager->GetDefaultTexture().m_pView;

			VkDescriptorImageInfo& imageDescriptorInfo = imageDescriptorInfos.Add();

			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = pVlkTexture->GetVkImageView();
			imageDescriptorInfo.sampler = ((VlkSamplerObject*)m_renderingResourceManager->GetGlobalSampler(GlobalSamplers::Point))->GetInternalSampler();

			VkWriteDescriptorSet& writeDescriptor = descriptorWrites.Add();

			writeDescriptor = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				pMat->m_instanceDescriptors[instance.m_gpuResourceInstance].descriptors[frameInFlight],
				&imageDescriptorInfo, descriptor.bindingPoint);

			textureIndex++;
		}
		else if (descriptor.decorationType == eDecorationType::Image)
		{

			VlkTextureView* pVlkTexture = instance.m_textureHandles[textureIndex] != MAX_UINT ?
				(VlkTextureView*)m_textureManager->GetTextureView(instance.m_textureHandles[textureIndex]) :
				(VlkTextureView*)m_textureManager->GetDefaultTexture().m_pView;

			VkDescriptorImageInfo& imageDescriptorInfo = imageDescriptorInfos.Add();

			imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptorInfo.imageView = pVlkTexture->GetVkImageView();

			VkWriteDescriptorSet& writeDescriptor = descriptorWrites.Add();

			writeDescriptor = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				pMat->m_instanceDescriptors[instance.m_gpuResourceInstance].descriptors[frameInFlight],
				&imageDescriptorInfo, descriptor.bindingPoint);

			textureIndex++;
		}
		else
		{
			H_ASSERT(false, "Incorrect shader descriptor setup");
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
