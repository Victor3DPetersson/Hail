//Interface for the entire engine
#pragma once
#include <atomic>
#include "StartupAttributes.h"
#include "RenderCommands.h"
#include "Interface\GameCommands.h"
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
		ApplicationCommandPool* commandPoolToFill;
		ImGuiCommandRecorder* imguiCommandRecorder;
	};

	class ThreadSyncronizer
	{
	public:
		ThreadSyncronizer() = default;
		void Init(float tickTimer);
		// Swap buffers and prepares the app data for a new frame
		void SynchronizeAppData(InputActionMap& inputActionMap, ImGuiCommandRecorder& imguiCommandRecorder, ResourceManager& resourceManager);
		// Moves over and sorts data from the game commands in to render commands, as well as batches the data
		void TransferGameCommandsToRenderCommands(ResourceManager& resourceManager);
		void SynchronizeRenderData(float frameDeltaTime);
		// Moves sprites and the like to the correct position with the camera. Called on the application thread. 
		void PrepareApplicationData();
		ApplicationFrameData& GetAppFrameData() { return m_appData; }
		RenderCommandPool& GetRenderPool() { return m_renderCommandPools[m_currentActiveRenderPoolWrite]; }
	private:
		void SwapBuffersInternal();
		void LerpRenderBuffers();
		void Lerp3DModels(float tValue);
		void LerpDebugLines(float tValue);

		RenderCommandPool m_renderCommandPools[3]{};
		// DoubleBuffered for application as that does not need any other update frequency
		ApplicationCommandPool m_appCommandPools[2];

		ApplicationFrameData m_appData{};
		uint32 m_currentActiveRenderPoolWrite = 0; // Write is the pool that we are blending towards in the frame
		uint32 m_currentActiveRenderPoolRead = 0;
		uint32 m_currentActiveRenderPoolLastRead = 0;
		uint32 m_currentActiveAppCommandPoolWrite = 0;
		uint32 m_currentActiveAppCommandPoolRead = 0;
		float m_renderBlendValue = 0.0f;
		float m_engineTickRate = 0.0f;
		float m_currentRenderTimer = 0.0f;
		glm::uvec2 m_currentResolution;
		eResolutions m_renderResolution;
	};
}

