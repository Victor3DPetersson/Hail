#include "Types.h"

namespace Hail
{
	struct Settings
	{
		bool b_enableGpuDebugDrawing = true; // TODO make a gpu piupeline to fill debugging from Compute passes
		bool b_enableGpuParticles = true;
		bool b_enableEngineImgui = true; // TODO: hook up to be used
	};

}