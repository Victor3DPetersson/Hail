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
	constexpr uint32 locMaxNumberOfTextCommands = 128u;

	// TODO: Make a proper color utility class
	uint32 locPackColor(glm::vec3 colorToConvert)
	{
		// Drag down values above 1.0
		colorToConvert = glm::min(colorToConvert, 1.0f);

		return uint32(uint8(colorToConvert.x * 255.f) << 16 | uint8(colorToConvert.y * 255.f) << 8 | uint8(colorToConvert.z * 255.f));
	}

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
		fontVertexBufferProperties.offset = 0;
		fontVertexBufferProperties.type = eBufferType::structured;
		fontVertexBufferProperties.domain = eShaderBufferDomain::GpuOnly;
		fontVertexBufferProperties.usage = eShaderBufferUsage::Read;
		fontVertexBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
		m_pVertexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(fontVertexBufferProperties);

		BufferProperties triangleListBufferProperties;
		triangleListBufferProperties.elementByteSize = sizeof(GlyphTri);
		triangleListBufferProperties.numberOfElements = m_fontData.m_glyphData.m_triangles.Size();
		triangleListBufferProperties.offset = 0;
		triangleListBufferProperties.type = eBufferType::structured;
		triangleListBufferProperties.domain = eShaderBufferDomain::GpuOnly;
		triangleListBufferProperties.usage = eShaderBufferUsage::Read;
		triangleListBufferProperties.updateFrequency = eShaderBufferUpdateFrequency::Once;
		m_pIndexBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(triangleListBufferProperties);

		BufferProperties glyphletInstanceListProps;
		glyphletInstanceListProps.elementByteSize = sizeof(RenderGlypghlet);
		glyphletInstanceListProps.numberOfElements = locMaxNumberOfGlyphlets;
		glyphletInstanceListProps.offset = 0;
		glyphletInstanceListProps.type = eBufferType::structured;
		glyphletInstanceListProps.domain = eShaderBufferDomain::CpuToGpu;
		glyphletInstanceListProps.usage = eShaderBufferUsage::Read;
		glyphletInstanceListProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pGlyphletBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(glyphletInstanceListProps);

		BufferProperties fontTextCommandBufferProps;
		fontTextCommandBufferProps.elementByteSize = sizeof(glm::vec4);
		fontTextCommandBufferProps.numberOfElements = locMaxNumberOfTextCommands;
		fontTextCommandBufferProps.offset = 0;
		fontTextCommandBufferProps.type = eBufferType::structured;
		fontTextCommandBufferProps.domain = eShaderBufferDomain::CpuToGpu;
		fontTextCommandBufferProps.usage = eShaderBufferUsage::Read;
		fontTextCommandBufferProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pTextCommandBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(fontTextCommandBufferProps);

		ResourceRegistry& reg = GetResourceRegistry();
		MaterialManager* pMatManager = m_pResourceManager->GetMaterialManager();
		MaterialCreationProperties matProperties{};
		if (const MetaResource* metaData = reg.GetResourceMetaInformation(ResourceType::Shader,
			pMatManager->ImportShaderResource("D:/Projects/YlvicProject/Hail/Source/Resources/Shaders/MS_fontBasic.mesh")))
		{
			matProperties.m_shaders[0].m_id = metaData->GetGUID();
			matProperties.m_shaders[0].m_type = eShaderType::Mesh;
		}
		if (const MetaResource* metaData = reg.GetResourceMetaInformation(ResourceType::Shader,
			pMatManager->ImportShaderResource("D:/Projects/YlvicProject/Hail/Source/Resources/Shaders/FS_fontBasic.frag")))
		{
			matProperties.m_shaders[1].m_id = metaData->GetGUID();
			matProperties.m_shaders[1].m_type = eShaderType::Fragment;
		}

		matProperties.m_baseMaterialType = eMaterialType::CUSTOM;
		matProperties.m_typeRenderPass = eMaterialType::SPRITE;
		m_pFontPipeline = pMatManager->CreateMaterialPipeline(matProperties);

		// TODO: Fix so that this can be uploaded once and not per frame.
		RenderContext* pContext = m_pRenderer->GetCurrentContext();
		pContext->StartTransferPass();
		pContext->UploadDataToBuffer(m_pVertexBuffer, m_fontData.m_renderVerts.Data(), m_fontData.m_renderVerts.Size() * sizeof(glm::vec2));
		pContext->UploadDataToBuffer(m_pIndexBuffer, m_fontData.m_glyphData.m_triangles.Data(), m_fontData.m_glyphData.m_triangles.Size() * sizeof(GlyphTri));
		pContext->EndTransferPass();

		m_glyphletsToRender.Prepare(locMaxNumberOfGlyphlets);
		m_textCommandsToRender.Prepare(locMaxNumberOfTextCommands);
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

		glm::uvec2 resolution = m_pResourceManager->GetSwapChain()->GetRenderTargetResolution();

		glm::vec2 resolutionToFontRatio = glm::vec2((float)resolution.x / 1920.f, (float)resolution.y / 1080.f);

		float aspectRatioY = (float)resolution.y / (float)resolution.x;
		float aspectRatioX = (float)resolution.x / (float)resolution.y;
		glm::vec2 pixelSize = glm::vec2(1.f / resolution.x, 1.f / resolution.y);
		pixelSize.x = pixelSize.x * aspectRatioY;

		m_glyphletsToRender.RemoveAll();
		m_textCommandsToRender.RemoveAll();

		for (uint32 iTextCommand = 0; iTextCommand < poolOfCommands.textCommands.Size(); iTextCommand++)
		{
			const StringLW& stringToRender = poolOfCommands.textCommands[iTextCommand].text;
			uint32 stringL = poolOfCommands.textCommands[iTextCommand].text.Length();
			glm::vec2 glyphPosition = poolOfCommands.textCommands[iTextCommand].transform.GetPosition();
			uint32 fontSize = poolOfCommands.textCommands[iTextCommand].textSize; // the font size is relative to the height of 1080p

			//correct for aspectRatio
			float adjustedFontSize = (float)fontSize * resolutionToFontRatio.y;
			glm::vec2 pixelSizeOfGlyph = pixelSize * adjustedFontSize;
			const uint32 packedColor = locPackColor(poolOfCommands.textCommands[iTextCommand].color);

			H_ASSERT(iTextCommand < locMaxNumberOfTextCommands, "To many text render commands.");

			glm::vec4 packedPositionRotation = { glyphPosition.x, glyphPosition.y, poolOfCommands.textCommands[iTextCommand].transform.GetRotationRad(), 0.f };
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


		pContext->StartTransferPass();
		pContext->UploadDataToBuffer(m_pGlyphletBuffer, m_glyphletsToRender.Data(), m_glyphletsToRender.Size() * sizeof(RenderGlypghlet));
		pContext->UploadDataToBuffer(m_pTextCommandBuffer, m_textCommandsToRender.Data(), m_textCommandsToRender.Size() * sizeof(glm::vec4));
		pContext->EndTransferPass();
		// End of temp code.
	}

	void FontRenderer::Render()
	{
		if (!m_pFontPipeline || m_glyphletsToRender.Empty())
			return;

		RenderContext* pContext = m_pRenderer->GetCurrentContext();
		
		pContext->SetBufferAtSlot(m_pVertexBuffer, 0);
		pContext->SetBufferAtSlot(m_pIndexBuffer, 1);
		pContext->SetBufferAtSlot(m_pGlyphletBuffer, 2);
		pContext->SetBufferAtSlot(m_pTextCommandBuffer, 3);

		pContext->BindMaterial(m_pFontPipeline->m_pPipeline);
		pContext->SetPipelineState(m_pFontPipeline->m_pPipeline);
		pContext->RenderMeshlets(glm::uvec3(m_glyphletsToRender.Size(), 1, 1));
	}



}