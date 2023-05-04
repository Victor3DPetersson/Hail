#pragma once
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "ShaderCommons.h"
#include "MaterialResources.h"


namespace Hail
{
	class ShaderCompiler;
	class MaterialManager
	{
	public:

		void Update();
		bool LoadMaterial(MATERIAL_TYPE  type);
		bool LoadMaterial(GUID uuid);
		Material& GetMaterial(MATERIAL_TYPE materialType);
		const MaterialInstance& GetMaterialInstance(MATERIAL_TYPE materialType, uint32_t instanceID);
		MaterialInstance& CreateInstance(MATERIAL_TYPE materialType);
	protected:

		CompiledShader LoadShader(const char* shaderName, SHADERTYPE shaderType);
		void InitCompiler();
		void DeInitCompiler();
		void Cleanup();

		ShaderCompiler* m_compiler = nullptr;
		GrowingArray<CompiledShader> m_compiledRequiredShaders;
		Material m_materials[static_cast<uint32_t>(MATERIAL_TYPE::COUNT)];
		GrowingArray< MaterialInstance> m_materialsInstanceData[static_cast<uint32_t>(MATERIAL_TYPE::COUNT)];
	};
}
