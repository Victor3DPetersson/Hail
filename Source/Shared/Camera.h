#pragma once
#include "Transforms.h"
#include "MathUtils.h"
namespace Hail
{
	class Camera
	{
	public:

		Transform3D& GetTransform() { return m_transform; }
		float GetFov() { return m_fov; }
		void SetFov(float fov) { m_fov = fov; }

		float GetFar() { return m_far; }
		void SetFar(float far);

		float GetNear() { return m_near; }
		void SetNear(float near);

		bool GetOrthographic() { return m_orthographic; }
		void SetOrthographic(float orthoState);

		static Camera LerpCamera(const Camera cam1, const Camera cam2, float t);

	private:
		Transform3D m_transform;
		float m_fov = 90.0f;
		float m_far = 10000.0f;
		float m_near = 1.0f;

		bool m_orthographic = false;
	};
}
 