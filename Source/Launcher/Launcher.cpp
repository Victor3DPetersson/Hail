#include "Launcher_PCH.h"
#include "HailEngine.h"
#include "StartUpAttributes.h"
#include "Game.h"

#include <Windows.h>

#include <string>
#include <stringapiset.h>

// Comment out below define to disable command line
#ifndef NDEBUG
#define USE_CONSOLE_COMMAND
#endif

void InitConsole()
{
#pragma warning( push )
#pragma warning( disable : 4996 )
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
#pragma warning( pop )
}

void CloseConsole()
{
#pragma warning( push )
#pragma warning( disable : 4996 )
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
#pragma warning( pop )
}

namespace Hail
{
	struct ApplicationFrameData;
}


int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, char*, int /*nShowCmd*/)
{
#ifdef USE_CONSOLE_COMMAND
	InitConsole();
#endif

	Hail::GameApplication* game = new Hail::GameApplication();

	Hail::StartupAttributes startData;
	startData.initFunctionToCall = [game](void* initData) { game->Init(initData); };
	startData.postInitFunctionToCall = [game]() { game->PostInit(); };
	startData.shutdownFunctionToCall = [game]() { game->Shutdown(); };
	startData.updateFunctionToCall = [game](double totalTime, float dt, Hail::ApplicationFrameData& frameData) { game->Update(totalTime, dt, frameData); };

	if (Hail::InitEngine(startData))
	{
		Hail::StartEngine();
	}

#ifdef USE_CONSOLE_COMMAND
	CloseConsole();
#endif
	return 0;
}



