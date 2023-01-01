//Interface for the entire engine
#pragma once
#include <thread>
#include <atomic>
#include "StartupAttributes.h"

class InputHandler;
class Renderer;
class ApplicationWindow;

namespace Slask
{
	bool InitEngine(StartupAttributes startupData);
	void StartEngine();
	void ShutDownEngine();
	InputHandler& GetInputHandler();
	bool IsRunning();
	void HandleApplicationMessage(ApplicationMessage message);
	ApplicationWindow* GetApplicationWIndow();
}

