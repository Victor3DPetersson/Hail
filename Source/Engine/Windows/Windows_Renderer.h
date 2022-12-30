//Interface for the entire engine
#pragma once

#include "Renderer.h"


class VlkRenderer : public Renderer
{
public:

	bool Init(RESOLUTIONS startupResolution) override;
	void MainLoop() override;
	void Cleanup() override;

};