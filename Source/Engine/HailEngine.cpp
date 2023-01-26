#include "Engine_PCH.h"
#include "HailEngine.h"

#include "InputHandler.h"
#include "ApplicationWindow.h"
#include "Timer.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "Resources\ResourceManager.h"

#include <iostream>
#include "imgui.h"

#ifdef PLATFORM_WINDOWS

#include "Windows/Windows_ApplicationWindow.h"
#include "Windows/Windows_InputHandler.h"
#include "Windows/Windows_Renderer.h"
//#elif PLATFORM_OSX//.... more to be added

#endif
namespace Hail
{
	struct Data
	{
		Timer* timer = nullptr;
		InputHandler* inputHandler = nullptr;
		ApplicationWindow* appWindow = nullptr;
		Renderer* renderer = nullptr;
		ShaderManager* shaderManager = nullptr;
		TextureManager* textureManager = nullptr;
		ResourceManager* resourceManager = nullptr;
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
}



bool Hail::InitEngine(StartupAttributes startupData)
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
	if (!g_engineData->shaderManager->LoadAllRequiredShaders())
	{
		Cleanup();
		return false;
	}
	g_engineData->textureManager = new TextureManager();
	if (!g_engineData->textureManager->LoadAllRequiredTextures())
	{
		Cleanup();
		return false;
	}
	g_engineData->resourceManager = new ResourceManager();

	if(!g_engineData->appWindow->Init(startupData, g_engineData->inputHandler))
	{
		Cleanup();
		return false;
	}
	if (!g_engineData->renderer->Init(startupData.startupResolution, g_engineData->shaderManager, g_engineData->textureManager, g_engineData->resourceManager, g_engineData->timer))
	{
		Cleanup();
		return false;
	}

	startupData.initFunctionToCall(); // Init the calling application
	g_engineData->updateFunctionToCall = startupData.updateFunctionToCall;
	g_engineData->shutdownFunctionToCall = startupData.shutdownFunctionToCall;
	return true;
}



void Hail::StartEngine()
{
	g_engineData->runApplication = true;
	g_engineData->runMainThread = true;
	g_engineData->applicationThread = std::thread( &ProcessApplication );
	ProcessRendering();
}

void Hail::ShutDownEngine()
{
	g_engineData->terminateApplication = true;
}

InputHandler& Hail::GetInputHandler()
{
	return *g_engineData->inputHandler;
}

bool Hail::IsRunning()
{
	return g_engineData->runApplication;
}

void Hail::HandleApplicationMessage(ApplicationMessage message)
{
	g_engineData->appWindow->SetApplicationSettings(message);
}

ApplicationWindow* Hail::GetApplicationWIndow()
{
	return g_engineData->appWindow;
}


void Hail::ProcessRendering()
{
	bool runHello = false;
	while(g_engineData->runMainThread)
	{
		g_engineData->timer->FrameStart();
		g_engineData->appWindow->ApplicationUpdateLoop();
		g_engineData->renderer->StartFrame();

		if (g_engineData->inputHandler->IsKeyDown('A'))
		{
			Hail::ApplicationMessage message;
			message.command = Hail::TOGGLE_FULLSCREEN;
			g_engineData->appWindow->SetApplicationSettings(message);
			//runHello = true;
		}

		if (runHello)
		{
			float dt = static_cast<float>(g_engineData->timer->GetDeltaTime());
			std::cout << "Hello World: " << dt << std::endl;
		}
		ImGui::Begin("Window");
		//ImGuiCommands here
		float val = 0;
		ImGui::DragFloat("Hi", &val);

		ImGui::End();
		g_engineData->renderer->Render();

		g_engineData->renderer->EndFrame();
	}
	g_engineData->applicationThread.join();
	Cleanup();
}

void Hail::Cleanup()
{
	g_engineData->renderer->Cleanup();
	SAFEDELETE(g_engineData->appWindow);
	SAFEDELETE(g_engineData->inputHandler);
	SAFEDELETE(g_engineData->timer);
	SAFEDELETE(g_engineData->renderer);
	SAFEDELETE(g_engineData->shaderManager);
	SAFEDELETE(g_engineData->textureManager);
	SAFEDELETE(g_engineData->resourceManager);
	SAFEDELETE(g_engineData);
}

void Hail::ProcessApplication()
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
