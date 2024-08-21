#include "Engine_PCH.h"
#include "HailEngine.h"

#include "Input\InputHandler.h"
#include "Input\InputActionMap.h"

#include "ApplicationWindow.h"
#include "Timer.h"
#include "Resources\ResourceManager.h"
#include "Resources\ResourceRegistry.h"
#include "Interface\ResourceInterface.h"
#include "ThreadSynchronizer.h"

#include "InternalMessageHandling\InternalMessageLogger.h"
#include "StringMemoryAllocator.h"

#include <iostream>
#include "imgui.h"
#include "ImGui\ImGuiCommands.h"

#include "angelscript.h"
#include "AngelScript\AngelScriptHandler.h"
#include "AngelScript\AngelScriptRunner.h"
#include "AngelScript\AngelScriptDebugger.h"

#ifdef PLATFORM_WINDOWS

#include "Windows/Windows_ApplicationWindow.h"
#include "Windows/Windows_InputHandler.h"
#include "Windows/Windows_Renderer.h"
#include "Hail_Time.h"
//#elif PLATFORM_OSX//.... more to be added

#endif
namespace Hail
{
	struct EngineData
	{
		Timer timer;
		InputHandler* inputHandler = nullptr;
		InputActionMap inputActionMap;
		ApplicationWindow* appWindow = nullptr;
		Renderer* renderer = nullptr;
		ResourceManager* resourceManager = nullptr;
		ResourceRegistry resourceRegistry;
		ThreadSyncronizer threadSynchronizer;
		ImGuiCommandManager imguiCommandRecorder;
		callback_function_totalTime_dt_frmData updateFunctionToCall = nullptr;
		callback_function shutdownFunctionToCall = nullptr;


		std::atomic<bool> runApplication = false;
		std::atomic<bool> pauseApplication = false;
		std::atomic<bool> runMainThread = false;
		std::atomic<bool> terminateApplication = false;

		std::atomic<bool> applicationLoopDone = false;
		std::atomic<float> gameFrameTimer = 0.0f;

		std::thread applicationThread;
		float applicationTickRate = 0;

		AngelScript::Handler asHandler;
	};

	EngineData* g_engineData = nullptr;

	void MainLoop();
	void ProcessRendering(const bool applicationThreadLocked);
	void ProcessApplicationThread();
	void Cleanup();
}



bool Hail::InitEngine(StartupAttributes startupData)
{
	SetMainThread();
	InternalMessageLogger::Initialize();
	StringMemoryAllocator::Initialize();

	g_engineData = new EngineData();
	SetGlobalTimer(&g_engineData->timer);
#ifdef PLATFORM_WINDOWS
	 
	g_engineData->appWindow = new Windows_ApplicationWindow();
	g_engineData->inputHandler = new Windows_InputHandler();
	g_engineData->renderer = new VlkRenderer();

#elif PLATFORM_OSX

#endif
	g_engineData->inputHandler->InitInputMapping();
	if(!g_engineData->appWindow->Init(startupData, g_engineData->inputHandler))
	{
		Cleanup();
		return false;
	}
	if (!g_engineData->renderer->InitDevice(&g_engineData->timer))
	{
		return false;
	}

	g_engineData->resourceRegistry.Init();

	g_engineData->resourceManager = new ResourceManager();
	g_engineData->resourceManager->SetTargetResolution(startupData.renderTargetResolution);
	g_engineData->resourceManager->SetWindowResolution(startupData.startupWindowResolution);
	if (!g_engineData->resourceManager->InitResources(g_engineData->renderer->GetRenderingDevice()))
	{
		Cleanup();
		return false;
	}
	if (!g_engineData->renderer->InitGraphicsEngine(g_engineData->resourceManager))
	{
		Cleanup();
		return false;
	}
	ResourceInterface::InitializeResourceInterface(*g_engineData->resourceManager);

	g_engineData->inputActionMap.Init(g_engineData->inputHandler);
	// AngelScriptHandler
	g_engineData->asHandler.Init(&g_engineData->inputActionMap, &g_engineData->threadSynchronizer);

	const float tickTime = 1.0f / g_engineData->applicationTickRate;
	g_engineData->threadSynchronizer.Init(tickTime);
	g_engineData->imguiCommandRecorder.Init(g_engineData->resourceManager);
	startupData.initFunctionToCall(&g_engineData->inputHandler->GetInputMapping()); // Init the calling application
	g_engineData->updateFunctionToCall = startupData.updateFunctionToCall;
	g_engineData->shutdownFunctionToCall = startupData.shutdownFunctionToCall;
	g_engineData->applicationTickRate = (float32)startupData.applicationTickRate;

	g_engineData->threadSynchronizer.SynchronizeAppData(g_engineData->inputActionMap, g_engineData->imguiCommandRecorder.FetchImguiResults(), *g_engineData->resourceManager);
	startupData.postInitFunctionToCall();

	return true;
}

void Hail::StartEngine()
{
	g_engineData->runApplication = true;
	g_engineData->runMainThread = true;
	g_engineData->applicationThread = std::thread( &ProcessApplicationThread);
	MainLoop();
}

void Hail::ShutDownEngine()
{
	g_engineData->terminateApplication = true;
}

Hail::InputHandler& Hail::GetInputHandler()
{
	return *g_engineData->inputHandler;
}

Hail::ResourceRegistry& Hail::GetResourceRegistry()
{
	return g_engineData->resourceRegistry;
}

bool Hail::IsRunning()
{
	return g_engineData->runApplication;
}

void Hail::HandleApplicationMessage(ApplicationMessage message)
{
	g_engineData->appWindow->SetApplicationSettings(message);
}

Hail::ApplicationWindow* Hail::GetApplicationWIndow()
{
	return g_engineData->appWindow;
}

void Hail::MainLoop()
{
	bool lockApplicationThread = false;
	EngineData& engineData = *g_engineData;
	while (engineData.runMainThread)
	{
		engineData.timer.FrameStart();
		// Updates window state and checks for input messages from OS
		engineData.appWindow->ApplicationUpdateLoop();
		const glm::uvec2 resolution = Hail::GetApplicationWIndow()->GetWindowResolution();

		if (resolution.x != 0.0f && resolution.y != 0.0f)
		{
			ProcessRendering(lockApplicationThread);
		}
		if(lockApplicationThread == false)
		{
			engineData.threadSynchronizer.SynchronizeRenderData(engineData.timer.GetDeltaTime());
		}
		//SwapData
		if (engineData.applicationLoopDone)
		{
			engineData.inputHandler->UpdateGamepads();
			g_engineData->inputActionMap.UpdateInputActions();
			InternalMessageLogger::GetInstance().Update();
			engineData.imguiCommandRecorder.SwitchCommandBuffers(lockApplicationThread);
			engineData.threadSynchronizer.SynchronizeAppData(engineData.inputActionMap, engineData.imguiCommandRecorder.FetchImguiResults(), *engineData.resourceManager);

			if (lockApplicationThread)
			{
				engineData.pauseApplication = true;
				engineData.runApplication = false;
				engineData.applicationThread.join();
				engineData.threadSynchronizer.SynchronizeRenderData(0.0f);
			}

			engineData.applicationLoopDone = false;
			engineData.inputHandler->UpdateKeyStates();
		}
		if (engineData.pauseApplication == false)
		{
			lockApplicationThread = false;
		}
	}
	engineData.applicationThread.join();
	engineData.renderer->WaitForGPU();
	Cleanup();
}

void Hail::ProcessRendering(const bool applicationThreadLocked)
{
	EngineData& engineData = *g_engineData;
	Hail::InputMapping& inputMapping = g_engineData->inputHandler->GetInputMapping();
	engineData.renderer->StartFrame(g_engineData->threadSynchronizer.GetRenderPool());

	if (applicationThreadLocked)
	{
		bool unlockApplicationThread = false;
		engineData.imguiCommandRecorder.RenderSingleImguiCommand(unlockApplicationThread);
		if (unlockApplicationThread)
		{
			engineData.pauseApplication = false;
			engineData.runApplication = true;
			engineData.applicationThread = std::thread(&ProcessApplicationThread);
			engineData.inputHandler->UpdateKeyStates();
		}
	}
	else
	{
		engineData.imguiCommandRecorder.RenderImguiCommands();
	}

	engineData.renderer->Render();
	engineData.renderer->EndFrame();
}

void Hail::ProcessApplicationThread()
{
	EngineData& engineData = *g_engineData;
	Timer applicationTimer;
	const float tickTime = 1.0f / engineData.applicationTickRate;
	float applicationTime = 0.0;

	AngelScript::Runner asScriptRunner;
	asScriptRunner.Initialize(g_engineData->asHandler.GetScriptEngine());

	StringL firstScriptPath = ANGELSCRIPT_DIR;
	firstScriptPath += "FirstScript.as";
	asScriptRunner.ImportAndBuildScript(firstScriptPath.Data(), "FirstScript");

	while(engineData.runApplication)
	{
		applicationTimer.FrameStart();
		applicationTime += applicationTimer.GetDeltaTime();
		if (applicationTime >= tickTime && !engineData.applicationLoopDone)
		{
			engineData.updateFunctionToCall(applicationTimer.GetTotalTime(), tickTime, engineData.threadSynchronizer.GetAppFrameData());
			asScriptRunner.RunScript("FirstScript");
			engineData.threadSynchronizer.PrepareApplicationData();
			asScriptRunner.Update();
			applicationTime = 0.0;
			engineData.applicationLoopDone = true;
		}
		if(engineData.terminateApplication)
		{
			engineData.runApplication = false;
			engineData.shutdownFunctionToCall();
		}
	}
	if (engineData.pauseApplication == false)
	{
		g_engineData->runMainThread = false;
	}

	asScriptRunner.Cleanup();
}

void Hail::Cleanup()
{
	g_engineData->imguiCommandRecorder.DeInit();
	g_engineData->renderer->Cleanup();
	g_engineData->asHandler.GetScriptEngine()->ShutDownAndRelease();
	SAFEDELETE(g_engineData->appWindow);
	SAFEDELETE(g_engineData->inputHandler);
	SAFEDELETE(g_engineData->renderer);
	SAFEDELETE(g_engineData->resourceManager);
	SAFEDELETE(g_engineData);
	InternalMessageLogger::Deinitialize();
	StringMemoryAllocator::Deinitialize();
}