#include "Engine_PCH.h"
#include "CrabEngine.h"

#include "InputHandler.h"
#include "ApplicationWindow.h"
#include "Timer.h"
#include "ShaderManager.h"

#include <iostream>

#ifdef PLATFORM_WINDOWS

#include "Windows/Windows_ApplicationWindow.h"
#include "Windows/Windows_InputHandler.h"
#include "Windows/Windows_Renderer.h"
//#elif PLATFORM_OSX//.... more to be added

#endif

struct Data
{
	Timer* timer = nullptr;
	InputHandler* inputHandler = nullptr;
	ApplicationWindow* appWindow = nullptr;
	Renderer* renderer = nullptr;
	ShaderManager* shaderManager = nullptr;

	callback_function_dt updateFunctionToCall = nullptr;
	//callback_function_dt m_renderFunctionToCall = nullptr;
	callback_function shutdownFunctionToCall = nullptr;


	std::atomic<bool> runApplication = false;
	std::atomic<bool> runMainThread = false;
	std::atomic<bool> terminateApplication = false;

	std::thread applicationThread;
};

Data* g_engineData = nullptr;

void ProcessApplication();
void ProcessRendering();
void Cleanup();
void FrameStart();
void FrameEnd();


bool Crab::InitEngine(StartupAttributes startupData)
{
	g_engineData = new Data();
	g_engineData->timer = new Timer();

#ifdef PLATFORM_WINDOWS
	 
	g_engineData->appWindow = new Windows_ApplicationWindow();
	g_engineData->inputHandler = new Windows_InputHandler();
	g_engineData->renderer = new VlkRenderer();

#elif PLATFORM_OSX


#endif
	g_engineData->shaderManager = new ShaderManager();
	g_engineData->shaderManager->LoadAllRequiredShaders();

	if(!g_engineData->appWindow->Init(startupData, g_engineData->inputHandler))
	{
		Cleanup();
		return false;
	}
	if (!g_engineData->renderer->Init(startupData.startupResolution, g_engineData->shaderManager))
	{
		Cleanup();
		return false;
	}


	startupData.initFunctionToCall(); // Init the calling application

	g_engineData->updateFunctionToCall = startupData.updateFunctionToCall;
	g_engineData->shutdownFunctionToCall = startupData.shutdownFunctionToCall;

	return true;
}



void Crab::StartEngine()
{
	g_engineData->runApplication = true;
	g_engineData->runMainThread = true;
	g_engineData->applicationThread = std::thread( &ProcessApplication );
	ProcessRendering();
}

void Crab::ShutDownEngine()
{
	g_engineData->terminateApplication = true;
}

InputHandler& Crab::GetInputHandler()
{
	return *g_engineData->inputHandler;
}

bool Crab::IsRunning()
{
	return g_engineData->runApplication;
}

void Crab::HandleApplicationMessage(ApplicationMessage message)
{

}

ApplicationWindow* Crab::GetApplicationWIndow()
{
	return g_engineData->appWindow;
}


void ProcessRendering()
{
	bool runHello = false;
	while(g_engineData->runMainThread)
	{
		g_engineData->timer->FrameStart();
		g_engineData->appWindow->ApplicationUpdateLoop();

		if (g_engineData->inputHandler->IsKeyDown('A'))
		{
			g_engineData->inputHandler->LockMouseToWindow(true);
			runHello = true;
		}

		if (runHello)
		{
			float dt = static_cast<float>(g_engineData->timer->GetDeltaTime());
			std::cout << "Hello World: " << dt << std::endl;
		}
		g_engineData->renderer->MainLoop();
	}
	g_engineData->applicationThread.join();
	Cleanup();
}

void Cleanup()
{
	g_engineData->renderer->Cleanup();
	SAFEDELETE(g_engineData->appWindow);
	SAFEDELETE(g_engineData->inputHandler);
	SAFEDELETE(g_engineData->timer);
	SAFEDELETE(g_engineData->renderer);
	SAFEDELETE(g_engineData->shaderManager);
	SAFEDELETE(g_engineData);
}

void ProcessApplication()
{
	while(g_engineData->runApplication)
	{
		const float deltaTime = 60.0f / 1.0f;
		g_engineData->updateFunctionToCall(deltaTime);

		if(g_engineData->terminateApplication)
		{
			g_engineData->runApplication = false;
			g_engineData->shutdownFunctionToCall();
		}
	}
	g_engineData->runMainThread = false;
}
