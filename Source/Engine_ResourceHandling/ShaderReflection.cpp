#include "ResourceCompiler_PCH.h"

#include "ShaderCompiler.h"


#include "ShaderReflection.h"
#include "spirv_cross\spirv_reflect.hpp"
#include "Resources_Materials\Materials_Common.h"
#include "DebugMacros.h"

using namespace Hail;

const char* LocalGetTypeFromSpirVDataType(spirv_cross::SPIRType::BaseType type, uint32 length)
{
	switch (type)
	{
	case spirv_cross::SPIRType::Unknown:
		//TODO: error for unknown
		return "unknown";
	case spirv_cross::SPIRType::Void:
		//TODO: error if length is not 1
		return "void";
	case spirv_cross::SPIRType::Boolean:
		//TODO: error if length is not 1 < - > 4
		return length == 1 ? "bool" : String64::Format("bool%u", length).Data();
	case spirv_cross::SPIRType::SByte:
		//TODO: error if length is not 1
		return "sbyte";
	case spirv_cross::SPIRType::UByte:
		//TODO: error if length is not 1
		return "ubyte";
	case spirv_cross::SPIRType::Short:
		//TODO: error if length is not 1
		return "short";
	case spirv_cross::SPIRType::UShort:
		//TODO: error if length is not 1
		return "ushort";
	case spirv_cross::SPIRType::Int:
		//TODO: error if length is not 1 < - > 4
		return length == 1 ? "int" : String64::Format("int%u", length).Data();
	case spirv_cross::SPIRType::UInt:
		//TODO: error if length is not 1 < - > 4
		return length == 1 ? "uint" : String64::Format("uint%u", length).Data();
	case spirv_cross::SPIRType::Int64:
		//TODO: error if length is not 1 < - > 4
		return length == 1 ? "int64" : String64::Format("int64%u", length).Data();
	case spirv_cross::SPIRType::UInt64:
		//TODO: error if length is not 1 < - > 4
		return length == 1 ? "uint64" : String64::Format("uint64%u", length).Data();
	case spirv_cross::SPIRType::AtomicCounter:
		//TODO: error if length is not 1
		return "atomic counter";
	case spirv_cross::SPIRType::Half:
		//TODO: check matrix support as well
		return length == 1 ? "half" : String64::Format("half%u", length).Data();
	case spirv_cross::SPIRType::Float:
		//TODO: check matrix support as well
		return length == 1 ? "float" : String64::Format("float%u", length).Data();
	case spirv_cross::SPIRType::Double:
		//TODO: check matrix support as well
		return length == 1 ? "double" : String64::Format("double%u", length).Data();
	case spirv_cross::SPIRType::Struct:
		//TODO: error if length is not 1
		return "struct";
	case spirv_cross::SPIRType::Image:
		//TODO: error if length is not 1
		return "image";
	case spirv_cross::SPIRType::SampledImage:
		//TODO: error if length is not 1
		return "sampled image";
	case spirv_cross::SPIRType::Sampler:
		//TODO: error if length is not 1
		return "sampler";
	case spirv_cross::SPIRType::AccelerationStructure:
		//TODO: error if length is not 1
		return "acceleration sctructure";
	case spirv_cross::SPIRType::RayQuery:
		//TODO: error if length is not 1
		return "ray query";
	case spirv_cross::SPIRType::ControlPointArray:
		//TODO: error if length is not 1
		return "control point array";
	case spirv_cross::SPIRType::Interpolant:
		//TODO: error if length is not 1
		return "interpolant";
	case spirv_cross::SPIRType::Char:
		//TODO: error if length is not 1
		return "char";
	default:
		return "";
	}
}

eShaderValueType LocalGetValueTypeFromSpirVType(spirv_cross::SPIRType::BaseType type)
{
	switch (type)
	{
	case spirv_cross::SPIRType::Unknown:
		return eShaderValueType::none;

	case spirv_cross::SPIRType::Void:
		return eShaderValueType::none;

	case spirv_cross::SPIRType::Boolean:
		return eShaderValueType::boolean;

	case spirv_cross::SPIRType::SByte:
		return eShaderValueType::int8;

	case spirv_cross::SPIRType::UByte:
		return eShaderValueType::uint8;

	case spirv_cross::SPIRType::Short:
		return eShaderValueType::int16;

	case spirv_cross::SPIRType::UShort:
		return eShaderValueType::uint16;

	case spirv_cross::SPIRType::Int:
		return eShaderValueType::int32;

	case spirv_cross::SPIRType::UInt:
		return eShaderValueType::uint32;

	case spirv_cross::SPIRType::Int64:
		return eShaderValueType::int64;

	case spirv_cross::SPIRType::UInt64:
		return eShaderValueType::uint64;

	case spirv_cross::SPIRType::AtomicCounter:
		return eShaderValueType::none;

	case spirv_cross::SPIRType::Half:
		return eShaderValueType::float16;

	case spirv_cross::SPIRType::Float:
		return eShaderValueType::float32;

	case spirv_cross::SPIRType::Double:
		return eShaderValueType::float64;

	case spirv_cross::SPIRType::Struct:
	case spirv_cross::SPIRType::Image:
	case spirv_cross::SPIRType::SampledImage:
	case spirv_cross::SPIRType::Sampler:
	case spirv_cross::SPIRType::AccelerationStructure:
	case spirv_cross::SPIRType::RayQuery:
	case spirv_cross::SPIRType::ControlPointArray:
	case spirv_cross::SPIRType::Interpolant:
	case spirv_cross::SPIRType::Char:
	default:
		//TODO: error if length is not 1
		return eShaderValueType::none;
	}
}

uint32 GetTypeByteSize(spirv_cross::SPIRType::BaseType type)
{
	switch (type)
	{
	case spirv_cross::SPIRType::SByte:
	case spirv_cross::SPIRType::UByte:
		return 1;

	case spirv_cross::SPIRType::Short:
	case spirv_cross::SPIRType::UShort:
	case spirv_cross::SPIRType::Half:
		return 2;

	case spirv_cross::SPIRType::Boolean:
	case spirv_cross::SPIRType::Int:
	case spirv_cross::SPIRType::UInt:
	case spirv_cross::SPIRType::Float:
		return 4;

	case spirv_cross::SPIRType::Int64:
	case spirv_cross::SPIRType::UInt64:
	case spirv_cross::SPIRType::Double:
		return 8;

	case spirv_cross::SPIRType::Unknown:
	case spirv_cross::SPIRType::Void:
	case spirv_cross::SPIRType::AtomicCounter:
	case spirv_cross::SPIRType::Struct:
	case spirv_cross::SPIRType::Image:
	case spirv_cross::SPIRType::SampledImage:
	case spirv_cross::SPIRType::Sampler:
	case spirv_cross::SPIRType::AccelerationStructure:
	case spirv_cross::SPIRType::RayQuery:
	case spirv_cross::SPIRType::ControlPointArray:
	case spirv_cross::SPIRType::Interpolant:
	case spirv_cross::SPIRType::Char:
	default:
		//TODO: error if length is not 1
		return 0;
	}
}

uint32 LocalGetStructByteSize(spirv_cross::CompilerReflection& compiler, const spirv_cross::SPIRType& typeToCheck)
{
	uint32 currentByteSize = 0;
	if (typeToCheck.basetype == spirv_cross::SPIRType::Struct)
	{
		for (uint32 i = 0; i < typeToCheck.member_types.size(); i++)
		{
			const spirv_cross::SPIRType& structMemberType = compiler.get_type(typeToCheck.member_types[i]);

			if (typeToCheck.basetype == spirv_cross::SPIRType::Struct)
			{
				currentByteSize += LocalGetStructByteSize(compiler, structMemberType) * structMemberType.vecsize;
			}
			else
			{
				currentByteSize += GetTypeByteSize(structMemberType.basetype) * structMemberType.vecsize;
			}
		}
	}
	else
	{
		currentByteSize += GetTypeByteSize(typeToCheck.basetype);
	}
	return currentByteSize;
}

void Hail::ParseShader(ReflectedShaderData& returnData, const char* shaderName, const char* compiledShader, uint32 shaderLength)
{
	const uint32 uintShaderLength = shaderLength / 4u;
	GrowingArray<uint32> compiledCode;
	compiledCode.Resize(uintShaderLength);
	compiledCode.Fill();
	memcpy(compiledCode.Data(), compiledShader, shaderLength);
	spirv_cross::CompilerReflection comp(std::move(compiledCode.Data()), compiledCode.Size()); // const uint32_t *, size_t interface is also available

	spirv_cross::ShaderResources res = comp.get_shader_resources();

	Debug_PrintConsoleStringL(StringL::Format("\nShader: %s\n", shaderName));

	for (auto& resource : res.stage_inputs)
	{
		unsigned location = comp.get_decoration(resource.id, spv::DecorationLocation);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Input %s with type: %s, location = %u\n", resource.name.c_str(), typeString.Data(), location));

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_bindingLocation = location;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_set = 0xff;
		returnData.m_shaderInputs.Add(decoration);
	}
	for (auto& resource : res.stage_outputs)
	{
		unsigned location = comp.get_decoration(resource.id, spv::DecorationLocation);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Output %s with type: %s, location = %u\n", resource.name.c_str(), typeString.Data(), location));
		if (type.basetype == spirv_cross::SPIRType::Struct)
		{
			// TODO: Throw error 
		}
		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_bindingLocation = location;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_set = 0xff;
		returnData.m_shaderOutputs.Add(decoration);
	}

	for (auto& resource : res.sampled_images)
	{
		unsigned int set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned int binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Image %s at set = %u, binding = %u, with type: %s\n", resource.name.c_str(), set, binding, typeString.Data()));

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::SampledImage;
		decoration.m_set = set;
		returnData.m_setDecorations[set].m_sampledImages.Add(decoration);

		if (set < InstanceDomain)
		{
			returnData.m_globalMaterialDecorations.Add(decoration);
		}
	}

	for (auto& resource : res.separate_images)
	{
		unsigned int set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned int binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Image %s at set = %u, binding = %u, with type: %s\n", resource.name.c_str(), set, binding, typeString.Data()));

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::Image;
		decoration.m_set = set;
		returnData.m_setDecorations[set].m_images.Add(decoration);

		if (set < InstanceDomain)
		{
			returnData.m_globalMaterialDecorations.Add(decoration);
		}
	}

	for (auto& resource : res.separate_samplers)
	{
		unsigned int set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned int binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Sampler %s at set = %u, binding = %u, with type: %s\n", resource.name.c_str(), set, binding, typeString.Data()));
		H_ASSERT(set == 0, "Samplers should only reside at set 0");
		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::Sampler;
		decoration.m_set = set;
		returnData.m_setDecorations[set].m_samplers.Add(decoration);

		if (set < InstanceDomain)
		{
			returnData.m_globalMaterialDecorations.Add(decoration);
		}
	}

	for (auto& resource : res.uniform_buffers)
	{
		unsigned set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Uniform Buffer %s at set = %u, binding = %u, with type: %s\n", resource.name.c_str(), set, binding, typeString.Data()));

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_elementCount = type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::UniformBuffer;
		decoration.m_set = set;
		returnData.m_setDecorations[set].m_uniformBuffers.Add(decoration);
		if (set < InstanceDomain)
		{
			returnData.m_globalMaterialDecorations.Add(decoration);
		}
	}
	for (auto& resource : res.storage_buffers)
	{
		unsigned set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Storage buffer %s at set = %u, binding = %u, with type: %s\n", resource.name.c_str(), set, binding, typeString.Data()));

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_elementCount = type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_type = eDecorationType::ShaderStorageBuffer;
		decoration.m_set = set;
		returnData.m_setDecorations[set].m_storageBuffers.Add(decoration);
		if (set < InstanceDomain)
		{
			returnData.m_globalMaterialDecorations.Add(decoration);
		}
	}

	for (auto& resource : res.push_constant_buffers)
	{
		unsigned set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);

		Debug_PrintConsoleStringL(StringL::Format("Push constant %s at set = %u, binding = %u, with type: %s\n", resource.name.c_str(), set, binding, typeString.Data()));

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type) * type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_type = eDecorationType::PushConstant;
		decoration.m_set = set;
		returnData.m_pushConstants.Add(decoration);
	}
}
