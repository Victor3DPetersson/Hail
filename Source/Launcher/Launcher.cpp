#include "Launcher_PCH.h"
#include "CrabEngine.h"
#include "StartUpAttributes.h"
#include "Game.h"

int main()
{

	GameApplication* game = new GameApplication();

	StartupAttributes startData;


	startData.initFunctionToCall = [game]() { game->Init(); };
	startData.shutdownFunctionToCall = [game]() { game->Shutdown(); };
	startData.updateFunctionToCall = [game](float dt) { game->Update(dt); };

	if(Crab::InitEngine(startData))
	{
		Crab::StartEngine();
	}

	return 0;
}