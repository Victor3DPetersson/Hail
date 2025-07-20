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

	struct ShaderProperties
	{
		eShaderStage m_type = eShaderStage::None;
		GUID m_id = GuidZero;
	};

	struct MaterialCreationProperties
	{
		eMaterialType m_baseMaterialType{}; // Which type descriptor domain the pipeline will be using
		eMaterialType m_typeRenderPass{}; // Which renderpass the popeline will be using
		eBlendMode m_blendMode{};
		bool m_bIsWireFrame = false;
		// Add depth state and the like

		ShaderProperties m_shaders[2];
		bool m_bUsesMaterialTypeData{};
	};

	struct SerializeableMaterial
	{
		eMaterialType m_baseMaterialType{};
		eBlendMode m_blendMode{};
		uint16 m_extraData{};
		StaticArray<GUID, MAX_TEXTURE_HANDLES> m_textureHandles;
		glm::vec4 m_instanceFloatParameters;
		ShaderProperties m_shaders[2];
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
	// If the pipeline is custom this is not a reusable object for other pipelines
	class MaterialTypeObject
	{
	public:
		virtual void CleanupResource(RenderingDevice& device) = 0;
		eMaterialType m_type = eMaterialType::COUNT;
		ResourceValidator m_typeDataValidator;
		VectorOnStack<ReflectedShaderData, 2> m_expectedShaderData;

		class BindingInformation
		{
		public:
			BindingInformation();
			void ClearBindings();
			StaticArray<uint32, MaxShaderBindingCount>& GetDecorationBoundList(uint32 frameInFlight, eDecorationType decoration);

		private:
			StaticArray<StaticArray<StaticArray<uint32, MaxShaderBindingCount>, (uint32)eDecorationType::Count>, MAX_FRAMESINFLIGHT> m_boundResourceLists;
		};
		// There exists one set for each shader
		GrowingArray<BindingInformation> m_boundResources;
		StaticArray<bool, MAX_FRAMESINFLIGHT> m_bBoundTypeData = false;
	};
	
	// Descriptors unique to one material, Set 2 data, binds textures and buffers to the shaders
	class MaterialInstanceDescriptor
	{
	public:
		ResourceValidator m_instanceDataValidator;
	};

	// The pipeline is the shader and the combination of a shader to specific blend modes
	class Pipeline
	{
	public:

		virtual void CleanupResource(RenderingDevice& device) = 0;
		eMaterialType m_type = eMaterialType::COUNT;
		VectorOnStack<CompiledShader*, 2> m_pShaders;
		eBlendMode m_blendMode = eBlendMode::None;
		bool m_bIsWireFrame = false;
		bool m_bUsesVertexBuffer = false;
		uint64 m_sortKey = MAX_UINT;
		bool m_bUseTypePasses = false;
		eMaterialType m_typeRenderPass = eMaterialType::COUNT;
		bool m_bIsCompute = false;
		MaterialTypeObject* m_pTypeObject = nullptr;
		VectorOnStack<ShaderDecoration, 4> m_pushConstants;
		uint32 m_shaderStages = 1 << (uint8)eShaderStage::None;
	};

	// The material owns the shader, has a pointer to the shared type data and owns the instance descriptors
	class Material
	{
	public:

		virtual void CleanupResource(RenderingDevice& device) = 0;

		ResourceValidator m_validator;

		GrowingArray<MaterialInstanceDescriptor*> m_pInstanceDescriptors;
		Pipeline* m_pPipeline = nullptr;
	};

	// A full pipeline state object that owns its own type descriptor. Can not be instanced. 
	class MaterialPipeline
	{
	public:

		virtual void CleanupResource(RenderingDevice& device) = 0;
		ResourceValidator m_validator;
		Pipeline* m_pPipeline = nullptr;
	};


}

