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
	enum class eMaterialType : uint8
	{
		SPRITE,
		FULLSCREEN_PRESENT_LETTERBOX, 
		MODEL3D,
		DEBUG_LINES2D,
		DEBUG_LINES3D,
		COUNT
	};
	enum class eBlendMode : uint8
	{
		None,
		Cutout,
		Translucent,
		Additive,
		COUNT
	};
	constexpr uint8 MAX_TEXTURE_HANDLES = 16;


	constexpr uint8 MATERIAL_VERSION = 1;

	struct SerializeableMaterialInstance
	{
		eMaterialType m_baseMaterialType{};
		eBlendMode m_blendMode{};
		uint16 m_extraData{};
		StaticArray<GUID, MAX_TEXTURE_HANDLES> m_textureHandles;
		glm::vec4 m_instanceFloatParameters;
	};

	class MaterialInstance
	{
	public:
		GUID m_id;
		uint32 m_materialIndex = 0;
		uint32 m_instanceIdentifier = 0;
		uint32 m_gpuResourceInstance = 0;
		eMaterialType m_materialType;
		eBlendMode m_blendMode = eBlendMode::None;
		uint8 m_cutoutThreshold = 0;
		//Textures
		StaticArray<uint32, MAX_TEXTURE_HANDLES> m_textureHandles = (MAX_UINT);
		//Other instanced parameters
		glm::vec4 m_instanceFloatParameters;
	};

	// TODO: Add unique shader hashes
	uint64 GetMaterialSortValue(eMaterialType type, eBlendMode blend, uint32 shaderValues);

	class Material
	{
	public:

		CompiledShader m_vertexShader;
		CompiledShader m_fragmentShader;
		CompiledShader m_tessShader;
		CompiledShader m_controlShader;

		eBlendMode m_blendMode = eBlendMode::None;
		eMaterialType m_type = eMaterialType::COUNT;
		uint64 m_sortKey = MAX_UINT;
		ResourceValidator m_validator;

	};



}

