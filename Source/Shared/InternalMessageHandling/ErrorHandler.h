#pragma once
#include "String.hpp"

#include "Containers/StaticArray/StaticArray.h"

namespace Hail
{
	enum class EErrorType
	{
		Startup,
		Count
	};
	enum EStartupErrors
	{
		InitResourceRegistry = 1,
		InitDebugRenderer = 2,
		InitCloudRenderer = 4,
		InitFontRenderer = 8,
		InitTextureManager = 16,
		InitMaterialManager = 32,
		InitResourceManager = 64,
		InitDefaultMaterial = 128,
		InitRenderingResourceManager = 256,
		InitSwapChain = 512,
		InitGpuDevice = 1024,
	};

	class ErrorManager
	{
	public:
		ErrorManager();
		~ErrorManager() = default;

		void AddErrors(uint64 errors, EErrorType errorType);

		void DumpErrorLog() const;
		void AddString(StringL error);
		bool GetAreErrorsLogged() const;

	private:
		StaticArray<uint64, (uint32)EErrorType::Count> m_errors;
		StringL m_errorString;
	};
}