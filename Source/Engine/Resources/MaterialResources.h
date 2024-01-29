#pragma once
#include "glm\vec4.hpp"
#include "Resource.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"
#include "String.hpp"
#include "../EngineConstants.h"
#include "ShaderCommons.h"

namespace Hail
{
	enum class MATERIAL_TYPE : uint8
	{
		SPRITE,
		//FULLSCREEN_POSTEFFECTS,
		FULLSCREEN_PRESENT_LETTERBOX,
		MODEL3D,
		DEBUG_LINES2D,
		DEBUG_LINES3D,
		COUNT
	};
	enum class BLEND_MODE : uint8
	{
		NORMAL,
		ALPHABLEND,
		CUTOUT,
		ADDITIVE,
		COUNT
	};
	constexpr uint8 MAX_TEXTURE_HANDLES = 16;


	constexpr uint8 MATERIAL_VERSION = 1;

	struct SerializeableMaterialInstance
	{
		MATERIAL_TYPE m_baseMaterialType{};
		BLEND_MODE m_blendMode{};
		uint16 m_extraData{};
		StaticArray<GUID, MAX_TEXTURE_HANDLES> m_textureHandles;
		glm::vec4 m_instanceFloatParameters;
	};

	class MaterialInstance
	{
	public:
		GUID m_id;
		uint32 m_materialIdentifier = 0;
		uint32 m_instanceIdentifier = 0;
		uint32 m_gpuResourceInstance = 0;
		//Textures
		StaticArray<uint32, MAX_TEXTURE_HANDLES> m_textureHandles = (MAX_UINT);
		//Other instanced parameters
		glm::vec4 m_instanceFloatParameters;
	};

	class Material
	{
	public:

		CompiledShader m_vertexShader;
		CompiledShader m_fragmentShader;
		CompiledShader m_tessShader;
		CompiledShader m_controlShader;
		BLEND_MODE m_blendMode = BLEND_MODE::NORMAL;

		//Add shader reflection data here

		MATERIAL_TYPE m_type = MATERIAL_TYPE::COUNT;
	};



}

