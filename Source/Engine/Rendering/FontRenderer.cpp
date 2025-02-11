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


namespace Hail
{
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
		glyphletInstanceListProps.numberOfElements = 1024;
		glyphletInstanceListProps.offset = 0;
		glyphletInstanceListProps.type = eBufferType::structured;
		glyphletInstanceListProps.domain = eShaderBufferDomain::CpuToGpu;
		glyphletInstanceListProps.usage = eShaderBufferUsage::Read;
		glyphletInstanceListProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pGlyphletBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(glyphletInstanceListProps);

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

		m_glyphletsToRender.Prepare(128);

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
				listToFill.Add(glyphletToAdd);
			}
			else
			{
				localGetCompoundRenderGlyphlet(fontData, listToFill, glyphlet, compoundGlyph);
			}
		}
	}

	void FontRenderer::Prepare()
	{
		RenderContext* pContext = m_pRenderer->GetCurrentContext();

		// Temp code below, should be driven by render commands.
		const wchar_t* helloWorld = L"En katt �r ett litet sm� djur! :}";

		glm::vec2 glyphPosition = { 0.0, 0.5 };
		uint32 fontSize = 48; // the font size is relative to the height of 1080p
		glm::uvec2 resolution = m_pResourceManager->GetSwapChain()->GetRenderTargetResolution();

		glm::vec2 resolutionToFontRatio = glm::vec2((float)resolution.x / 1920.f, (float)resolution.y / 1080.f);

		float adjustedFontSize = (float)fontSize * resolutionToFontRatio.y;

		float aspectRatioY = (float)resolution.y / (float)resolution.x;
		float aspectRatioX = (float)resolution.x / (float)resolution.y;

		glm::vec2 pixelSize = glm::vec2(1.f / resolution.x, 1.f / resolution.y);
		//correct for aspectRatio
		pixelSize.x = pixelSize.x * aspectRatioY;
		glm::vec2 pixelSizeOfGlyph = pixelSize * adjustedFontSize;

		m_glyphletsToRender.RemoveAll();

		uint32 stringL = StringLength(helloWorld);
		for (size_t i = 0; i < stringL; i++)
		{
			if (helloWorld[i] == L' ')
			{
				glyphPosition.x += pixelSizeOfGlyph.x;
			}
			else
			{
				uint16 glyphID = m_fontData.m_uniCodeToGlyphID[helloWorld[i]];
				const Glyph& glyph = m_fontData.m_glyphData.m_glyphs[glyphID];

				const glm::ivec2 glyphExtents = { glyph.m_maxExtent.x - glyph.m_minExtent.x, glyph.m_maxExtent.y - glyph.m_minExtent.y };

				float xRatioOfAdvanceWidth = ((float)glyph.m_advanceWidth / (float)m_fontData.m_glyphExtents.x);

				float advanceWidth = (pixelSize.x * adjustedFontSize * xRatioOfAdvanceWidth) * aspectRatioX;
				float glyphSize = pixelSizeOfGlyph.x * ((float)glyphExtents.x / (float)m_fontData.m_glyphExtents.x) * aspectRatioX;
				float leftSideBearing = ((float)glyph.m_leftSideBearing / (float)m_fontData.m_glyphExtents.x) * pixelSize.x;

				RenderGlypghlet glyphletToCreate;
				glyphletToCreate.pos = glm::vec2(glyphPosition.x + leftSideBearing, glyphPosition.y);
				glyphletToCreate.glyphletColor = 1u;
				glyphletToCreate.glyphPixelSize = adjustedFontSize;
				if (glyph.m_bIsSimpleGlyph)
				{
					glyphletToCreate.indexOffset = glyph.m_indexOffset;
					glyphletToCreate.numberOfTrianglesNumberOfVertices = glyph.m_numberOfTrianglesNumberOfVertices;
					glyphletToCreate.vertexOffset = glyph.m_vertexOffset;
					m_glyphletsToRender.Add(glyphletToCreate);
				}
				else
				{
					localGetCompoundRenderGlyphlet(m_fontData, m_glyphletsToRender, glyphletToCreate, glyph);
				}
				glyphPosition.x += advanceWidth + glyphSize;
			}
		}
		pContext->StartTransferPass();
		pContext->UploadDataToBuffer(m_pGlyphletBuffer, m_glyphletsToRender.Data(), m_glyphletsToRender.Size() * sizeof(RenderGlypghlet));
		pContext->EndTransferPass();
		// End of temp code.
	}

	void FontRenderer::Render()
	{
		if (!m_pFontPipeline)
			return;

		RenderContext* pContext = m_pRenderer->GetCurrentContext();

		pContext->SetBufferAtSlot(m_pVertexBuffer, 0);
		pContext->SetBufferAtSlot(m_pIndexBuffer, 1);
		pContext->SetBufferAtSlot(m_pGlyphletBuffer   , 2);

		pContext->BindMaterial(m_pFontPipeline->m_pPipeline);
		pContext->SetPipelineState(m_pFontPipeline->m_pPipeline);
		pContext->RenderMeshlets(glm::uvec3(m_glyphletsToRender.Size(), 1, 1));
	}



}