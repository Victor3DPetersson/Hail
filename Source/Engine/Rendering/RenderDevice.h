#pragma once

namespace Hail
{
	class ErrorManager;
	struct RenderingDeviceLimits
	{
		uint32 m_maxComputeWorkGroupInvocations = 0u;
		uint32 m_maxComputeSharedMemorySize = 0u;
	};
	class RenderingDevice
	{
	public:
		virtual void CreateInstance(ErrorManager* pErrorManager) = 0;
		virtual void DestroyDevice() = 0;

		const RenderingDeviceLimits& GetDeviceLimits() { return m_deviceLimits; }
	protected:
		RenderingDeviceLimits m_deviceLimits;
	};
}

