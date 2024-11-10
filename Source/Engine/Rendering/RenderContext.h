#pragma once
#include "Types.h"

#include "Containers\StaticArray\StaticArray.h"

namespace Hail
{
	class BufferObject;
	class MaterialManager;
	class ResourceManager;
	class TextureResource;
	class Pipeline;


	class RenderContext
	{
	public:
		RenderContext(ResourceManager* pResourceManager);

		void SetBufferAtSlot(BufferObject* pBuffer, uint32 slot);
		void SetTextureAtSlot(TextureResource* pTexture, uint32 slot);

		// Creates a complete state for a pipeline if it does not exist, this pipeline will be with the bound resources. 
		void SetPipelineState(Pipeline* pPipeline);

		TextureResource* GetBoundTextureAtSlot(uint32 slot);
		BufferObject* GetBoundStructuredBufferAtSlot(uint32 slot);
		BufferObject* GetBoundUniformBufferAtSlot(uint32 slot);

		void UploadDataToBuffer(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData);

	private:


		StaticArray<TextureResource*, 16> m_pBoundTextures;
		StaticArray<BufferObject*, 16> m_pBoundStructuredBuffers;
		StaticArray<BufferObject*, 16> m_pBoundUniformBuffers;

		ResourceManager* m_pResourceManager;
	};
}