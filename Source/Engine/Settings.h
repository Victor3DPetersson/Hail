#include "Types.h"

namespace Hail
{
	struct Settings
	{
		bool b_enableGpuDebugDrawing = true; // TODO make a gpu piupeline to fill debugging from Compute passes
		bool b_enableGpuParticles = true;
#ifndef NDEBUG
		bool b_enableEngineImgui = true;
#else
		// TODO: hook up to be connected to commandline arguments
		bool b_enableEngineImgui = false;
#endif
	};

}