#include "Launcher_PCH.h"
#include "HailEngine.h"
#include "StartUpAttributes.h"
#include "Game.h"

namespace Hail
{
	struct ApplicationFrameData;
}

int main()
{

	Hail::GameApplication* game = new Hail::GameApplication();

	StartupAttributes startData;


	startData.initFunctionToCall = [game](void* initData) { game->Init(initData); };
	startData.postInitFunctionToCall = [game]() { game->PostInit(); };
	startData.shutdownFunctionToCall = [game]() { game->Shutdown(); };
	startData.updateFunctionToCall = [game](double totalTime, float dt, Hail::ApplicationFrameData& frameData) { game->Update(totalTime, dt, frameData); };

	if(Hail::InitEngine(startData))
	{
		Hail::StartEngine();
	}

	return 0;
}