//Interface for the entire engine
#pragma once
#include "StartupAttributes.h"

class GameApplication
{
public:
	void Init();
	void Update(float deltaTime);
	void Shutdown();

private:

	void SendFrameData();

};

