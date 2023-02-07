#include "Engine_PCH.h"
#include "ThreadSynchronizer.h"
#include "InputHandler.h"

void Hail::ThreadSyncronizer::Init(float tickTimer)
{
	m_engineTickRate = tickTimer;
	m_currentActiveRenderPoolWrite = 0;
	m_currentActiveRenderPoolRead = 1;
	m_currentActiveRenderPoolLastRead = 2;
	m_appData.renderPool = &m_renderCommandPools[m_currentActiveRenderPoolWrite];
}

void Hail::ThreadSyncronizer::SynchronizeAppData(InputHandler& inputHandler)
{
	SwapBuffersInternal();

	m_appData.inputData = inputHandler.GetInputMap();
	m_appData.renderPool = &m_renderCommandPools[m_currentActiveRenderPoolWrite];
	m_currentRenderTimer = 0.0f;
}

void Hail::ThreadSyncronizer::SynchronizeRenderData(float frameDeltaTime)
{
	m_currentRenderTimer += frameDeltaTime;

	LerpRenderBuffers();
}

void Hail::ThreadSyncronizer::SwapBuffersInternal()
{
	const uint32_t lastRead = m_currentActiveRenderPoolLastRead;
	m_currentActiveRenderPoolLastRead = m_currentActiveRenderPoolRead;
	m_currentActiveRenderPoolRead = m_currentActiveRenderPoolWrite;
	m_currentActiveRenderPoolWrite = lastRead;
}

void Hail::ThreadSyncronizer::LerpRenderBuffers()
{
	float tValue = m_currentRenderTimer / m_engineTickRate;
	const RenderCommandPool& readPool = m_renderCommandPools[m_currentActiveRenderPoolRead];
	const RenderCommandPool& lastReadPool = m_renderCommandPools[m_currentActiveRenderPoolLastRead];
	m_rendererCommandPool.renderCamera = Camera::LerpCamera(readPool.renderCamera, lastReadPool.renderCamera, tValue);

}
