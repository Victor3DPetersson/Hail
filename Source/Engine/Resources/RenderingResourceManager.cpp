#include "Engine_PCH.h"
#include "RenderingResourceManager.h"
#include "Resources_Materials\ShaderBufferList.h"
using namespace Hail;

BufferObject* Hail::RenderingResourceManager::GetBuffer(eDecorationSets setToGet, eBufferType bufferType, uint8 bindingPoint)
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

bool Hail::RenderingResourceManager::InternalInit()
{
    // TODO use a Macro definition from the ShaderBufferList and decipher that to remove the hard-coded aspect of this:
    BufferProperties properties;
    for (size_t i = 0; i < (uint32)eGlobalUniformBuffers::count; i++)
    {
        properties.type = eBufferType::uniform;
        properties.numberOfElements = 1;
        properties.offset = 0;
        switch ((eGlobalUniformBuffers)i)
        {
        case Hail::eGlobalUniformBuffers::frameData:
            properties.elementByteSize = sizeof(PerFrameUniformBuffer);
            break;
        case Hail::eGlobalUniformBuffers::viewData:
            properties.elementByteSize = sizeof(PerCameraUniformBuffer);
            break;
        case Hail::eGlobalUniformBuffers::count:
        default:
            // TODO: ASSERT
            break;
        }
        CreateBuffer(properties, eDecorationSets::GlobalDomain);
    }
    for (size_t i = 0; i < (uint32)eGlobalStructuredBuffers::count; i++)
    {
        properties.type = eBufferType::structured;
        properties.numberOfElements = 1;
        properties.offset = 0;
        switch ((eGlobalStructuredBuffers)i)
        {
        case Hail::eGlobalStructuredBuffers::count:
        default:
            // TODO: ASSERT
            break;
        }
        CreateBuffer(properties, eDecorationSets::GlobalDomain);
    }

    for (size_t i = 0; i < (uint32)eMaterialUniformBuffers::count; i++)
    {
        properties.type = eBufferType::uniform;
        properties.numberOfElements = 0;
        properties.offset = 0;
        switch ((eMaterialUniformBuffers)i)
        {
        case Hail::eMaterialUniformBuffers::vulkanTutorialUniformBUffer:
            properties.numberOfElements = 1;
            properties.elementByteSize = sizeof(TutorialUniformBufferObject);
            break;
        case Hail::eMaterialUniformBuffers::count:
        default:
            // TODO: ASSERT
            break;
        }
        CreateBuffer(properties, eDecorationSets::MaterialTypeDomain);
    }

    for (size_t i = 0; i < (uint32)eMaterialBuffers::count; i++)
    {
        properties.type = eBufferType::structured;
        properties.offset = 0;
        switch ((eMaterialBuffers)i)
        {
        case Hail::eMaterialBuffers::spriteInstanceBuffer:
            properties.numberOfElements = MAX_NUMBER_OF_SPRITES;
            properties.elementByteSize = sizeof(SpriteInstanceData);
            break;
        case Hail::eMaterialBuffers::lineInstanceBuffer:
            properties.numberOfElements = MAX_NUMBER_OF_DEBUG_LINES;
            properties.elementByteSize = sizeof(DebugLineData);
            break;
        case Hail::eMaterialBuffers::count:
        default:
            // TODO: ASSERT
            break;
        }
        CreateBuffer(properties, eDecorationSets::MaterialTypeDomain);
    }

	return true;
}
