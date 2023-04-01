//Interface for the entire engine
#pragma once
#include <atomic>
#include "StartupAttributes.h"
#include "RenderCommands.h"
#include <atomic>
#include "InputMappings.h"

class InputHandler;
class Renderer;
class ApplicationWindow;
class Timer;

namespace Hail
{
	struct RecordedImGuiCommands
	{

	};
	struct ApplicationFrameData
	{
		InputMap inputData;
		RenderCommandPool* renderPool;
		RecordedImGuiCommands* recordedImguiCommands;
	};

	class ThreadSyncronizer
	{
	public:
		ThreadSyncronizer() = default;
		void Init(float tickTimer);
		void SynchronizeAppData(InputHandler& inputHandler);
		void SynchronizeRenderData(float frameDeltaTime);
		ApplicationFrameData& GetAppFrameData() { return m_appData; }
		RecordedImGuiCommands* GetToEngineImGuiData() {	return &m_toEngineImguiData; }
		RenderCommandPool& GetRenderPool() { return m_rendererCommandPool; }
	private:
		void SwapBuffersInternal();
		void ClearApplicationBuffers();
		void TransferBufferSizes();
		void LerpRenderBuffers();

		void LerpSpriteCommand(const RenderCommand_Sprite& readSprite, const RenderCommand_Sprite& lastReadSprite,	RenderCommand_Sprite& writeSprite, float t);

		RenderCommandPool m_renderCommandPools[3]{};
		RenderCommandPool m_rendererCommandPool{};
		ApplicationFrameData m_appData{};
		RecordedImGuiCommands m_toEngineImguiData{};
		uint32_t m_currentActiveRenderPoolWrite = 0;
		uint32_t m_currentActiveRenderPoolRead = 0;
		uint32_t m_currentActiveRenderPoolLastRead = 0;
		float m_renderBlendValue = 0.0f;
		float m_engineTickRate = 0.0f;
		float m_currentRenderTimer = 0.0f;
	};
}

