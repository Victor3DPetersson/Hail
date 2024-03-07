//Interface for the entire engine
#pragma once
#include <thread>
#include <atomic>
#include "StartupAttributes.h"


namespace Hail
{
	class InputHandler;
	class ApplicationWindow;

	class ResourceRegistry;
	bool InitEngine(StartupAttributes startupData);
	void StartEngine();
	void ShutDownEngine();
	//TODO: make thread safe for the getters
	InputHandler& GetInputHandler();
	ResourceRegistry& GetResourceRegistry();

	bool IsRunning();
	void HandleApplicationMessage(ApplicationMessage message);
	ApplicationWindow* GetApplicationWIndow();
}

