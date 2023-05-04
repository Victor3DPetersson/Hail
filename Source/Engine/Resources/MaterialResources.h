#pragma once
#include "Vertices.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "../EngineConstants.h"
#include "ShaderCommons.h"

namespace Hail
{
	enum class MATERIAL_TYPE : uint32_t
	{
		SPRITE,
		//FULLSCREEN_POSTEFFECTS,
		FULLSCREEN_PRESENT_LETTERBOX,
		MODEL3D,
		COUNT
	};
	enum class BLEND_MODE : uint32_t
	{
		NORMAL,
		ALPHABLEND,
		CUTOUT,
		ADDITIVE
	};

	class MaterialInstance
	{
	public:
		//Textures
		uint32_t m_materialIdentifier = 0;
		uint32_t m_instanceIdentifier = 0;
		uint32_t m_textureHandles[8]{};
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

		MATERIAL_TYPE m_type;
	};



}

