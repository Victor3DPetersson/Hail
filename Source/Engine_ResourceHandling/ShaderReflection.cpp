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

void LocalSetImageTypeDataFromFormat(ShaderDecoration& decorationToFill, spv::ImageFormat imageFormat)
{
	decorationToFill.m_elementCount = 0;
	decorationToFill.m_valueType = eShaderValueType::none;
	switch (imageFormat)
	{
		case spv::ImageFormatRgba32f:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::float32;
			break;
		}
		case spv::ImageFormatRgba16f:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::float16;
			break;
		}
		case spv::ImageFormatR32f:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::float32;
			break;
		}
		case spv::ImageFormatRgba8:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::uint8norm;
			break;
		}
		case spv::ImageFormatRgba8Snorm:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::int8norm;
			break;
		}
		case spv::ImageFormatRg32f:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::float32;
			break;
		}
		case spv::ImageFormatRg16f:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::float16;
			break;
		}
		case spv::ImageFormatR11fG11fB10f:
		{
			decorationToFill.m_elementCount = 3;
			decorationToFill.m_valueType = eShaderValueType::composite;
			break;
		}
		case spv::ImageFormatR16f:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::float16;
			break;
		}
		case spv::ImageFormatRgba16:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::uint16;
			break;
		}
		case spv::ImageFormatRgb10A2:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::composite;
			break;
		}
		case spv::ImageFormatRg16:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::uint16;
			break;
		}
		case spv::ImageFormatRg8:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::uint8;
			break;
		}
		case spv::ImageFormatR16:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::uint16;
			break;
		}
		case spv::ImageFormatR8:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::uint8norm;
			break;
		}
		case spv::ImageFormatRgba16Snorm:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::int16norm;
			break;
		}
		case spv::ImageFormatRg16Snorm:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::int16norm;
			break;
		}
		case spv::ImageFormatRg8Snorm:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::int8norm;
			break;
		}
		case spv::ImageFormatR16Snorm:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::int16norm;
			break;
		}
		case spv::ImageFormatR8Snorm:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::int8norm;
			break;
		}
		case spv::ImageFormatRgba32i:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::int32;
			break;
		}
		case spv::ImageFormatRgba16i:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::int16;
			break;
		}
		case spv::ImageFormatRgba8i:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::int8;
			break;
		}
		case spv::ImageFormatR32i:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::int32;
			break;
		}
		case spv::ImageFormatRg32i:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::int32;
			break;
		}
		case spv::ImageFormatRg16i:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::int16;
			break;
		}
		case spv::ImageFormatRg8i:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::int8;
			break;
		}
		case spv::ImageFormatR16i:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::float32;
			break;
		}
		case spv::ImageFormatR8i:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::int8;
			break;
		}
		case spv::ImageFormatRgba32ui:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::uint32;
			break;
		}
		case spv::ImageFormatRgba16ui:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::uint16;
			break;
		}
		case spv::ImageFormatRgba8ui:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::uint8;
			break;
		}
		case spv::ImageFormatR32ui:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::uint32;
			break;
		}
		case spv::ImageFormatRgb10a2ui:
		{
			decorationToFill.m_elementCount = 4;
			decorationToFill.m_valueType = eShaderValueType::composite;
			break;
		}
		case spv::ImageFormatRg32ui:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::uint32;
			break;
		}
		case spv::ImageFormatRg16ui:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::uint16;
			break;
		}
		case spv::ImageFormatRg8ui:
		{
			decorationToFill.m_elementCount = 2;
			decorationToFill.m_valueType = eShaderValueType::uint8;
			break;
		}
		case spv::ImageFormatR16ui:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::uint16;
			break;
		}
		case spv::ImageFormatR8ui:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::uint8;
			break;
		}
		case spv::ImageFormatR64ui:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::uint64;
			break;
		}
		case spv::ImageFormatR64i:
		{
			decorationToFill.m_elementCount = 1;
			decorationToFill.m_valueType = eShaderValueType::int64;
			break;
		}
		case spv::ImageFormatMax:
		case spv::ImageFormatUnknown:
		default:
		{
			// Optional: handle unknown or unsupported formats
			break;
		}
	}
}

uint32 LocalGetTypeByteSize(spirv_cross::SPIRType::BaseType type)
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
				currentByteSize += LocalGetTypeByteSize(structMemberType.basetype) * structMemberType.vecsize;
			}
		}
	}
	else
	{
		currentByteSize += LocalGetTypeByteSize(typeToCheck.basetype);
	}
	return currentByteSize;
}

void LocalFillDispatchWorkgroups(ReflectedShaderData& returnData, const spirv_cross::SPIREntryPoint& entryPoint, const spirv_cross::CompilerReflection& comp)
{
	const spirv_cross::SPIRConstant& x = comp.get_constant(entryPoint.workgroup_size.id_x);
	const spirv_cross::SPIRType& xType = comp.get_type(x.constant_type);
	H_ASSERT(xType.basetype == spirv_cross::SPIRType::BaseType::UInt);
	returnData.m_entryDecorations.workGroupSize.x = x.m.c->r->u32;

	const spirv_cross::SPIRConstant& y = comp.get_constant(entryPoint.workgroup_size.id_y);
	const spirv_cross::SPIRType& yType = comp.get_type(y.constant_type);
	H_ASSERT(yType.basetype == spirv_cross::SPIRType::BaseType::UInt);
	returnData.m_entryDecorations.workGroupSize.y = y.m.c->r->u32;

	const spirv_cross::SPIRConstant& z = comp.get_constant(entryPoint.workgroup_size.id_z);
	const spirv_cross::SPIRType& zType = comp.get_type(z.constant_type);
	H_ASSERT(zType.basetype == spirv_cross::SPIRType::BaseType::UInt);
	returnData.m_entryDecorations.workGroupSize.z = z.m.c->r->u32;
}

void Hail::ParseShader(ReflectedShaderData& returnData, eShaderStage shaderStage, const char* shaderName, const char* compiledShader, uint32 shaderLength)
{
	returnData.m_bIsValid = true;
	const uint32 uintShaderLength = shaderLength / 4u;
	GrowingArray<uint32> compiledCode;
	compiledCode.Resize(uintShaderLength);
	compiledCode.Fill();
	memcpy(compiledCode.Data(), compiledShader, shaderLength);
	spirv_cross::CompilerReflection* pComp = new spirv_cross::CompilerReflection(std::move(compiledCode.Data()), compiledCode.Size()); // const uint32_t *, size_t interface is also available
	spirv_cross::CompilerReflection& comp = *pComp;

	spirv_cross::ShaderResources res = comp.get_shader_resources();
	auto specializationConstants = comp.get_specialization_constants();
	Debug_PrintConsoleStringL(StringL::Format("\nShader: %s\n", shaderName));

	spirv_cross::SPIREntryPoint entryPoint;
	bool bUsesWorkGroups = false;
	switch (shaderStage)
	{
	case Hail::eShaderStage::Compute:
		entryPoint = comp.get_entry_point("main", spv::ExecutionModelGLCompute);
		LocalFillDispatchWorkgroups(returnData, entryPoint, comp);
		bUsesWorkGroups = true;
		break;
	case Hail::eShaderStage::Vertex:
		entryPoint = comp.get_entry_point("main", spv::ExecutionModelVertex);
		returnData.m_entryDecorations.workGroupSize = glm::uvec3(0u);
		break;
	case Hail::eShaderStage::Fragment:
		entryPoint = comp.get_entry_point("main", spv::ExecutionModelFragment);
		returnData.m_entryDecorations.workGroupSize = glm::uvec3(0u);
		break;
	case Hail::eShaderStage::Amplification:
		entryPoint = comp.get_entry_point("main", spv::ExecutionModelTaskEXT);
		LocalFillDispatchWorkgroups(returnData, entryPoint, comp);
		bUsesWorkGroups = true;
		break;
	case Hail::eShaderStage::Mesh:
		entryPoint = comp.get_entry_point("main", spv::ExecutionModelMeshEXT);
		LocalFillDispatchWorkgroups(returnData, entryPoint, comp);
		bUsesWorkGroups = true;
		break;
	case Hail::eShaderStage::None:
		break;
	default:
		break;
	}
	
	if (bUsesWorkGroups && 
		(returnData.m_entryDecorations.workGroupSize.x == 0u || 
			returnData.m_entryDecorations.workGroupSize.y == 0u || 
			returnData.m_entryDecorations.workGroupSize.z == 0u))
	{
		returnData.m_bIsValid = false;
		H_ERROR("Workgroup size can not be 0");
	}

	for (auto& resource : res.stage_inputs)
	{
		unsigned location = comp.get_decoration(resource.id, spv::DecorationLocation);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Input %s with type: %s, location = %u\n", resource.name.c_str(), typeString.Data(), location));

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_bindingLocation = location;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_set = 0xff;
		decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
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
			H_ERROR("Base type of output is a struct, invalid output");
			returnData.m_bIsValid = false;
		}
		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_bindingLocation = location;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_set = 0xff;
		decoration.m_accessQualifier = eShaderAccessQualifier::WriteOnly;
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
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::SampledImage;
		decoration.m_set = set;
		decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_decorations[binding] = (decoration);
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_indices.Add(binding);

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

		H_ASSERT(type.basetype == spirv_cross::SPIRType::BaseType::Image);

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::Image;
		decoration.m_set = set;
		decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_decorations[binding] = (decoration);
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_indices.Add(binding);

		if (set < InstanceDomain)
		{
			returnData.m_globalMaterialDecorations.Add(decoration);
		}
	}

	for (auto& resource : res.storage_images)
	{
		unsigned int set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned int binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		const spirv_cross::SPIRType& type = comp.get_type(resource.base_type_id);
		String64 typeString = LocalGetTypeFromSpirVDataType(type.basetype, type.vecsize);
		Debug_PrintConsoleStringL(StringL::Format("Image %s at set = %u, binding = %u, with type: %s\n", resource.name.c_str(), set, binding, typeString.Data()));

		H_ASSERT(type.basetype == spirv_cross::SPIRType::BaseType::Image);

		if (returnData.m_setDecorations[set][(uint32)eDecorationType::Image].m_decorations[binding].m_type != eDecorationType::Count)
		{
			H_ERROR("Duplicate decoration at binding slot");
			returnData.m_bIsValid = false;
			continue;
		}

		ShaderDecoration decoration;
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_bindingLocation = binding;
		LocalSetImageTypeDataFromFormat(decoration, type.image.format);

		decoration.m_type = eDecorationType::Image;
		decoration.m_set = set;
		if (comp.has_decoration(resource.id, spv::DecorationNonWritable))
		{
			decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		}
		else if (comp.has_decoration(resource.id, spv::DecorationNonReadable))
		{
			decoration.m_accessQualifier = eShaderAccessQualifier::WriteOnly;
			if (decoration.m_valueType == eShaderValueType::none)
			{
				H_ERROR("Invalid image as no defined value type is present in shader code");
				returnData.m_bIsValid = false;
			}
		}
		else
		{
			decoration.m_accessQualifier = eShaderAccessQualifier::ReadWrite;
			if (decoration.m_valueType == eShaderValueType::none)
			{
				H_ERROR("Invalid image as no defined value type is present in shader code");
				returnData.m_bIsValid = false;
			}
		}
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_decorations[binding] = (decoration);
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_indices.Add(binding);

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
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::Sampler;
		decoration.m_set = set;
		decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_decorations[binding] = (decoration);
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_indices.Add(binding);

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
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_elementCount = type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_valueType = eShaderValueType::none;
		decoration.m_type = eDecorationType::UniformBuffer;
		decoration.m_set = set;
		decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_decorations[binding] = (decoration);
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_indices.Add(binding);
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
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_elementCount = type.vecsize;
		decoration.m_bindingLocation = binding;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_type = eDecorationType::ShaderStorageBuffer;
		decoration.m_set = set;
		spirv_cross::Bitset bufferFlags = comp.get_buffer_block_flags(resource.id);
		decoration.m_accessQualifier = bufferFlags.get(spv::DecorationNonWritable) ? eShaderAccessQualifier::ReadOnly : eShaderAccessQualifier::ReadWrite;

		if (bufferFlags.get(spv::DecorationNonWritable))
		{
			decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		}
		else if (bufferFlags.get(spv::DecorationNonReadable))
		{
			decoration.m_accessQualifier = eShaderAccessQualifier::WriteOnly;
		}
		else
		{
			decoration.m_accessQualifier = eShaderAccessQualifier::ReadWrite;
		}
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_decorations[binding] = (decoration);
		returnData.m_setDecorations[set][(uint32)decoration.m_type].m_indices.Add(binding);
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
		decoration.m_byteSize = LocalGetStructByteSize(comp, type);
		decoration.m_bindingLocation = binding;
		decoration.m_elementCount = type.vecsize;
		decoration.m_valueType = LocalGetValueTypeFromSpirVType(type.basetype);
		decoration.m_type = eDecorationType::PushConstant;
		decoration.m_set = set;
		decoration.m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		returnData.m_pushConstants.Add(decoration);
	}
	SAFEDELETE(pComp);
}
