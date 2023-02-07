#include "Launcher_PCH.h"
#include "HailEngine.h"
#include "StartUpAttributes.h"
#include "Game.h"

int main()
{

	GameApplication* game = new GameApplication();

	StartupAttributes startData;


	startData.initFunctionToCall = [game](void* initData) { game->Init(initData); };
	startData.shutdownFunctionToCall = [game]() { game->Shutdown(); };
	startData.updateFunctionToCall = [game](float dt, void* frameData) { game->Update(dt, frameData); };

	if(Hail::InitEngine(startData))
	{
		Hail::StartEngine();
	}

	return 0;
}