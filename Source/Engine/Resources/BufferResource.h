#pragma once

#include "EngineConstants.h"
#include "Resources\MaterialResources.h"

#include "glm\mat4x4.hpp"
#include "glm\vec2.hpp"

namespace Hail
{
	class RenderingDevice;

	enum class eShaderBufferDomain : uint8
	{
		GpuOnly,
		CpuOnly,
		GpuToCpu, // transfer
		CpuToGpu, // transfer
		Undefined
	};

	// For GPU only memory this can be left undefined, otherwise this must be specified
	enum class eShaderBufferUpdateFrequency : uint8
	{
		PerFrame, 
		Sporadic, // If this can be updated sometimes, 
		Once, // Will only update and send its data on creation, useful for uploading of large chunks of data, like font data
		Never,
		Undefined // 
	};

	enum class eBufferType : uint32
	{
		vertex,
		index,
		uniform,
		structured, 
		staging
	};

	struct TutorialUniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct PerFrameUniformBuffer
	{
		glm::uvec2 mainRenderResolution; // Actual render resolution on the final window
		glm::uvec2 mainWindowResolution; 

		glm::uvec2 renderTargetRes; // Main rendertargets resolution
		glm::vec2 totalTime_horizonLevel;

		glm::vec2 deltaTime_cameraZoom;
		glm::vec2 cameraPos;

		glm::vec2 mousePos; // normalized
		glm::vec2 mouseRMBLMBDeltas;

		glm::vec2 playerPosition; // TEMP;
		glm::vec2 padding;
	};

	struct PerCameraUniformBuffer {
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct BufferViewProperties
	{
		eShaderAccessQualifier accessQualifier = eShaderAccessQualifier::ReadOnly;
		uint32 elementByteSize = 0;
		uint32 offset = 0;
		uint32 numberOfElements = 0;
		eShaderBufferDomain domain = eShaderBufferDomain::Undefined;
	};


	struct BufferProperties
	{
		eBufferType type;
		uint32 elementByteSize = 0;
		uint32 numberOfElements = 0;
		eShaderAccessQualifier accessQualifier = eShaderAccessQualifier::ReadOnly;
		eShaderBufferDomain domain = eShaderBufferDomain::Undefined;
		eShaderBufferUpdateFrequency updateFrequency = eShaderBufferUpdateFrequency::Undefined;
	};


	class BufferObject
	{
	public:
		bool Init(RenderingDevice* device, BufferProperties properties, const char* name);
		virtual void CleanupResource(RenderingDevice* device) = 0;

		const BufferProperties& GetProperties() { return m_properties; }
		bool UsesFrameInFlight() const { return m_bUsesFramesInFlight; }
		virtual bool UsesPersistentMapping(RenderingDevice* device, uint32 frameInFlight) = 0; // 
		const eShaderAccessQualifier GetAccessQualifier() { return m_accessQualifier; }
		void SetAccessQualifier(eShaderAccessQualifier qualifier) { m_accessQualifier = qualifier; }
		const uint32 GetBufferSize() const;
		const uint32 GetID() const { return m_id; }
	protected:

		virtual bool InternalInit(RenderingDevice* device) = 0;
		static uint32 g_idCounter;

		BufferProperties m_properties;
		eShaderAccessQualifier m_accessQualifier = eShaderAccessQualifier::ReadOnly;
		uint32 m_id = UINT_MAX;
		String64 m_name;
		bool m_bUsesFramesInFlight = false;
	};


	class BufferView
	{
	public:
		BufferObject* m_pBuffer;
		BufferViewProperties m_properties;
	};
}

