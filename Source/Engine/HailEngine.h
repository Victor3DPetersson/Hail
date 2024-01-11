//Interface for the entire engine
#pragma once
#include <thread>
#include <atomic>
#include "StartupAttributes.h"

class InputHandler;
class ApplicationWindow;

namespace Hail
{
	class ResourceRegistry;
	bool InitEngine(StartupAttributes startupData);
	void StartEngine();
	void ShutDownEngine();
	InputHandler& GetInputHandler();
	ResourceRegistry& GetResourceRegistry();
	bool IsRunning();
	void HandleApplicationMessage(ApplicationMessage message);
	ApplicationWindow* GetApplicationWIndow();
}

