#pragma once
#include "Resources\MaterialResources.h"
#include "Windows\VulkanInternal\VlkResources.h"

namespace Hail
{
	class VlkMaterial : public Material
	{
	public:

		VlkPassData m_passData;
		ResourceValidator m_passDataValidator;
	};
}


