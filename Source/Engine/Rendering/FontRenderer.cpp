#include "Engine_PCH.h"
#include "FontRenderer.h"

#include "HailEngine.h"
#include "Renderer.h"
#include "Resources\ResourceManager.h"
#include "Resources\RenderingResourceManager.h"
#include "Resources\ResourceRegistry.h"
#include "Resources\MaterialManager.h"
#include "RenderContext.h"
#include "MathUtils.h"
#include "RenderCommands.h"

namespace Hail
{
	constexpr uint32 locMaxNumberOfGlyphlets = 1028u;

	FontRenderer::~FontRenderer()
	{
		H_ASSERT(m_pVertexBuffer == nullptr);
		H_ASSERT(m_pIndexBuffer == nullptr);
		H_ASSERT(m_pGlyphletBuffer == nullptr);
	}

	FontRenderer::FontRenderer(Renderer* pRenderer, ResourceManager* pResourceManager)
		: m_pRenderer(pRenderer)
		, m_pResourceManager(pResourceManager) 
		, m_pFontPipeline(nullptr)
	{
	}

	bool FontRenderer::Initialize()
	{
		//StringL fontFileDir = StringL::Format("%s%s", RESOURCE_DIR_OUT, "fonts/JetBrainsMono-Bold.ttf");
		//StringL fontFileDir = StringL::Format("%s%s", RESOURCE_DIR_OUT, "fonts/Pine.ttf");
		StringL fontFileDir = StringL::Format("%s%s", RESOURCE_DIR_OUT, "fonts/Roboto-Medium.ttf");
		m_fontData = TTF_ParseFontFile(fontFileDir);

		BufferProperties fontVertexBufferProperties;
		fontVertexBufferProperties.elementByteSize = sizeof(glm::vec4);
		fontVertexBufferProperties.numberOfElements = m_fontData.m_renderVerts.Size();
		fontVertexBufferProperties.type = eBufferType::structured;
		fontVertexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
		fontVertexBufferProperties.accessQualifier = eShaderAccessQualifier::ReadOnly;
		fontVertexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
		m_pVertexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(fontVertexBufferProperties, "Font Vertex Buffer");

		BufferProperties triangleListBufferProperties;
		triangleListBufferProperties.elementByteSize = sizeof(GlyphTri);
		triangleListBufferProperties.numberOfElements = m_fontData.m_glyphData.m_triangles.Size();
		triangleListBufferProperties.type = eBufferType::structured;
		triangleListBufferProperties.domain = eShaderBufferDomain::GpuOnly;
		triangleListBufferProperties.accessQualifier = eShaderAccessQualifier::ReadOnly;
		triangleListBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
		m_pIndexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(triangleListBufferProperties, "Font Glyph Triangle Buffer");

		BufferProperties glyphletInstanceListProps;
		glyphletInstanceListProps.elementByteSize = sizeof(RenderGlypghlet);
		glyphletInstanceListProps.numberOfElements = locMaxNumberOfGlyphlets;
		glyphletInstanceListProps.type = eBufferType::structured;
		glyphletInstanceListProps.domain = eShaderBufferDomain::CpuToGpu;
		glyphletInstanceListProps.accessQualifier = eShaderAccessQualifier::ReadOnly;
		glyphletInstanceListProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pGlyphletBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(glyphletInstanceListProps, "Font Glyphlet Buffer");

		BufferProperties fontTextCommandBufferProps;
		fontTextCommandBufferProps.elementByteSize = sizeof(glm::vec4);
		fontTextCommandBufferProps.numberOfElements = MAX_NUMBER_OF_TEXT_COMMANDS;
		fontTextCommandBufferProps.type = eBufferType::structured;
		fontTextCommandBufferProps.domain = eShaderBufferDomain::CpuToGpu;
		fontTextCommandBufferProps.accessQualifier = eShaderAccessQualifier::ReadOnly;
		fontTextCommandBufferProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pTextCommandBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(fontTextCommandBufferProps, "Font Instance Position Buffer");

		BufferProperties textBatchOffsetBufferProps;
		textBatchOffsetBufferProps.elementByteSize = sizeof(uint32) * 4u;
		textBatchOffsetBufferProps.numberOfElements = MAX_NUMBER_OF_TEXT_COMMANDS / 4;
		textBatchOffsetBufferProps.type = eBufferType::uniform;
		textBatchOffsetBufferProps.domain = eShaderBufferDomain::CpuToGpu;
		textBatchOffsetBufferProps.accessQualifier = eShaderAccessQualifier::ReadOnly;
		textBatchOffsetBufferProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pBatchOffsetBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(textBatchOffsetBufferProps, "Font Batch Offset Buffer");

		ResourceRegistry& reg = GetResourceRegistry();
		MaterialManager* pMatManager = m_pResourceManager->GetMaterialManager();

		MaterialCreationProperties matProperties{};

		RelativeFilePath meshProjectPath("resources/shaders/MS_fontBasic.shr");
		if (const MetaResource* metaData = reg.GetResourceMetaInformation(ResourceType::Shader,
		pMatManager->ImportShaderResource(meshProjectPath.GetFilePath(), eShaderStage::Mesh)))
		{
			matProperties.m_shaders[0].m_id = metaData->GetGUID();
			matProperties.m_shaders[0].m_type = eShaderStage::Mesh;
		}

		RelativeFilePath fragmentProjectPath("resources/shaders/FS_fontBasic.shr");
		if (const MetaResource* metaData = reg.GetResourceMetaInformation(ResourceType::Shader,
			pMatManager->ImportShaderResource(fragmentProjectPath.GetFilePath(), eShaderStage::Fragment)))
		{
			matProperties.m_shaders[1].m_id = metaData->GetGUID();
			matProperties.m_shaders[1].m_type = eShaderStage::Fragment;
		}

		matProperties.m_baseMaterialType = eMaterialType::CUSTOM;
		matProperties.m_typeRenderPass = eMaterialType::SPRITE;
		m_pFontPipeline = pMatManager->CreateMaterialPipeline(matProperties);

		RenderContext* pContext = m_pRenderer->GetCurrentContext();
		pContext->UploadDataToBuffer(m_pVertexBuffer, m_fontData.m_renderVerts.Data(), m_fontData.m_renderVerts.Size() * sizeof(glm::vec2));
		pContext->UploadDataToBuffer(m_pIndexBuffer, m_fontData.m_glyphData.m_triangles.Data(), m_fontData.m_glyphData.m_triangles.Size() * sizeof(GlyphTri));

		m_glyphletsToRender.Prepare(locMaxNumberOfGlyphlets);
		m_textCommandsToRender.Prepare(MAX_NUMBER_OF_TEXT_COMMANDS);
		m_batchOffsetToInstanceStart.Prepare(MAX_NUMBER_OF_TEXT_COMMANDS);
		return m_pFontPipeline != nullptr;
	}

	void FontRenderer::Cleanup()
	{
		H_ASSERT(m_pVertexBuffer);
		m_pVertexBuffer->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pVertexBuffer);

		H_ASSERT(m_pIndexBuffer);
		m_pIndexBuffer->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pIndexBuffer);

		H_ASSERT(m_pGlyphletBuffer);
		m_pGlyphletBuffer->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pGlyphletBuffer);

		H_ASSERT(m_pTextCommandBuffer);
		m_pTextCommandBuffer->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pTextCommandBuffer);

		H_ASSERT(m_pBatchOffsetBuffer);
		m_pBatchOffsetBuffer->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pBatchOffsetBuffer);

		m_pFontPipeline->CleanupResource(*m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pFontPipeline);
	}

	void localGetCompoundRenderGlyphlet(const TTF_FontStruct& fontData, GrowingArray<RenderGlypghlet>& listToFill, RenderGlypghlet glyphlet, const Glyph& glyph)
	{
		for (size_t iCGlyph = 0; iCGlyph < glyph.m_indexOffset; iCGlyph++)
		{
			const Glyph& compoundGlyph = fontData.m_glyphData.m_compoundGlyphs[glyph.m_vertexOffset + iCGlyph];
			if (compoundGlyph.m_bIsSimpleGlyph)
			{
				RenderGlypghlet glyphletToAdd = glyphlet;
				glyphletToAdd.indexOffset = compoundGlyph.m_indexOffset;
				glyphletToAdd.numberOfTrianglesNumberOfVertices = compoundGlyph.m_numberOfTrianglesNumberOfVertices;
				glyphletToAdd.vertexOffset = compoundGlyph.m_vertexOffset;
				if (listToFill.Size() < locMaxNumberOfGlyphlets)
					listToFill.Add(glyphletToAdd);
				else
					H_ERROR("To many glyphlets spawned! Either increase buffer size or make better culling");
			}
			else
			{
				localGetCompoundRenderGlyphlet(fontData, listToFill, glyphlet, compoundGlyph);
			}
		}
	}

	void FontRenderer::Prepare(const RenderCommandPool& poolOfCommands)
	{
		RenderContext* pContext = m_pRenderer->GetCurrentContext();

		glm::uvec2 resolution = m_pResourceManager->GetSwapChain()->GetTargetResolution();

		glm::vec2 resolutionToFontRatio = glm::vec2((float)resolution.x / 1920.f, (float)resolution.y / 1080.f);

		float aspectRatioY = (float)resolution.y / (float)resolution.x;
		float aspectRatioX = (float)resolution.x / (float)resolution.y;
		glm::vec2 pixelSize = glm::vec2(1.f / resolution.x, 1.f / resolution.y);
		pixelSize.x = pixelSize.x * aspectRatioY;

		m_glyphletsToRender.RemoveAll();
		m_textCommandsToRender.RemoveAll();
		m_batchOffsetToInstanceStart.RemoveAll();
		m_batchNumberOfGlypsToRender.RemoveAll();
		for (uint32 iLayer = 0; iLayer < poolOfCommands.m_layersBatchOffset.Size(); iLayer++)
		{
			uint32 nextOffset = iLayer + 1 == poolOfCommands.m_layersBatchOffset.Size() ? poolOfCommands.m_batches.Size() : poolOfCommands.m_layersBatchOffset[iLayer + 1];
			uint32 numberOfBatches = nextOffset - poolOfCommands.m_layersBatchOffset[iLayer];
			for (uint32 iBatch = 0; iBatch < numberOfBatches; iBatch++)
			{
				const Batch2DInfo& batchToRender = poolOfCommands.m_batches[poolOfCommands.m_layersBatchOffset[iLayer] + iBatch];
				if (batchToRender.m_type == eCommandType::Text)
				{
					const uint32 batchGlyphOffset = m_batchOffsetToInstanceStart.Add(m_glyphletsToRender.Size());
					for (uint32 iTextCommand = 0; iTextCommand < batchToRender.m_numberOfInstances; iTextCommand++)
					{
						const RenderCommand2DBase& textCommandBase = poolOfCommands.m_2DRenderCommands[batchToRender.m_instanceOffset + iTextCommand];
						H_ASSERT((textCommandBase.m_index_materialIndex_flags.u & IsSpriteFlagMask) == false, "Invalid RenderCommand");
						const RenderData_Text& textData = poolOfCommands.m_textData[textCommandBase.m_dataIndex];
						H_ASSERT(textData.text.Length(), "Invalid text command");
						const StringLW& stringToRender = textData.text;
						uint32 stringL = textData.text.Length();
						glm::vec2 glyphPosition = textCommandBase.m_transform.GetPosition();
						uint32 fontSize = textData.textSize; // the font size is relative to the height of 1080p

						//correct for aspectRatio
						float adjustedFontSize = (float)fontSize * resolutionToFontRatio.y;
						glm::vec2 pixelSizeOfGlyph = pixelSize * adjustedFontSize;
						const uint32 packedColor = textCommandBase.m_color.GetColorPacked();

						H_ASSERT(m_textCommandsToRender.Size() + 1 < MAX_NUMBER_OF_TEXT_COMMANDS, "To many text render commands.");

						glm::vec4 packedPositionRotation = { glyphPosition.x, glyphPosition.y, textCommandBase.m_transform.GetRotationRad(), 0.f };
						m_textCommandsToRender.Add(packedPositionRotation);
						glm::vec2 glyphRelativePosition = { 0.0, 0.0 };
						for (size_t i = 0; i < stringL; i++)
						{
							const wchar_t& charToRender = stringToRender[i];
							if (charToRender == L' ')
							{
								glyphRelativePosition.x += pixelSizeOfGlyph.x;
							}
							else
							{
								uint16 glyphID = m_fontData.m_uniCodeToGlyphID[charToRender];
								const Glyph& glyph = m_fontData.m_glyphData.m_glyphs[glyphID];

								const glm::ivec2 glyphExtents = { glyph.m_maxExtent.x - glyph.m_minExtent.x, glyph.m_maxExtent.y - glyph.m_minExtent.y };

								float xRatioOfAdvanceWidth = ((float)glyph.m_advanceWidth / (float)m_fontData.m_glyphExtents.x);

								float advanceWidth = (pixelSize.x * adjustedFontSize * xRatioOfAdvanceWidth) * aspectRatioX;
								float glyphSize = pixelSizeOfGlyph.x * ((float)glyphExtents.x / (float)m_fontData.m_glyphExtents.x) * aspectRatioX;
								float leftSideBearing = ((float)glyph.m_leftSideBearing / (float)m_fontData.m_glyphExtents.x) * pixelSize.x;

								RenderGlypghlet glyphletToCreate;
								glyphletToCreate.relativePos = glm::vec2(glyphRelativePosition.x + leftSideBearing, glyphRelativePosition.y);
								glyphletToCreate.glyphletColor = packedColor;
								glyphletToCreate.glyphPixelSize = adjustedFontSize;
								glyphletToCreate.commandBufferIndex = iTextCommand;
								if (glyph.m_bIsSimpleGlyph)
								{
									glyphletToCreate.indexOffset = glyph.m_indexOffset;
									glyphletToCreate.numberOfTrianglesNumberOfVertices = glyph.m_numberOfTrianglesNumberOfVertices;
									glyphletToCreate.vertexOffset = glyph.m_vertexOffset;
									if (m_glyphletsToRender.Size() < locMaxNumberOfGlyphlets)
										m_glyphletsToRender.Add(glyphletToCreate);
									else
										H_ERROR("To many glyphlets spawned! Either increase buffer size or make better culling");
								}
								else
								{
									localGetCompoundRenderGlyphlet(m_fontData, m_glyphletsToRender, glyphletToCreate, glyph);
								}
								glyphRelativePosition.x += advanceWidth + glyphSize;
							}
						}
					}
					m_batchNumberOfGlypsToRender.Add(m_glyphletsToRender.Size() - batchGlyphOffset);
				}
			}
		}

		pContext->StartTransferPass();
		pContext->UploadDataToBuffer(m_pGlyphletBuffer, m_glyphletsToRender.Data(), m_glyphletsToRender.Size() * sizeof(RenderGlypghlet));
		pContext->UploadDataToBuffer(m_pTextCommandBuffer, m_textCommandsToRender.Data(), m_textCommandsToRender.Size() * sizeof(glm::vec4));
		pContext->UploadDataToBuffer(m_pBatchOffsetBuffer, m_batchOffsetToInstanceStart.Data(), MAX_NUMBER_OF_TEXT_COMMANDS * sizeof(uint32));
		pContext->EndTransferPass();
	}

	void FontRenderer::RenderBatch(uint32 numberOfInstances, uint32 batchOffset)
	{
		RenderContext* pContext = m_pRenderer->GetCurrentContext();

		pContext->SetBufferAtSlot(m_pVertexBuffer, 0);
		pContext->SetBufferAtSlot(m_pIndexBuffer, 1);
		pContext->SetBufferAtSlot(m_pGlyphletBuffer, 2);
		pContext->SetBufferAtSlot(m_pTextCommandBuffer, 3);
		pContext->SetBufferAtSlot(m_pBatchOffsetBuffer, 4);

		pContext->BindMaterial(m_pFontPipeline->m_pPipeline);
		glm::uvec4 pushConstantData = glm::uvec4(batchOffset, 0, 0, 0);
		pContext->SetPushConstantValue(&pushConstantData);

		const uint32 numberOfInstancesToRender = m_batchNumberOfGlypsToRender[batchOffset];
		const uint32 offsetToInstance = m_batchOffsetToInstanceStart[batchOffset];
		pContext->RenderMeshlets(glm::uvec3(numberOfInstancesToRender, 1, 1));
	}



}