#include "Engine_PCH.h"
#include "CloudRenderer.h"

#include "HailEngine.h"
#include "Renderer.h"
#include "Resources\ResourceManager.h"
#include "Resources\TextureManager.h"
#include "Resources\RenderingResourceManager.h"
#include "Resources\ResourceRegistry.h"
#include "Resources\MaterialManager.h"
#include "RenderContext.h"
#include "MathUtils.h"
#include "RenderCommands.h"
#include "Utility\DebugLineHelpers.h"

namespace Hail
{
	// TODO, link to website for credit
	// Dead reckoning alghorithm to generate a SDF to a texture from point data, found on the web. 
	void DeadReckoning(int width, int height, const GrowingArray<uint8>& binaryPixels, GrowingArray<float>& setSignedDistance) 
	{
		const auto contains = [&](int x, int y) -> bool 
		{
			return x >= 0 && x < width&& y >= 0 && y < height;
		};

		// I - a 2D binary image
		const bool outside = false;
		const bool inside = true;
		const auto I = [&](int x, int y) -> bool 
		{
			uint32 pixelCoord = x + y * width;
			return binaryPixels[pixelCoord] ? inside : outside;
		};

		// d - a 2D grey image representing the distance image
		GrowingArray<float> ds(width * height);
		ds.Fill();
		const float inf = static_cast<float>(256 * 256 * 256);
		const auto getd = [&](int x, int y) -> float { return ds[y * width + x]; };
		const auto setd = [&](int x, int y, float v) { ds[y * width + x] = v; };
		const auto distance = [](int x, int y) -> float 
		{
			return hypotf(static_cast<float>(x), static_cast<float>(y));
		};

		// p - for each pixel, the corresponding border point
		struct Point { int x, y; };
		GrowingArray<Point> ps(width * height);
		ps.Fill();
		const Point outOfBounds = { -1, -1 };
		const auto getp = [&](int x, int y) -> Point { return ps[y * width + x]; };
		const auto setp = [&](int x, int y, const Point& p) { ps[y * width + x] = p; };

		// initialize d
		// initialize immediate interior & exterior elements
		for (int y = 0; y < height; ++y) 
		{
			for (int x = 0; x < width; ++x) 
			{
				const bool c = I(x, y);
				const bool w = contains(x - 1, y) ? I(x - 1, y) : outside;
				const bool e = contains(x + 1, y) ? I(x + 1, y) : outside;
				const bool n = contains(x, y - 1) ? I(x, y - 1) : outside;
				const bool s = contains(x, y + 1) ? I(x, y + 1) : outside;
				if ((w != c) || (e != c) || (n != c) || (s != c)) 
				{
					setd(x, y, 0);
					setp(x, y, { x, y });
				}
				else 
				{
					setd(x, y, inf);
					setp(x, y, outOfBounds);
				}
			}
		}

		// Perform minimum distance choice for single pixel, single direction.
		enum class Dir 
		{
			NW, N, NE,      // NW=(x-1, y-1), N=(x, y-1), NE=(x+1, y-1)
			W, E,       // W =(x-1, y),               E =(x+1, y)
			SW, S, SE       // SW=(x-1, y+1), S=(x, y+1), SE=(x+1, y+1)
		};
		const auto f = [&](int x, int y, Dir dir) 
		{
			// d1 - distance between two adjacent pixels in either the x or y direction
			const float d1 = 1.0f;

			// d2 - distance between two diagonally adjacent pixels (sqrt(2))
			const float d2 = d1 * 1.4142135623730950488f;

			int dx, dy;
			float od;
			switch (dir) 
			{
			default:
			case Dir::NW: dx = -1; dy = -1; od = d2; break; // first pass
			case Dir::N:  dx = 0; dy = -1; od = d1; break; // first pass
			case Dir::NE: dx = 1; dy = -1; od = d2; break; // first pass
			case Dir::W:  dx = -1; dy = 0; od = d1; break; // first pass
			case Dir::E:  dx = 1; dy = 0; od = d1; break; // final pass
			case Dir::SW: dx = -1; dy = 1; od = d2; break; // final pass
			case Dir::S:  dx = 0; dy = 1; od = d1; break; // final pass
			case Dir::SE: dx = 1; dy = 1; od = d2; break; // final pass
			}
			const bool b = contains(x + dx, y + dy);
			const float cd = b ? getd(x + dx, y + dy) : inf;
			if (cd + od < getd(x, y)) 
			{
				const Point p = b ? getp(x + dx, y + dy) : outOfBounds;
				setp(x, y, p);
				const int xx = x - p.x;
				const int yy = y - p.y;
				const float nd = distance(xx, yy);
				setd(x, y, nd);
			}
		};

		// perform the first pass
		for (int y = 0; y < height; ++y) 
		{           // top to bottom
			for (int x = 0; x < width; ++x) 
			{        // left to right
				static const Dir dirs[] = { Dir::NW, Dir::N, Dir::NE, Dir::W };
				for (const Dir dir : dirs) 
				{
					f(x, y, dir);
				}
			}
		}

		// perform the final pass
		for (int y = height - 1; y >= 0; --y) 
		{        // bottom to top
			for (int x = width - 1; x >= 0; --x) 
			{     // right to left
				static const Dir dirs[] = { Dir::E, Dir::SW, Dir::S, Dir::SE };
				for (const Dir dir : dirs) 
				{
					f(x, y, dir);
				}
			}
		}

		// indicate inside & outside
		for (int y = 0; y < height; ++y) 
		{
			for (int x = 0; x < width; ++x) 
			{
				float d = getd(x, y);
				if (I(x, y) == outside) 
				{
					d = -d;                         // Negative distance means outside.
				}

				uint32 pixelCoord = x + y * width;
				setSignedDistance[pixelCoord] = d * -1.f;
			}
		}
	}


	constexpr uint32 locMaxNumberOfGlyphlets = 1028u;

	CloudRenderer::~CloudRenderer()
	{
		H_ASSERT(m_pCloudPipeline == nullptr);
		H_ASSERT(m_pCloudBuffer == nullptr);
	}

	CloudRenderer::CloudRenderer(Renderer* pRenderer, ResourceManager* pResourceManager)
		: m_pRenderer(pRenderer)
		, m_pResourceManager(pResourceManager) 
		, m_pCloudPipeline(nullptr)
		, m_pCloudBuffer(nullptr)
		, m_pSdfTexture(nullptr)
		, m_pSdfView(nullptr)
		, m_numberOfPointsUploaded(0u)
	{
	}

	glm::vec2 Vogel(uint32 sampleIndex, uint32 samplesCount, float Offset)
	{
		float r = sqrt(float(sampleIndex) + 0.5f) / sqrt(float(samplesCount));
		float theta = float(sampleIndex) * Math::GoldenAngle + Offset;
		return r * glm::vec2(cos(theta), sin(theta));
	}

	bool CloudRenderer::Initialize()
	{
		// Hard coded shape for debugging
		GrowingArray<glm::uvec2> pointBaseList = {
			{1, 7},
			{2, 7},
			{2, 6},
			{3, 7},
			{3, 6},
			{3, 4},
			{4, 7},
			{4, 6},
			{4, 5},
			{4, 4},
			{4, 3},
			{4, 2},
			{5, 7},
			{5, 6},
			{5, 5},
			{5, 4},
			{5, 3},
			{5, 2},
			{6, 7},
			{6, 6},
			{6, 5},
			{6, 4},
			{6, 3},
			{7, 7},
			{7, 6},
			{7, 5},
			{7, 4},
			{7, 3},
			{7, 2},
			{8, 7},
			{8, 6},
			{8, 5},
			{9, 7}
		};

		for (uint32 i = 0; i < pointBaseList.Size(); i++)
		{
			const uint32 rnd = ((pointBaseList[i].x * 5u + pointBaseList[i].y * 7u) >> (pointBaseList[i].x & 3u));
			for (uint32 iPRand = 0; iPRand < 4; ++iPRand)
			{
				glm::vec2 vogelNoise = Vogel(rnd + iPRand, 128u, 0.1 * iPRand) * 0.5f;
				glm::vec2 finalPos = (glm::vec2( pointBaseList[i].x + 0.5, pointBaseList[i].y + 0.5) / 11.f + vogelNoise / 11.f);

				m_pointsOnTheGPU.Add(finalPos);
			}
		}
		const float cloudPointRadius = 0.1f;
		const float cloudPointRadiusSq = cloudPointRadius * cloudPointRadius;
		uint32 width = 128u;
		uint32 height = 128u;
		GrowingArray<uint8> imageRepresentationOfCloud(width * height);
		imageRepresentationOfCloud.Fill();
		for (uint32 x = 0; x < width; x++)
		{
			for (uint32 y = 0; y < height; y++)
			{
				uint32 imageCoord = x + y * height;
				glm::vec2 cloudCoord = glm::vec2((float)(x + 0.5) / width, (float)(y + 0.5) / height);
				uint32 numberOfPointsInRadius = 0;
				for (uint32 iPoint = 0; iPoint < m_pointsOnTheGPU.Size(); iPoint++)
				{
					glm::vec2 cloudPointToSamplePoint = cloudCoord - m_pointsOnTheGPU[iPoint];
					float distanceSquared = glm::dot(cloudPointToSamplePoint, cloudPointToSamplePoint);
					if (distanceSquared <= cloudPointRadiusSq)
					{
						numberOfPointsInRadius++;
					}
				}
				imageRepresentationOfCloud[imageCoord] = numberOfPointsInRadius >= 4u ? numberOfPointsInRadius : 0;
			}
		}
		m_cloudSdfTexture.PrepareAndFill(width * height);
		DeadReckoning(width, height, imageRepresentationOfCloud, m_cloudSdfTexture);

		//pointsToPutOnTheGpu.Add(glm::vec2(0.95, 0.05));
		m_pointsOnTheGPU.Add(glm::vec2(0.9, 0.1));
		//pointsToPutOnTheGpu.Add(glm::vec2(0.85, 0.15));
		m_pointsOnTheGPU.Add(glm::vec2(0.8, 0.23));

		RenderContext* pContext = m_pRenderer->GetCurrentContext();

		CompiledTexture sdfTextureValues;
		sdfTextureValues.properties.height = height;
		sdfTextureValues.properties.width = width;
		sdfTextureValues.properties.textureType = (uint32)eTextureSerializeableType::R32F;
		sdfTextureValues.properties.format = eTextureFormat::R32_SFLOAT;
		sdfTextureValues.compiledColorValues = m_cloudSdfTexture.Data();
		sdfTextureValues.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
		m_pSdfTexture = m_pResourceManager->GetTextureManager()->CreateTexture(pContext, "CloudSdfTexture", sdfTextureValues);

		TextureViewProperties viewProps;
		viewProps.pTextureToView = m_pSdfTexture;
		viewProps.viewUsage = eTextureUsage::Texture;
		m_pSdfView = m_pResourceManager->GetTextureManager()->CreateTextureView(viewProps);

		BufferProperties cloudPointBufferProps;
		cloudPointBufferProps.elementByteSize = sizeof(glm::vec2);
		cloudPointBufferProps.numberOfElements = m_pointsOnTheGPU.Size();
		cloudPointBufferProps.type = eBufferType::structured;
		cloudPointBufferProps.domain = eShaderBufferDomain::CpuToGpu;
		cloudPointBufferProps.accessQualifier = eShaderAccessQualifier::ReadOnly;
		cloudPointBufferProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pCloudBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(cloudPointBufferProps, "Cloud PointBuffer");

		ResourceRegistry& reg = GetResourceRegistry();
		MaterialManager* pMatManager = m_pResourceManager->GetMaterialManager();
		MaterialCreationProperties matProperties{};
		RelativeFilePath vertexProjectPath("resources/shaders/VS_fullscreenPass.shr");
		if (const MetaResource* metaData = reg.GetResourceMetaInformation(ResourceType::Shader,
			pMatManager->ImportShaderResource(vertexProjectPath.GetFilePath(), eShaderStage::Vertex)))
		{
			matProperties.m_shaders[0].m_id = metaData->GetGUID();
			matProperties.m_shaders[0].m_type = eShaderStage::Vertex;
		}

		RelativeFilePath fragmentProjectPath("resources/shaders/FS_cloudDrawing.shr");
		if (const MetaResource* metaData = reg.GetResourceMetaInformation(ResourceType::Shader,
			pMatManager->ImportShaderResource(fragmentProjectPath.GetFilePath(), eShaderStage::Fragment)))
		{
			matProperties.m_shaders[1].m_id = metaData->GetGUID();
			matProperties.m_shaders[1].m_type = eShaderStage::Fragment;
		}

		matProperties.m_baseMaterialType = eMaterialType::CUSTOM;
		matProperties.m_typeRenderPass = eMaterialType::FULLSCREEN_PRESENT_LETTERBOX;
		m_pCloudPipeline = pMatManager->CreateMaterialPipeline(matProperties);

		pContext->UploadDataToBuffer(m_pCloudBuffer, m_pointsOnTheGPU.Data(), m_pointsOnTheGPU.Size() * sizeof(glm::vec2));
		m_numberOfPointsUploaded = m_pointsOnTheGPU.Size();

		MaterialCreationProperties computeMatProperties{};
		RelativeFilePath computeProjectPath("resources/shaders/CS_calculateCloudCoverage.shr");
		if (const MetaResource* metaData = reg.GetResourceMetaInformation(ResourceType::Shader,
			pMatManager->ImportShaderResource(computeProjectPath.GetFilePath(), eShaderStage::Compute, false)))
		{
			computeMatProperties.m_shaders[0].m_id = metaData->GetGUID();
			computeMatProperties.m_shaders[0].m_type = eShaderStage::Compute;
		}
		computeMatProperties.m_baseMaterialType = eMaterialType::CUSTOM;
		computeMatProperties.m_typeRenderPass = eMaterialType::CUSTOM;
		m_pCloudCoveragePipeline = pMatManager->CreateMaterialPipeline(computeMatProperties);

		BufferProperties particleUniformBufferProps;
		particleUniformBufferProps.elementByteSize = sizeof(ParticleUniformBuffer);
		particleUniformBufferProps.numberOfElements = 1;
		particleUniformBufferProps.type = eBufferType::uniform;
		particleUniformBufferProps.domain = eShaderBufferDomain::CpuToGpu;
		particleUniformBufferProps.accessQualifier = eShaderAccessQualifier::ReadOnly;
		particleUniformBufferProps.updateFrequency = eShaderBufferUpdateFrequency::PerFrame;
		m_pParticleUniformBuffer = m_pResourceManager->GetRenderingResourceManager()->CreateBuffer(particleUniformBufferProps, "Particle Uniform Buffer");

		TextureProperties particleCoverageTextureProps;
		particleCoverageTextureProps.height = m_pResourceManager->GetSwapChain()->GetTargetResolution().y;
		particleCoverageTextureProps.width = m_pResourceManager->GetSwapChain()->GetTargetResolution().x;
		particleCoverageTextureProps.format = eTextureFormat::R8G8B8A8_UNORM;
		particleCoverageTextureProps.accessQualifier = eShaderAccessQualifier::WriteOnly;
		
		m_pParticleCoverageTexture = m_pResourceManager->GetTextureManager()->CreateTexture(pContext, "Particle Coverage Texture", particleCoverageTextureProps);

		TextureViewProperties particleCoverageProps;
		particleCoverageProps.pTextureToView = m_pParticleCoverageTexture;
		particleCoverageProps.viewUsage = eTextureUsage::Texture;
		m_pParticleCoverageView = m_pResourceManager->GetTextureManager()->CreateTextureView(particleCoverageProps);


		CloudParticle particle;
		particle.velocity = glm::vec2(0.f);
		m_cloudParticles.Prepare(m_numberOfPointsUploaded);
		for (size_t i = 0; i < m_numberOfPointsUploaded; i++)
		{
			particle.pos = m_pointsOnTheGPU[i];
			m_cloudParticles.Add(particle);
		}

		return m_pCloudPipeline != nullptr && m_pCloudCoveragePipeline != nullptr;
	}

	void CloudRenderer::Cleanup()
	{
		m_pParticleUniformBuffer->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pParticleUniformBuffer);
		m_pParticleCoverageView->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pParticleCoverageView);
		m_pParticleCoverageTexture->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pParticleCoverageTexture);


		m_pCloudBuffer->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pCloudBuffer);

		m_pSdfTexture->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pSdfTexture);

		m_pSdfView->CleanupResource(m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pSdfView);

		m_pCloudPipeline->CleanupResource(*m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pCloudPipeline);

		m_pCloudCoveragePipeline->CleanupResource(*m_pRenderer->GetRenderingDevice());
		SAFEDELETE(m_pCloudCoveragePipeline);
	}

	void CloudRenderer::Prepare(RenderCommandPool& poolOfCommands)
	{
		const float aspectRatio = m_pResourceManager->GetSwapChain()->GetTargetHorizontalAspectRatio();
		m_simulator.UpdateParticles(m_cloudParticles, poolOfCommands, m_pResourceManager->GetSwapChain()->GetTargetResolution(), m_cloudSdfTexture);
		DebugCircle debugCircle;
		debugCircle.scale = 20.0;
		debugCircle.color = { 3.0 / 255.f, 169.f / 255.f, 252.f / 255.f };
		for (uint32 i = 0; i < m_cloudParticles.Size(); i++)
		{
			m_pointsOnTheGPU[i] = m_cloudParticles[i].pos;
			debugCircle.pos = m_cloudParticles[i].pos;
			debugCircle.pos.x /= aspectRatio;
			//poolOfCommands.m_debugCircles.Add(debugCircle);
		}

		debugCircle.pos = glm::vec2(1.0, 0.5);
		debugCircle.pos.x /= aspectRatio;
		debugCircle.scale = 30.0;
		debugCircle.color = Color::Orange;

		// Bounds lines
		//DrawRect2D(poolOfCommands.m_debugLineCommands, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f / aspectRatio, 1.0f) );

		RenderContext* pContext = m_pRenderer->GetCurrentContext();
		pContext->StartTransferPass();
		pContext->UploadDataToBuffer(m_pCloudBuffer, m_pointsOnTheGPU.Data(), m_pointsOnTheGPU.Size() * sizeof(glm::vec2));

		m_ParticleUniforms.numberOfParticles = m_pointsOnTheGPU.Size();
		m_ParticleUniforms.particleSize = 20.f;

		pContext->UploadDataToBuffer(m_pParticleUniformBuffer, &m_ParticleUniforms, sizeof(ParticleUniformBuffer));

		pContext->EndTransferPass();

	}

	void CloudRenderer::Render()
	{
		RenderContext* pContext = m_pRenderer->GetCurrentContext();

		// Bind Dispatch stuff
		pContext->SetBufferAtSlot(m_pCloudBuffer, 0);
		pContext->SetBufferAtSlot(m_pParticleUniformBuffer, 2);
		pContext->SetTextureAtSlot(m_pParticleCoverageView, 1);

		pContext->BindMaterial(m_pCloudCoveragePipeline->m_pPipeline);
		glm::uvec2 resolution = m_pResourceManager->GetSwapChain()->GetTargetResolution();
		pContext->Dispatch(glm::uvec3(resolution.x / 64u, resolution.y / 8u, 1u));

		pContext->SetTextureAtSlot(m_pParticleCoverageView, 1);
		pContext->SetTextureAtSlot(m_pSdfView, 2);

		pContext->BindMaterial(m_pCloudPipeline->m_pPipeline);
		glm::uvec4 pushConstantData = glm::uvec4(m_numberOfPointsUploaded, 0, 0, 0);
 		pContext->SetPushConstantValue(&pushConstantData);

		pContext->RenderFullscreenPass();
	}
}