#pragma once
#include "glm\vec4.hpp"
#include "Resource.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "../EngineConstants.h"
#include "ShaderCommons.h"

namespace Hail
{
	enum class MATERIAL_TYPE : uint32
	{
		SPRITE,
		//FULLSCREEN_POSTEFFECTS,
		FULLSCREEN_PRESENT_LETTERBOX,
		MODEL3D,
		COUNT
	};
	enum class BLEND_MODE : uint32
	{
		NORMAL,
		ALPHABLEND,
		CUTOUT,
		ADDITIVE,
		COUNT
	};

	class MaterialInstance
	{
	public:
		uint32 m_materialIdentifier = 0;
		uint32 m_instanceIdentifier = 0;
		uint32 m_gpuResourceInstance = 0;
		//Textures
		uint32 m_textureHandles[8]{};
		//Other instanced parameters
		glm::vec4 m_instanceFloatParameters;
	};

	class Material
	{
	public:
		GUID m_uuid;

		CompiledShader m_vertexShader;
		CompiledShader m_fragmentShader;
		CompiledShader m_tessShader;
		CompiledShader m_controlShader;
		BLEND_MODE m_blendMode = BLEND_MODE::NORMAL;

		//Add shader reflection data here

		MATERIAL_TYPE m_type = MATERIAL_TYPE::COUNT;
	};



}

