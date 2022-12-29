#include "Engine_PCH.h"
#include "SlaskEngine.h"

#include "InputHandler.h"
#include "ApplicationWindow.h"
#include "Timer.h"

#include <iostream>

#ifdef PLATFORM_WINDOWS

#include "Windows/Windows_ApplicationWindow.h"
#include "Windows/Windows_InputHandler.h"

//#elif PLATFORM_OSX//.... more to be added

#endif





struct Data
{
	Timer* timer = nullptr;
	InputHandler* inputHandler = nullptr;
	ApplicationWindow* appWindow = nullptr;
	Renderer* renderer = nullptr;


	callback_function_dt updateFunctionToCall = nullptr;
	//callback_function_dt m_renderFunctionToCall = nullptr;
	callback_function shutdownFunctionToCall = nullptr;


	std::atomic<bool> runApplication = false;
	std::atomic<bool> terminateApplication = false;

	std::thread applicationThread;
};

Data* g_engineData = nullptr;

void ProcessApplication();
void ProcessRendering();
void FrameStart();
void FrameEnd();


bool Slask::InitEngine(StartupAttributes startupData)
{
	g_engineData = new Data();
	g_engineData->timer = new Timer();

#ifdef PLATFORM_WINDOWS
	 
	g_engineData->appWindow = new Windows_ApplicationWindow();
	g_engineData->inputHandler = new Windows_InputHandler();

#elif PLATFORM_OSX


#endif

	if(!g_engineData->appWindow->Init(startupData, g_engineData->inputHandler))
	{
		return false;
	}
	/*
	Create Renderer
	Init Renderer
	*/

	startupData.initFunctionToCall(); // Init the calling application

	g_engineData->updateFunctionToCall = startupData.updateFunctionToCall;
	g_engineData->shutdownFunctionToCall = startupData.shutdownFunctionToCall;

	return true;
}



void Slask::StartEngine()
{
	g_engineData->runApplication = true;
	g_engineData->applicationThread = std::thread( &ProcessApplication );
	ProcessRendering();
}

void Slask::ShutDownEngine()
{
	g_engineData->terminateApplication = true;
}

InputHandler& Slask::GetInputHandler()
{
	return *g_engineData->inputHandler;
}

bool Slask::IsRunning()
{
	return g_engineData->runApplication;
}


void ProcessRendering()
{
	bool runHello = false;
	while(g_engineData->runApplication)
	{
		g_engineData->timer->FrameStart();
		g_engineData->appWindow->ApplicationUpdateLoop();

		if (g_engineData->inputHandler->IsKeyDown('A'))
		{
			runHello = true;
		}

		if (runHello)
		{
			float dt = static_cast<float>(g_engineData->timer->GetDeltaTime());
			std::cout << "Hello World: " << dt << std::endl;
		}
	}

	delete g_engineData->appWindow;
	delete g_engineData->inputHandler;
	delete g_engineData->timer;
	delete g_engineData;
}

void ProcessApplication()
{
	while(g_engineData->runApplication)
	{
		const float deltaTime = 60.0f / 1.0f;
		g_engineData->updateFunctionToCall(deltaTime);

		if(g_engineData->terminateApplication)
		{
			g_engineData->shutdownFunctionToCall();
			g_engineData->runApplication = false;
		}
	}
	g_engineData->applicationThread.join();
}
