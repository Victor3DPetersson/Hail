#include "Engine_PCH.h"
#include "VlkMaterialManager.h"

#include "Windows\VulkanInternal\VlkDevice.h"
#include "Windows\VulkanInternal\VlkSwapChain.h"
#include "Windows\VulkanInternal\VlkVertex_Descriptor.h"
#include "Rendering\UniformBufferManager.h"
#include "Windows\VulkanInternal\VlkResourceManager.h"
#include "VlkTextureManager.h"

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
#ifdef DEBUG
			throw std::runtime_error("failed to create shader module!");
#endif
			return nullptr;
		}
		shader.loadState = SHADER_LOADSTATE::UPLOADED_TO_GPU;
		SAFEDELETE_ARRAY(shader.compiledCode);
		return shaderModule;
	}

}

void VlkMaterialManager::Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain)
{
	MaterialManager::Init(renderingDevice, textureResourceManager, renderingResourceManager, swapChain);
}

void Hail::VlkMaterialManager::ClearAllResources()
{
	MaterialManager::ClearAllResources();
	//make sure to cleanup everything
	VlkDevice& device = *(VlkDevice*)m_renderDevice;

	for (uint32 i = 0; i < (uint32)(MATERIAL_TYPE::COUNT); i++)
	{
		m_passData[i].CleanupResource(device);
		for (uint32 j = 0; j < MAX_FRAMESINFLIGHT; j++)
		{
			m_passData[i].CleanupResourceFrameData(device, j);
		}
	}
}

VlkPassData& Hail::VlkMaterialManager::GetMaterialData(MATERIAL_TYPE material)
{
	return m_passData[(uint32)material];
}

bool VlkMaterialManager::InitMaterialInternal(MATERIAL_TYPE materialType, FrameBufferTexture* frameBufferToBindToMaterial, uint32 frameInFlight)
{
	if (frameBufferToBindToMaterial)
	{
		m_passesFrameBufferTextures[(uint32)materialType] = (VlkFrameBufferTexture*)(frameBufferToBindToMaterial);
	}
	return CreateMaterialPipeline(m_materials[(uint32)materialType], frameInFlight);
}

bool VlkMaterialManager::CreateMaterialPipeline(Material& material, uint32 frameInFlight)
{
	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	VlkPassData& passData = m_passData[(uint32)material.m_type];
	ResourceValidator& validator = m_passDataValidators[(uint32)material.m_type];
	if (validator.GetIsResourceDirty())
	{
		if (!validator.GetIsFrameDataDirty(frameInFlight))
		{
			//Add fatal assert here that something have gone wrong
			return false;
		}
	}

	passData.m_frameBufferTextures = m_passesFrameBufferTextures[(uint32)material.m_type];
	if(!passData.m_materialDescriptors.IsInitialized())
	{
		passData.m_materialDescriptors.Init(10);
	}
	if (!SetUpMaterialLayouts(passData, material.m_type, frameInFlight))
	{
		return false;
	}

	//for debugging, check so that once everyFrameDataIsDirty the validator is set to not dirty
	validator.ClearFrameData(frameInFlight);

	if (validator.GetFrameThatMarkedFrameDirty() != frameInFlight)
	{
		//Early out as below the resources are only created for the first frame in flight
		return true;
	}


	//Code below is creating Graphics Pipeline
	//TODO: Break out in to its own function
	bool renderDepth = false;
	VkVertexInputBindingDescription vertexBindingDescription = VkVertexInputBindingDescription();
	GrowingArray<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	switch (material.m_type)
	{
	case Hail::MATERIAL_TYPE::SPRITE:
		vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::SPRITE);
		vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::SPRITE);
		break;
	case Hail::MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
		vertexAttributeDescriptions.Init(1);
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
	case Hail::MATERIAL_TYPE::MODEL3D:
		renderDepth = true;
		vertexBindingDescription = GetBindingDescription(VERTEX_TYPES::MODEL);
		vertexAttributeDescriptions = GetAttributeDescriptions(VERTEX_TYPES::MODEL);
		break;
	default:
		break;
	}

	VkShaderModule vertShaderModule = CreateShaderModule(material.m_vertexShader, device);
	VkShaderModule fragShaderModule = CreateShaderModule(material.m_fragmentShader, device);

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
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	const glm::uvec2 resolution = passData.m_frameBufferTextures->GetResolution();
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
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
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

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

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

	pipelineInfo.layout = passData.m_pipelineLayout;
	pipelineInfo.renderPass = passData.m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.Size());
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.Data();

	if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &passData.m_pipeline) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}

	vkDestroyShaderModule(device.GetDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device.GetDevice(), vertShaderModule, nullptr);
	return true;
}

struct VlkLayoutDescriptor
{
	VkShaderStageFlags flags;
	uint32_t bindingPoint;
	VkDescriptorType type;
};

bool CreateSetLayoutDescriptor(GrowingArray<VlkLayoutDescriptor> descriptors, VkDescriptorSetLayout& returnDescriptorLayoput, VlkDevice& device)
{
	GrowingArray<VkDescriptorSetLayoutBinding>bindings;
	bindings.InitAndFill(descriptors.Size());

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

bool VlkMaterialManager::SetUpMaterialLayouts(VlkPassData& passData, MATERIAL_TYPE type, uint32 frameInFlight)
{
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();

	if (m_passDataValidators[(uint32)type].GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		if (!SetUpPipelineLayout(passData, type, frameInFlight))
		{
			//TODO ASSERT
			return false;
		}

		GrowingArray<VkDescriptorSetLayout> setLayouts(MAX_FRAMESINFLIGHT, passData.m_passSetLayout, false);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = vlkRenderingResources->m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = (uint32)MAX_FRAMESINFLIGHT;
		passAllocInfo.pSetLayouts = setLayouts.Data();

		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, passData.m_passDescriptors) != VK_SUCCESS)
		{
			//TODO ASSERT
			return false;
		}
	}

	if (!CreateRenderpassAndFramebuffers(passData, type, frameInFlight))
	{
		return false;
	}


	GrowingArray< VkWriteDescriptorSet> setWrites;
	uint32_t bufferSize = 0;
	m_descriptorImageInfo.imageView = VK_NULL_HANDLE;
	m_descriptorImageInfo.sampler = VK_NULL_HANDLE;
	m_descriptorBufferInfo.buffer = VK_NULL_HANDLE;
	m_descriptorBufferInfo.range = 0;
	switch (type)
	{
	case Hail::MATERIAL_TYPE::SPRITE:
	{
		setWrites.Init(1);

		bufferSize = GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER);
		m_descriptorBufferInfo.buffer = vlkRenderingResources->m_buffers[static_cast<uint32_t>(BUFFERS::SPRITE_INSTANCE_BUFFER)].m_buffer[frameInFlight];
		m_descriptorBufferInfo.offset = 0;
		m_descriptorBufferInfo.range = bufferSize;
		if (!ValidateDescriptorBufferWrite(m_descriptorBufferInfo))
		{
			return false;
		}
		VkWriteDescriptorSet spriteInstanceBuffer = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, passData.m_passDescriptors[frameInFlight], m_descriptorBufferInfo, 1);
		setWrites.Add(spriteInstanceBuffer);
	}
	break;
	case Hail::MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
	{
		setWrites.Init(1);

		m_descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_descriptorImageInfo.imageView = m_passesFrameBufferTextures[(uint32)(MATERIAL_TYPE::SPRITE)]->GetTextureImage(frameInFlight).imageView;
		m_descriptorImageInfo.sampler = vlkRenderingResources->m_pointTextureSampler;
		if (!ValidateDescriptorSamplerWrite(m_descriptorImageInfo))
		{
			return false;
		}
		VkWriteDescriptorSet textureSampler = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, passData.m_passDescriptors[frameInFlight], m_descriptorImageInfo, 1);
		setWrites.Add(textureSampler);
	}

	break;
	case Hail::MATERIAL_TYPE::MODEL3D:
	{
		setWrites.Init(2);

		bufferSize = GetUniformBufferSize(BUFFERS::TUTORIAL);
		m_descriptorBufferInfo.buffer = vlkRenderingResources->m_buffers[static_cast<uint32_t>(BUFFERS::TUTORIAL)].m_buffer[frameInFlight];
		m_descriptorBufferInfo.offset = 0;
		m_descriptorBufferInfo.range = bufferSize;
		if (!ValidateDescriptorBufferWrite(m_descriptorBufferInfo))
		{
			return false;
		}
		VkWriteDescriptorSet tutorialBufferSet = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, passData.m_passDescriptors[frameInFlight], m_descriptorBufferInfo, GetUniformBufferIndex(Hail::BUFFERS::TUTORIAL));
		setWrites.Add(tutorialBufferSet);

		m_descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_descriptorImageInfo.imageView = ((VlkTextureResourceManager*)m_textureManager)->GetTextureData(1).textureImageView;
		m_descriptorImageInfo.sampler = vlkRenderingResources->m_linearTextureSampler;
		if (!ValidateDescriptorSamplerWrite(m_descriptorImageInfo))
		{
			return false;
		}
		VkWriteDescriptorSet textureSampler = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, passData.m_passDescriptors[frameInFlight], m_descriptorImageInfo, 1);
		setWrites.Add(textureSampler);

	}
	break;
	default:
		break;
	}
	if (setWrites.Size() > 0)
	{
		vkUpdateDescriptorSets(device.GetDevice(), setWrites.Size(), setWrites.Data(), 0, nullptr);
	}
}

bool Hail::VlkMaterialManager::SetUpPipelineLayout(VlkPassData& passData, MATERIAL_TYPE type, uint32 frameInFlight)
{
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);

	switch (type)
	{
	case Hail::MATERIAL_TYPE::SPRITE:
		if (!CreateSetLayoutDescriptor(
			{
				{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, GetUniformBufferIndex(Hail::BUFFERS::SPRITE_INSTANCE_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}
			}, passData.m_passSetLayout, device))
		{
			return false;
		}
		if (!CreateSetLayoutDescriptor(
			{
				{ VK_SHADER_STAGE_FRAGMENT_BIT,  1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
			}, passData.m_materialSetLayout, device))
		{
			return false;
		}
		break;
	case Hail::MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
		if (!CreateSetLayoutDescriptor(
			{
				{ VK_SHADER_STAGE_FRAGMENT_BIT,  1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
			}, passData.m_passSetLayout, device))
		{
			return false;
		}
		break;
	case Hail::MATERIAL_TYPE::MODEL3D:
		if (!CreateSetLayoutDescriptor(
			{
				{VK_SHADER_STAGE_VERTEX_BIT,  GetUniformBufferIndex(Hail::BUFFERS::TUTORIAL), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
				{VK_SHADER_STAGE_FRAGMENT_BIT,  1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
			}, passData.m_passSetLayout, device))
		{
			return false;
		}
		if (!CreateSetLayoutDescriptor(
			{
				{ VK_SHADER_STAGE_FRAGMENT_BIT,  1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
			}, passData.m_materialSetLayout, device))
		{
			return false;
		}
		break;
	default:
		break;
	}

	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();

	//setup push constants
	VkPushConstantRange push_constant;
	//this push constant range starts at the beginning
	push_constant.offset = 0;
	//this push constant range takes up the size of a MeshPushConstants struct
	push_constant.size = sizeof(uint32_t) * 4;
	//this push constant range is accessible only in the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	GrowingArray<VkDescriptorSetLayout> layouts;
	layouts.Init(3);
	layouts.Add(vlkRenderingResources->m_globalPerFrameSetLayout);
	if (passData.m_passSetLayout != VK_NULL_HANDLE)
	{
		layouts.Add(passData.m_passSetLayout);
	}
	if (passData.m_materialSetLayout != VK_NULL_HANDLE)
	{
		layouts.Add(passData.m_materialSetLayout);
	}
	VkPipelineLayoutCreateInfo passPipelineLayoutInfo{};
	passPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	passPipelineLayoutInfo.setLayoutCount = layouts.Size(); // Optional
	passPipelineLayoutInfo.pSetLayouts = layouts.Data(); // Optional
	passPipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
	passPipelineLayoutInfo.pPushConstantRanges = &push_constant; // Optional

	if (vkCreatePipelineLayout(device.GetDevice(), &passPipelineLayoutInfo, nullptr, &passData.m_pipelineLayout) != VK_SUCCESS)
	{
		//TODO ASSERT
		return false;
	}
}

bool VlkMaterialManager::CreateRenderpassAndFramebuffers(VlkPassData& passData, MATERIAL_TYPE type, uint32 frameInFlight)
{
	VlkDevice& device = *(VlkDevice*)(m_renderDevice);
	GrowingArray<VkAttachmentDescription> attachmentDescriptors;
	VkAttachmentReference colorAttachmentRef{};
	VkAttachmentReference depthAttachmentRef{};
	switch (type)
	{
	case Hail::MATERIAL_TYPE::SPRITE:
	{
		attachmentDescriptors.Init(2);
		VkAttachmentDescription colorAttachment{};
		VkAttachmentDescription depthAttachment{};

		colorAttachment.format = ToVkFormat(passData.m_frameBufferTextures->GetTextureFormat());
		depthAttachment.format = ToVkFormat(passData.m_frameBufferTextures->GetDepthFormat());
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
	case Hail::MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
	{
		attachmentDescriptors.Init(1);
		VkAttachmentDescription colorAttachment{};
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescriptors.Add(colorAttachment);
	}


	break;
	case Hail::MATERIAL_TYPE::MODEL3D:
	{
		attachmentDescriptors.Init(2);
		VkAttachmentDescription colorAttachment{};
		VkAttachmentDescription depthAttachment{};
		colorAttachment.format = ToVkFormat(passData.m_frameBufferTextures->GetTextureFormat());
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

		depthAttachment.format = ToVkFormat(passData.m_frameBufferTextures->GetDepthFormat());
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

	if (type != MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX)
	{
		if (vkCreateRenderPass(device.GetDevice(), &renderPassInfo, nullptr, &passData.m_renderPass) != VK_SUCCESS)
		{
			//TODO ASSERT
			return false;
		}

		FrameBufferTextureData colorTexture = passData.m_frameBufferTextures->GetTextureImage(frameInFlight);
		FrameBufferTextureData depthTexture = passData.m_frameBufferTextures->GetDepthTextureImage(frameInFlight);
		VkImageView attachments[2] = { colorTexture.imageView, depthTexture.imageView };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = passData.m_renderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = passData.m_frameBufferTextures->GetResolution().x;
		framebufferInfo.height = passData.m_frameBufferTextures->GetResolution().y;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &passData.m_frameBuffer[frameInFlight]) != VK_SUCCESS)
		{
			//TODO ASSERT
			return false;
		}
	}
	else
	{
		VlkSwapChain* swapChain = (VlkSwapChain*)m_swapChain;
		passData.m_ownsFrameBuffer = false;
		passData.m_ownsRenderpass = false;

		passData.m_frameBuffer[frameInFlight] = swapChain->GetFrameBuffer(frameInFlight);
		passData.m_renderPass = swapChain->GetRenderPass();
	}
	return true;
}

bool VlkMaterialManager::InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight)
{
	const Material& material = m_materials[instance.m_materialIdentifier];
	VlkDevice& device = *(VlkDevice*)m_renderDevice;
	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)((RenderingResourceManager*)m_renderingResourceManager)->GetRenderingResources();
	VlkPassData& passData = m_passData[(uint32)material.m_type];

	if (m_materialsInstanceValidationData[instance.m_instanceIdentifier].GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		VlkPassData::VkInternalMaterialDescriptorSet setAllocLayouts{};
		setAllocLayouts.descriptors[0] = VK_NULL_HANDLE;
		setAllocLayouts.descriptors[1] = VK_NULL_HANDLE;
		GrowingArray<VkDescriptorSetLayout> setLayouts(MAX_FRAMESINFLIGHT, passData.m_materialSetLayout, false);
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
		instance.m_gpuResourceInstance = passData.m_materialDescriptors.Size();
		passData.m_materialDescriptors.Add(setAllocLayouts);
	}



	GrowingArray<VkWriteDescriptorSet> descriptorWrites{};
	switch (material.m_type)
	{
	case Hail::MATERIAL_TYPE::SPRITE:
	{
		descriptorWrites.Init(1);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = ((VlkTextureResourceManager*)m_textureManager)->GetTextureData(instance.m_textureHandles[0]).textureImageView;
		imageInfo.sampler = vlkRenderingResources->m_linearTextureSampler;
		VkWriteDescriptorSet textureWrite = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, passData.m_materialDescriptors[instance.m_gpuResourceInstance].descriptors[frameInFlight], imageInfo, 1);
		descriptorWrites.Add(textureWrite);
		vkUpdateDescriptorSets(device.GetDevice(), static_cast<uint32_t>(descriptorWrites.Size()), descriptorWrites.Data(), 0, nullptr);
	}

	break;
	case Hail::MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
	{
	}
	break;
	case Hail::MATERIAL_TYPE::MODEL3D:
	{
	}
	break;
	default:
		break;
	}


	return true;
}

void Hail::VlkMaterialManager::ClearMaterialInternal(MATERIAL_TYPE materialType, uint32 frameInFlight)
{
	m_passDataValidators[(uint32)materialType].MarkResourceAsDirty(frameInFlight);
	if (m_passDataValidators[(uint32)materialType].GetFrameThatMarkedFrameDirty() == frameInFlight)
	{
		m_passData[(uint32)materialType].CleanupResource(*(VlkDevice*)m_renderDevice);
	}
	m_passData[(uint32)materialType].CleanupResourceFrameData(*(VlkDevice*)m_renderDevice, frameInFlight);
}
