#pragma once
#include "glm\vec4.hpp"
#include "ResourceCommon.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"
#include "String.hpp"
#include "../EngineConstants.h"
#include "Resources_Materials/ShaderCommons.h"
#include "Resources_Materials/Materials_Common.h"
#include "ReflectedShaderData.h"

namespace Hail
{
	class RenderingDevice;

	constexpr uint8 MAX_TEXTURE_HANDLES = 8;

	constexpr uint8 MATERIAL_VERSION = 1;

	struct SerializeableMaterial
	{
		eMaterialType m_baseMaterialType{};
		eBlendMode m_blendMode{};
		uint16 m_extraData{};
		StaticArray<GUID, MAX_TEXTURE_HANDLES> m_textureHandles;
		glm::vec4 m_instanceFloatParameters;
		struct ShaderType
		{
			eShaderType m_type = eShaderType::None;
			GUID m_id = GuidZero;
		};
		ShaderType m_shaders[2];
	};

	// TODO: Add support for shader names in material instances
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
	uint64 GetMaterialSortValue(eMaterialType type, eBlendMode blend, uint64 shaderValues);

	// Owns common data for a specific material type, so common textures and descriptors Set 1 and 0 data
	class MaterialTypeDescriptor
	{
	public:
		virtual void CleanupResource(RenderingDevice& device) = 0;
		eMaterialType m_type = eMaterialType::COUNT;
		ResourceValidator m_typeDataValidator;
		VectorOnStack<ReflectedShaderData, 2> m_expectedShaderData;
	};
	
	// Descriptors unique to one material, Set 2 data
	class MaterialInstanceDescriptor
	{
	public:
		virtual void CleanupResource(RenderingDevice& device) = 0;
		virtual void CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight) = 0;
		ResourceValidator m_instanceDataValidator;
	};

	// The material owns the shader, has a pointer to the shared type data and owns the instance descriptors
	class Material
	{
	public:

		virtual void CleanupResource(RenderingDevice& device) = 0;
		virtual void CleanupResourceFrameData(RenderingDevice& device, uint32 frameInFlight) = 0;

		VectorOnStack<CompiledShader*, 2> m_pShaders;

		eBlendMode m_blendMode = eBlendMode::None;
		eMaterialType m_type = eMaterialType::COUNT;
		uint64 m_sortKey = MAX_UINT;
		ResourceValidator m_validator;

		GrowingArray<MaterialInstanceDescriptor*> m_pInstanceDescriptors;
		MaterialTypeDescriptor* m_pTypeDescriptor = nullptr;
	};




}

