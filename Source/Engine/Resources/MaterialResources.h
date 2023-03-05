#pragma once
#include "Vertices.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"

namespace Hail
{
	enum class MATERIAL_TYPE : uint32_t
	{
		SPRITE,
		FULLSCREEN,
		MODEL3D
	};
	enum class BLEND_MODE : uint32_t
	{
		NORMAL,
		ALPHABLEND,
		CUTOUT,
		ADDITIVE
	};
	class Material
	{
	public:




	protected:

		uint32_t m_identifier = 0;

		String64 m_vertexShader;
		String64 m_fragmentShader;
		String64 m_tessShader;
		String64 m_controlShader;

		//Add shader reflection data here

		MATERIAL_TYPE m_type;
	};



	class MaterialInstance
	{
	public:


	private:
		//Textures
		uint32_t m_materialIdentifier = 0;
		BLEND_MODE m_blendMode;
		//Other instanced parameters
		glm::vec4 m_instanceFloatParameters;

	};
}

