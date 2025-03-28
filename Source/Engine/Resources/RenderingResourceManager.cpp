#include "Engine_PCH.h"
#include "RenderingResourceManager.h"
#include "Resources_Materials\ShaderBufferList.h"

#include "RenderCommands.h"
using namespace Hail;

BufferObject* Hail::RenderingResourceManager::GetGlobalBuffer(eDecorationSets setToGet, eBufferType bufferType, uint8 bindingPoint)
{
	// TODO: add asserts to this function 
	if (bufferType == eBufferType::uniform)
	{
		switch (setToGet)
		{
		case Hail::GlobalDomain:
			return m_uniformBuffers[setToGet][bindingPoint];
		case Hail::MaterialTypeDomain:
			return m_uniformBuffers[setToGet][bindingPoint];
		case Hail::InstanceDomain:
		case Hail::Count:
		default:
            break;
		}
	}
	else if (bufferType == eBufferType::structured)
	{
		switch (setToGet)
		{
		case Hail::GlobalDomain:
			return m_structuredBuffers[setToGet][bindingPoint];
		case Hail::MaterialTypeDomain:
			return m_structuredBuffers[setToGet][bindingPoint];
		case Hail::InstanceDomain: 
		case Hail::Count:
		default:
            break;
        }
	}

	return nullptr;
}

SamplerObject* Hail::RenderingResourceManager::GetGlobalSampler(GlobalSamplers samplerToGet)
{
    return m_samplers[(uint32)samplerToGet];
}

bool Hail::RenderingResourceManager::InternalInit()
{
    // TODO use a Macro definition from the ShaderBufferList and decipher that to remove the hard-coded aspect of this:

    auto assignBuffer = [=](BufferObject* pBuffer,  eDecorationSets setToCreateBufferFor) {
        if (pBuffer->GetProperties().type == eBufferType::structured)
        {
            m_structuredBuffers[setToCreateBufferFor].Add(pBuffer);
        }
        else if (pBuffer->GetProperties().type == eBufferType::uniform)
        {
            m_uniformBuffers[setToCreateBufferFor].Add(pBuffer);
        }
    };

    BufferProperties properties;
    for (size_t i = 0; i < (uint32)eGlobalUniformBuffers::count; i++)
    {
        properties.type = eBufferType::uniform;
        properties.numberOfElements = 1;
        properties.offset = 0;
        properties.numberOfElements = 0;
        properties.elementByteSize = 0;
        switch ((eGlobalUniformBuffers)i)
        {
        case Hail::eGlobalUniformBuffers::frameData:
            properties.numberOfElements = 1;
            properties.elementByteSize = sizeof(PerFrameUniformBuffer);
            properties.domain = eShaderBufferDomain::CpuToGpu;
            properties.usage = eShaderBufferUsage::Read;
            properties.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
            break;
        case Hail::eGlobalUniformBuffers::viewData:
            properties.numberOfElements = 1;
            properties.elementByteSize = sizeof(PerCameraUniformBuffer);
            properties.domain = eShaderBufferDomain::CpuToGpu;
            properties.usage = eShaderBufferUsage::Read;
            properties.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
            break;
        case Hail::eGlobalUniformBuffers::count:
        default:
            break;
        }
        if (properties.numberOfElements == 0 || properties.elementByteSize == 0)
            continue;
        assignBuffer(CreateBuffer(properties), eDecorationSets::GlobalDomain);
    }
    for (size_t i = 0; i < (uint32)eGlobalStructuredBuffers::count; i++)
    {
        properties.type = eBufferType::structured;
        properties.numberOfElements = 1;
        properties.offset = 0;
        properties.numberOfElements = 0;
        properties.elementByteSize = 0;
        switch ((eGlobalStructuredBuffers)i)
        {
        case Hail::eGlobalStructuredBuffers::count:
        default:
            // TODO: ASSERT
            break;
        }
        if (properties.numberOfElements == 0 || properties.elementByteSize == 0)
            continue;
        assignBuffer(CreateBuffer(properties), eDecorationSets::GlobalDomain);
    }

    for (size_t i = 0; i < (uint32)eMaterialUniformBuffers::count; i++)
    {
        properties.type = eBufferType::uniform;
        properties.numberOfElements = 0;
        properties.offset = 0;
        properties.numberOfElements = 0;
        properties.elementByteSize = 0;
        switch ((eMaterialUniformBuffers)i)
        {
        case Hail::eMaterialUniformBuffers::vulkanTutorialUniformBUffer:
            properties.numberOfElements = 1;
            properties.elementByteSize = sizeof(TutorialUniformBufferObject);
            properties.domain = eShaderBufferDomain::CpuToGpu;
            properties.usage = eShaderBufferUsage::Read;
            properties.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;

            break;
        case Hail::eMaterialUniformBuffers::count:
        default:
            // TODO: ASSERT
            break;
        }
        if (properties.numberOfElements == 0 || properties.elementByteSize == 0)
            continue;
        assignBuffer(CreateBuffer(properties), eDecorationSets::MaterialTypeDomain);
    }

    for (size_t i = 0; i < (uint32)eMaterialBuffers::count; i++)
    {
        properties.type = eBufferType::structured;
        properties.offset = 0;
        properties.numberOfElements = 0;
        properties.elementByteSize = 0;
        switch ((eMaterialBuffers)i)
        {
        case Hail::eMaterialBuffers::instanceBuffer2D:
            properties.numberOfElements = MAX_NUMBER_OF_2D_RENDER_COMMANDS;
            properties.elementByteSize = sizeof(RenderCommand2DBase);
            properties.domain = eShaderBufferDomain::CpuToGpu;
            properties.usage = eShaderBufferUsage::Read;
            properties.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
            break;
        case Hail::eMaterialBuffers::lineInstanceBuffer:
            properties.numberOfElements = MAX_NUMBER_OF_DEBUG_LINES;
            properties.elementByteSize = sizeof(DebugLineData);
            properties.domain = eShaderBufferDomain::CpuToGpu;
            properties.usage = eShaderBufferUsage::Read;
            properties.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
            break;
        case Hail::eMaterialBuffers::spriteDataBuffer:
            properties.numberOfElements = MAX_NUMBER_OF_SPRITES;
            properties.elementByteSize = sizeof(RenderData_Sprite);
            properties.domain = eShaderBufferDomain::CpuToGpu;
            properties.usage = eShaderBufferUsage::Read;
            properties.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
            break;
        case Hail::eMaterialBuffers::count:
        default:
            break;
        }
        if (properties.numberOfElements == 0 || properties.elementByteSize == 0)
            continue;

        assignBuffer(CreateBuffer(properties), eDecorationSets::MaterialTypeDomain);
    }

    m_samplers[(uint32)GlobalSamplers::Point] = CreateSamplerObject(SamplerProperties());

    SamplerProperties bilinearSamplerProps = SamplerProperties();
    bilinearSamplerProps.sampler_mode = TEXTURE_SAMPLER_FILTER_MODE::LINEAR;
    bilinearSamplerProps.filter_mag = TEXTURE_FILTER_MODE::LINEAR;
    bilinearSamplerProps.filter_min = TEXTURE_FILTER_MODE::LINEAR;
    m_samplers[(uint32)GlobalSamplers::Bilinear] = CreateSamplerObject(bilinearSamplerProps);

	return true;
}