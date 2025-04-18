#pragma once
#include "StartupAttributes.h"


namespace Hail
{
	class ApplicationWindow;
	class InputHandler;
	class Timer;

	class ResourceRegistry;
	bool InitEngine(StartupAttributes startupData);
	void StartEngine();
	void ShutDownEngine();
	//TODO: make thread safe for the getters
	InputHandler& GetInputHandler();
	ResourceRegistry& GetResourceRegistry();
	const Timer& GetRenderLoopTimer();

	bool IsRunning();
	// A getter for locking operations to see if the engine or window has been terminated from an OS command.
	bool IsApplicationTerminated();
	void HandleApplicationMessage(ApplicationMessage message);
	ApplicationWindow* GetApplicationWIndow();
}

