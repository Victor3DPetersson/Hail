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
	class ImGuiCommandRecorder;
	struct ApplicationFrameData
	{
		InputMap inputData;
		RenderCommandPool* renderPool;
		ImGuiCommandRecorder* imguiCommandRecorder;
	};

	class ThreadSyncronizer
	{
	public:
		ThreadSyncronizer() = default;
		void Init(float tickTimer);
		void SynchronizeAppData(InputHandler& inputHandler, ImGuiCommandRecorder& imguiCommandRecorder);
		void SynchronizeRenderData(float frameDeltaTime);
		ApplicationFrameData& GetAppFrameData() { return m_appData; }
		RenderCommandPool& GetRenderPool() { return m_rendererCommandPool; }
	private:
		void SwapBuffersInternal();
		void ClearApplicationBuffers();
		void TransferBufferSizes();
		void LerpRenderBuffers();
		void LerpSprites(float tValue);
		void Lerp3DModels(float tValue);

		

		RenderCommandPool m_renderCommandPools[3]{};
		RenderCommandPool m_rendererCommandPool{};
		ApplicationFrameData m_appData{};
		uint32_t m_currentActiveRenderPoolWrite = 0;
		uint32_t m_currentActiveRenderPoolRead = 0;
		uint32_t m_currentActiveRenderPoolLastRead = 0;
		float m_renderBlendValue = 0.0f;
		float m_engineTickRate = 0.0f;
		float m_currentRenderTimer = 0.0f;
	};
}

