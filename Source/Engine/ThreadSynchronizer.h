//Interface for the entire engine
#pragma once
#include <atomic>
#include "StartupAttributes.h"
#include "RenderCommands.h"
#include <atomic>
#include "InputMappings.h"


namespace Hail
{
	class ImGuiCommandRecorder;
	class InputHandler;
	class ResourceManager;
	class InputActionMap;

	struct ApplicationFrameData
	{
		InputMap rawInputData;
		InputActionMap* inputActionMap;
		RenderCommandPool* renderPool;
		ImGuiCommandRecorder* imguiCommandRecorder;
	};

	class ThreadSyncronizer
	{
	public:
		ThreadSyncronizer() = default;
		void Init(float tickTimer);
		void SynchronizeAppData(InputActionMap& inputActionMap, ImGuiCommandRecorder& imguiCommandRecorder, ResourceManager& resourceManager);
		void SynchronizeRenderData(float frameDeltaTime);
		// Moves sprites and the like to the correct position with the camera. Called on the application thread. 
		void PrepareApplicationData();
		ApplicationFrameData& GetAppFrameData() { return m_appData; }
		RenderCommandPool& GetRenderPool() { return m_rendererCommandPool; }
	private:
		void SwapBuffersInternal();
		void ClearApplicationBuffers();
		void TransferBufferSizes();
		void LerpRenderBuffers();
		void LerpSprites(float tValue);
		void Lerp3DModels(float tValue);
		void LerpDebugLines(float tValue);
		void LerpTextCommands(float tValue);

		

		RenderCommandPool m_renderCommandPools[3]{};
		RenderCommandPool m_rendererCommandPool{};
		ApplicationFrameData m_appData{};
		uint32 m_currentActiveRenderPoolWrite = 0;
		uint32 m_currentActiveRenderPoolRead = 0;
		uint32 m_currentActiveRenderPoolLastRead = 0;
		float m_renderBlendValue = 0.0f;
		float m_engineTickRate = 0.0f;
		float m_currentRenderTimer = 0.0f;
		glm::uvec2 m_currentResolution;
		eResolutions m_renderResolution;
	};
}

