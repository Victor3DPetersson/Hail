#pragma once
#include "Types.h"

namespace Hail
{
	class RenderSettings
	{
	public:
		bool m_bPausedSimulation = false; // All GPU simulation and the frame interpolation will come to a halt
		bool m_bEnableGpuDebugDrawing = true; // TODO make a gpu pipeline to fill debugging from Compute passes
		bool m_bEnableGpuParticles = true;
#ifndef NDEBUG
		bool m_bEnableImgui = true;
#else
		// TODO: hook up to be connected to commandline arguments
		bool m_bEnableImgui = false;
#endif
	};

	class ApplicationSettings
	{
	public:
#ifdef DEBUG
		bool m_bAngelscriptDebuggingEnabled = true; // Only set on Init, if disabled no debugger or Server will be ran for VSC extension or diagnostics
#else
		// TODO: hook up to be connected to commandline arguments
		bool m_bAngelscriptDebuggingEnabled = false;
#endif
	};
}