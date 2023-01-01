//Interface for the entire engine
#pragma once
#include <thread>
#include <atomic>

#include "StartupAttributes.h"

#include "glm\vec2.hpp"


class Renderer
{
public:

	virtual bool Init(RESOLUTIONS startupResolution) = 0;
	virtual void MainLoop() = 0;
	virtual void Cleanup() = 0;

protected:

	glm::uvec2 m_renderResolution;

};