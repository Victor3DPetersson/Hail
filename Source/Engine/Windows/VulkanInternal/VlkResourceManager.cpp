#include "Engine_PCH.h"
#include "VlkResourceManager.h"
#include "Resources\TextureCommons.h"

#include "VlkDevice.h"
#include "VlkBufferCreationFunctions.h"
#include "VlkTextureCreationFunctions.h"
#include "VlkVertex_Descriptor.h"
#include "VlkSwapChain.h"

namespace Hail
{
	namespace {
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

    void VlkTextureResourceManager::Init(RenderingDevice* device)
    {
        m_device = reinterpret_cast<VlkDevice*>(device);
		m_textureData.Init(10);

    }

	bool VlkTextureResourceManager::CreateTextureData(CompiledTexture& compiledTexture)
    {
		VlkTextureData vlkTextureData{};

		VlkDevice& device = *m_device;
		if (compiledTexture.loadState != TEXTURE_LOADSTATE::LOADED_TO_RAM)
		{
			return false;
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		const uint32_t imageSize = GetTextureByteSize(compiledTexture.header);

		CreateBuffer(device, imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, compiledTexture.compiledColorValues, static_cast<size_t>(imageSize));
		vkUnmapMemory(device.GetDevice(), stagingBufferMemory);
		DeleteCompiledTexture(compiledTexture);

		VkFormat textureFormat = ToVkFormat(TextureTypeToTextureFormat(static_cast<TEXTURE_TYPE>(compiledTexture.header.textureType)));

		CreateImage(device, compiledTexture.header.width, compiledTexture.header.height,
			textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vlkTextureData.textureImage, vlkTextureData.textureImageMemory);

		TransitionImageLayout(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());
		CopyBufferToImage(device, stagingBuffer, vlkTextureData.textureImage, compiledTexture.header.width, compiledTexture.header.height, device.GetCommandPool(), device.GetGraphicsQueue());
		TransitionImageLayout(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());

		vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);

		vlkTextureData.textureImageView = CreateImageView(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		m_textureData.Add(vlkTextureData);

		return true;
    }


    VlkTextureData& VlkTextureResourceManager::GetTextureData(uint32_t index)
    {
		return m_textureData[index];
    }

	FrameBufferTexture* VlkTextureResourceManager::FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat)
	{
		VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_device);
		VlkFrameBufferTexture* frameBuffer = new VlkFrameBufferTexture(resolution, format, depthFormat);
		frameBuffer->SetName(name);
		frameBuffer->CreateFrameBufferTextureObjects(device);
		return frameBuffer;
	}


	bool VlkMaterialeResourceManager::Init(RenderingDevice* device, VlkTextureResourceManager* textureResourceManager, VlkSwapChain* swapChain)
	{
		m_device = reinterpret_cast<VlkDevice*>(device);
		m_swapChain = swapChain;
		m_textureResourceManager = textureResourceManager;
		m_linearTextureSampler = CreateTextureSampler(*m_device, TextureSamplerData{});
		TextureSamplerData pointSamplerData;
		pointSamplerData.sampler_mode = TEXTURE_SAMPLER_FILTER_MODE::POINT;
		m_pointTextureSampler = CreateTextureSampler(*m_device, pointSamplerData);
		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			volatile uint32_t index = 0;
			index = static_cast<uint32_t>(BUFFERS::TUTORIAL);
			if (!CreateBuffer(*m_device, GetUniformBufferSize(BUFFERS::TUTORIAL), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffers[index].m_buffer[i], m_buffers[index].m_bufferMemory[i]))
			{
				return false;
			}
			vkMapMemory(m_device->GetDevice(), m_buffers[index].m_bufferMemory[i], 0, GetUniformBufferSize(BUFFERS::TUTORIAL), 0, &m_buffers[index].m_bufferMapped[i]);

			index = static_cast<uint32_t>(BUFFERS::SPRITE_INSTANCE_BUFFER);
			if (!CreateBuffer(*m_device, GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffers[index].m_buffer[i], m_buffers[index].m_bufferMemory[i]))
			{
				return false;
			}
			vkMapMemory(m_device->GetDevice(), m_buffers[index].m_bufferMemory[i], 0, GetUniformBufferSize(BUFFERS::SPRITE_INSTANCE_BUFFER), 0, &m_buffers[index].m_bufferMapped[i]);

			index = static_cast<uint32_t>(BUFFERS::PER_FRAME_DATA);
			if (!CreateBuffer(*m_device, GetUniformBufferSize(BUFFERS::PER_FRAME_DATA), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffers[index].m_buffer[i], m_buffers[index].m_bufferMemory[i]))
			{
				return false;
			}
			vkMapMemory(m_device->GetDevice(), m_buffers[index].m_bufferMemory[i], 0, GetUniformBufferSize(BUFFERS::PER_FRAME_DATA), 0, &m_buffers[index].m_bufferMapped[i]);
		}

		if (!SetUpCommonLayouts())
		{
			return false;
		}
	}

	bool VlkMaterialeResourceManager::InitMaterial(Material& material, FrameBufferTexture* frameBufferTexture)
	{
		m_passesFrameBufferTextures[static_cast<uint32_t>(material.m_type)] = reinterpret_cast<VlkFrameBufferTexture*>(frameBufferTexture);
		bool result = CreateMaterialPipeline(material);
		return result;
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

	bool VlkMaterialeResourceManager::InitInstance(const Material material, MaterialInstance& instance)
	{
		VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_device);
		VlkPassData& passData = m_passData[static_cast<uint32_t>(material.m_type)];
		instance.m_materialIdentifier = static_cast<uint32_t>(material.m_type);

		VlkPassData::VkInternalMaterialDescriptorSet setAllocLayouts{};
		setAllocLayouts.descriptors[0] = VK_NULL_HANDLE;
		setAllocLayouts.descriptors[1] = VK_NULL_HANDLE;
		GrowingArray<VkDescriptorSetLayout> setLayouts(MAX_FRAMESINFLIGHT, passData.m_materialSetLayout, false);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
		passAllocInfo.pSetLayouts = setLayouts.Data();

		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, setAllocLayouts.descriptors) != VK_SUCCESS)
		{
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to allocate descriptor sets!");
#endif
		}

		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			GrowingArray<VkWriteDescriptorSet> descriptorWrites{};
			switch (material.m_type)
			{
			case Hail::MATERIAL_TYPE::SPRITE: 
			{
				descriptorWrites.Init(1);

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = m_textureResourceManager->GetTextureData(instance.m_textureHandles[0]).textureImageView;
				imageInfo.sampler = m_linearTextureSampler;

				VkWriteDescriptorSet textureWrite = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, setAllocLayouts.descriptors[i], imageInfo, 1);
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
		}

		instance.m_instanceIdentifier = passData.m_materialDescriptors.Size();
		passData.m_materialDescriptors.Add(setAllocLayouts);

		return true;
	}

	VlkPassData& VlkMaterialeResourceManager::GetMaterialData(MATERIAL_TYPE material)
	{
		return m_passData[static_cast<uint32_t>(material)];
	}

	void VlkMaterialeResourceManager::MapMemoryToBuffer(BUFFERS buffer, void* dataToMap, uint32_t sizeOfData)
	{
		memcpy(m_buffers[static_cast<uint32_t>(buffer)].m_bufferMapped[m_swapChain->GetCurrentFrame()], dataToMap, sizeOfData);
	}

	VkDescriptorSet& VlkMaterialeResourceManager::GetGlobalDescriptorSet(uint32_t frameInFlight)
	{
		return m_globalDescriptorSetsPerFrame[frameInFlight];
	}

	bool VlkMaterialeResourceManager::CreateMaterialPipeline(Material& material)
	{
		VlkDevice& device = *m_device;
		VlkPassData& passData = m_passData[static_cast<uint32_t>(material.m_type)];
		passData.m_frameBufferTextures = m_passesFrameBufferTextures[static_cast<uint32_t>(material.m_type)];
		passData.m_materialDescriptors.Init(10);
		bool renderDepth = false;
		if (!SetUpMaterialLayouts(passData, material.m_type))
		{
			return false;
		}

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
		const VkExtent2D passExtent = { resolution.x, resolution.y};
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
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to create graphics pipeline!");
#endif
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

	bool CreateSetLayoutDescriptor(GrowingArray<VlkLayoutDescriptor> descriptors, VkDescriptorSetLayout& returnDescriptorLayoput, VlkDevice & device)
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




	bool VlkMaterialeResourceManager::SetUpCommonLayouts()
	{
		VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_device);
		// SWIMMING POOOL 
		std::array<VkDescriptorPoolSize, 4> poolSizes =
		{
			{
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
			}
		};
		VkDescriptorPoolCreateInfo finalPoolInfo{};
		finalPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		finalPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		finalPoolInfo.pPoolSizes = poolSizes.data();
		finalPoolInfo.maxSets = 100;
		finalPoolInfo.flags = 0;
		if (vkCreateDescriptorPool(device.GetDevice(), &finalPoolInfo, nullptr, &m_globalDescriptorPool) != VK_SUCCESS)
		{
			return false;
		}

		if (!CreateSetLayoutDescriptor({
			{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,  GetUniformBufferIndex(Hail::BUFFERS::PER_FRAME_DATA), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
			}, m_globalPerFrameSetLayout, device))
		{
			return false;
		}
		GrowingArray<VkDescriptorSetLayout> layouts(MAX_FRAMESINFLIGHT, m_globalPerFrameSetLayout, false);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
		passAllocInfo.pSetLayouts = layouts.Data();

		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, m_globalDescriptorSetsPerFrame) != VK_SUCCESS)
		{
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to allocate descriptor sets!");
#endif
		}

		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_buffers[static_cast<uint32_t>(BUFFERS::PER_FRAME_DATA)].m_buffer[i];
			bufferInfo.offset = 0;
			bufferInfo.range = static_cast<VkDeviceSize>(GetUniformBufferSize(BUFFERS::PER_FRAME_DATA));
			VkWriteDescriptorSet perFrameBuffer = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_globalDescriptorSetsPerFrame[i], bufferInfo, 0);
			VkWriteDescriptorSet setWrites[] = { perFrameBuffer };
			vkUpdateDescriptorSets(device.GetDevice(), 1, setWrites, 0, nullptr);
		}

		return true;
	}

	[[nodiscard]] volatile bool ValidateDescriptorSamplerWrite(VkDescriptorImageInfo& descriptorToValidate)
	{
		bool returnValue = true;
		returnValue |= descriptorToValidate.imageView != VK_NULL_HANDLE;
		returnValue |= descriptorToValidate.sampler != VK_NULL_HANDLE;
		return returnValue;
	}
	[[nodiscard]] volatile bool ValidateDescriptorBufferWrite(VkDescriptorBufferInfo& descriptorToValidate)
	{
		bool returnValue = true;
		returnValue |= descriptorToValidate.buffer != VK_NULL_HANDLE;
		returnValue |= descriptorToValidate.range != 0u;
		return returnValue;
	}

	bool VlkMaterialeResourceManager::SetUpMaterialLayouts(VlkPassData& passData, MATERIAL_TYPE type)
	{
		VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_device);

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
		layouts.Add(m_globalPerFrameSetLayout);
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
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to create pipeline layout!");
#endif
		}

		if (!CreateRenderpassAndFramebuffers(passData, type))
		{
			return false;
		}

		GrowingArray<VkDescriptorSetLayout> setLayouts(MAX_FRAMESINFLIGHT, passData.m_passSetLayout, false);
		VkDescriptorSetAllocateInfo passAllocInfo{};
		passAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		passAllocInfo.descriptorPool = m_globalDescriptorPool;
		passAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMESINFLIGHT);
		passAllocInfo.pSetLayouts = setLayouts.Data();


		if (vkAllocateDescriptorSets(device.GetDevice(), &passAllocInfo, passData.m_passDescriptors) != VK_SUCCESS)
		{
			return false;
#ifdef DEBUG
			throw std::runtime_error("failed to allocate descriptor sets!");
#endif
		}

		for (uint32_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
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
					m_descriptorBufferInfo.buffer = m_buffers[static_cast<uint32_t>(BUFFERS::SPRITE_INSTANCE_BUFFER)].m_buffer[i];
					m_descriptorBufferInfo.offset = 0;
					m_descriptorBufferInfo.range = bufferSize;
					if (!ValidateDescriptorBufferWrite(m_descriptorBufferInfo))
					{
						return false;
					}
					VkWriteDescriptorSet spriteInstanceBuffer = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, passData.m_passDescriptors[i], m_descriptorBufferInfo, 1);
					setWrites.Add(spriteInstanceBuffer);
				}
				break;
			case Hail::MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
				{
					setWrites.Init(1);

					m_descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					m_descriptorImageInfo.imageView = m_passesFrameBufferTextures[static_cast<uint32_t>(MATERIAL_TYPE::SPRITE)]->GetTextureImage(i).imageView;
					m_descriptorImageInfo.sampler = m_pointTextureSampler;
					if (!ValidateDescriptorSamplerWrite(m_descriptorImageInfo))
					{
						return false;
					}
					VkWriteDescriptorSet textureSampler = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, passData.m_passDescriptors[i], m_descriptorImageInfo, 1);
					setWrites.Add(textureSampler);
				}

				break;
			case Hail::MATERIAL_TYPE::MODEL3D:
				{
					setWrites.Init(2);

					bufferSize = GetUniformBufferSize(BUFFERS::TUTORIAL);
					m_descriptorBufferInfo.buffer = m_buffers[static_cast<uint32_t>(BUFFERS::TUTORIAL)].m_buffer[i];
					m_descriptorBufferInfo.offset = 0;
					m_descriptorBufferInfo.range = bufferSize;
					if (!ValidateDescriptorBufferWrite(m_descriptorBufferInfo))
					{
						return false;
					}
					VkWriteDescriptorSet tutorialBufferSet = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, passData.m_passDescriptors[i], m_descriptorBufferInfo, GetUniformBufferIndex(Hail::BUFFERS::TUTORIAL));
					setWrites.Add(tutorialBufferSet);

					m_descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					m_descriptorImageInfo.imageView = m_textureResourceManager->GetTextureData(1).textureImageView;
					m_descriptorImageInfo.sampler = m_linearTextureSampler;
					if (!ValidateDescriptorSamplerWrite(m_descriptorImageInfo))
					{
						return false;
					}
					VkWriteDescriptorSet textureSampler = WriteDescriptorSampler(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, passData.m_passDescriptors[i], m_descriptorImageInfo, 1);
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

	}
	bool VlkMaterialeResourceManager::CreateRenderpassAndFramebuffers(VlkPassData& passData, MATERIAL_TYPE type)
	{
		VlkDevice& device = *reinterpret_cast<VlkDevice*>(m_device);
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
				return false;
#ifdef DEBUG
				throw std::runtime_error("failed to create render pass!");
#endif
			}

			for (uint32_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
			{

				FrameBufferTextureData colorTexture = passData.m_frameBufferTextures->GetTextureImage(i);
				FrameBufferTextureData depthTexture = passData.m_frameBufferTextures->GetDepthTextureImage(i);
				VkImageView attachments[2] = { colorTexture.imageView, depthTexture.imageView };

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = passData.m_renderPass;
				framebufferInfo.attachmentCount = 2;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = passData.m_frameBufferTextures->GetResolution().x;
				framebufferInfo.height = passData.m_frameBufferTextures->GetResolution().y;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &passData.m_frameBuffer[i]) != VK_SUCCESS)
				{
					return false;
#ifdef DEBUG
					throw std::runtime_error("failed to create framebuffer!");
#endif
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < m_swapChain->GetSwapchainImageCount(); i++)
			{
				passData.m_frameBuffer[i] = m_swapChain->GetFrameBuffer(i);
			}
			passData.m_renderPass = m_swapChain->GetRenderPass();
		}
		return true;
	}
}