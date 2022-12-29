//Interface for the entire engine
#pragma once
#include <thread>
#include <atomic>
#include "StartupAttributes.h"

class InputHandler;
class Renderer;


namespace Slask
{
	bool InitEngine(StartupAttributes startupData);
	void StartEngine();
	void ShutDownEngine();
	InputHandler& GetInputHandler();
	bool IsRunning();

}

