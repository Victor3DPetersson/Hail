#pragma once
#include "Resources\MaterialResources.h"


namespace Hail
{

	// These tasks can be reordered in what order they are drawn if they are enabled
	enum class eRenderTask
	{
		Sprite,
		Model3D,
		DebugLine2D,
		DebugLine3D
		
	};

	struct RenderDependency
	{

	};

	struct RenderTask
	{
		eRenderTask mainTask;

	};


	class RenderDefenition
	{
	public:



	private:

	};


}