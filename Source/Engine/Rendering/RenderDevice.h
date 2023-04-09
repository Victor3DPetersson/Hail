#pragma once

namespace Hail
{
	class RenderingDevice
	{
	public:
		virtual bool CreateInstance() = 0;
		virtual void DestroyDevice() = 0;
	private:

	};
}

