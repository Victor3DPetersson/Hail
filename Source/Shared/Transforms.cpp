#include "Shared_PCH.h"

#include "Transforms.h"
#include "glm\trigonometric.hpp"
#include "glm\common.hpp"
#include "glm\ext\quaternion_common.hpp"
#include "glm\ext\quaternion_trigonometric.hpp"
#include "glm\ext\quaternion_transform.hpp"
#include "glm\ext.hpp"
#include "glm\gtx\euler_angles.hpp"
#include "glm\gtx\matrix_decompose.hpp"
#include "MathUtils.h"

#include "DebugMacros.h"

namespace Hail
{
	Transform3D::Transform3D(const Transform3D& transform)
	{
		m_pos = transform.m_pos;
		m_scl = transform.m_scl;
		m_rot = transform.m_rot;
		m_quat = transform.m_quat;
	}
	Transform3D::Transform3D(const glm::highp_mat4& matrix)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, m_scl, m_quat, m_pos, skew, perspective);
		m_rot = glm::axis(m_quat);
	}

	const Transform3D& Transform3D::operator=(const Transform3D& transform)
	{
		m_pos = transform.m_pos;
		m_scl = transform.m_scl;
		m_rot = transform.m_rot;
		m_quat = transform.m_quat;
		return *this;
	}

	const Transform3D& Transform3D::operator=(const glm::highp_mat4& matrix)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, m_scl, m_quat, m_pos, skew, perspective);
		m_rot = glm::axis(m_quat);
		return *this;
	}

	void Transform3D::LookAt(const glm::vec3 worldPositionToLookAt, const glm::vec3 fromPos)
	{
		glm::mat4 matrix = glm::lookAt(fromPos, worldPositionToLookAt, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, m_scl, m_quat, m_pos, skew, perspective);
		m_rot = glm::axis(m_quat);
	}

	glm::highp_mat4 Transform3D::LerpTransforms_mat(const Transform3D& tr1, const Transform3D& tr2, const float t)
	{
		glm::highp_mat4 outMatrix;

		glm::qua<float, glm::packed_highp> rotation = glm::slerp(tr1.m_quat, tr2.m_quat, t);

		glm::vec3 scale = glm::mix(tr1.m_scl, tr2.m_scl, t);
		glm::vec3 pos = glm::mix(tr1.m_pos, tr2.m_pos, t);
		outMatrix = glm::mat4_cast(rotation);

		outMatrix = glm::scale(outMatrix, scale);
		outMatrix = glm::translate(outMatrix, pos);

		return outMatrix;
	}

	Transform3D Transform3D::LerpTransforms_t(const Transform3D& tr1, const Transform3D& tr2, const float t)
	{
		Transform3D outTransform;
		outTransform.m_pos = glm::mix(tr1.m_pos, tr2.m_pos, t);
		outTransform.m_scl = glm::mix(tr1.m_scl, tr2.m_scl, t);
		outTransform.m_rot = glm::mix(tr1.m_rot, tr2.m_rot, t);
		outTransform.m_quat = glm::slerp(tr1.m_quat, tr2.m_quat, t);
		return outTransform;
	}

	glm::highp_mat4 Transform3D::GetMatrix(const Transform3D& transform)
	{
		glm::highp_mat4 outMatrix = glm::identity<glm::highp_mat4>();
		outMatrix = glm::mat4_cast(transform.m_quat);
		outMatrix[3] += glm::vec4(transform.m_pos, 1.0f);
		outMatrix = glm::scale(outMatrix, transform.m_scl);
		return outMatrix;
	}

	glm::vec3 Transform3D::GetForward() const
	{
		return 	m_quat * glm::vec3(0.0, 0.0, 1.0);
	}

	glm::vec3  Transform3D::GetRight() const
	{
		return m_quat * glm::vec3(1.0, 0.0, 0.0);
	}

	glm::vec3  Transform3D::GetUp() const
	{
		return m_quat * glm::vec3(0.0, 1.0, 0.0);
	}

	void Transform3D::SetPosition(const glm::vec3& pos)
	{
		m_pos = pos;
	}

	void Transform3D::AddToPosition(const glm::vec3& pos)
	{
		m_pos += pos;
	}
	void Transform3D::SetRotation(const glm::vec3& rotEuler)
	{
		m_rot.x = fmod(rotEuler.x, 360.0f);
		if (m_rot.x > 180)
			m_rot.x -= 360;
		else if (m_rot.x < -180)
			m_rot.x += 360;

		m_rot.y = fmod(rotEuler.y, 360.0f);
		if (m_rot.y > 180)
			m_rot.y -= 360;
		else if (m_rot.y < -180)
			m_rot.y += 360;

		m_rot.z = fmod(rotEuler.z, 360.0f);
		if (m_rot.z > 180)
			m_rot.z -= 360;
		else if (m_rot.z < -180)
			m_rot.z += 360;

		m_quat = glm::quat(glm::radians(m_rot));
	}
	//sets rotation with a quaternion
	void Transform3D::SetRotation(const glm::qua<float, glm::packed_highp>& rot)
	{
		m_quat = rot;
		m_rot = glm::axis(m_quat);
	}

	void Transform3D::AddToRotation(const glm::vec3& rot)
	{
		SetRotation(m_rot + rot);
	}

	void Transform3D::SetScale(const glm::vec3& scl)
	{
		m_scl = scl;
	}

	void Transform3D::AddToScale(const glm::vec3& scl)
	{
		m_scl += scl;
	}

	const Transform2D& Transform2D::operator=(const Transform2D& transform)
	{
		m_pos = transform.m_pos;
		m_scl = transform.m_scl;
		m_rot = transform.m_rot;
		return *this;
	}

	const Transform2D Hail::Transform2D::LerpTransforms(const Transform2D& tr1, const Transform2D& tr2, const float t)
	{
		Transform2D outTransform;
		outTransform.m_pos = glm::mix(tr1.m_pos, tr2.m_pos, t);
		outTransform.m_scl = glm::mix(tr1.m_scl, tr2.m_scl, t);
		outTransform.m_rot = glm::mix(tr1.m_rot, tr2.m_rot, t);
		return outTransform;
	}

	void Hail::Transform2D::SetPosition(const glm::vec2 pos)
	{
		m_pos = pos;
	}

	void Hail::Transform2D::AddToPosition(const glm::vec2 pos)
	{
		m_pos += pos;
	}

	void Hail::Transform2D::SetRotation(const float rotInEulerAngles)
	{
		m_rot = fmod(rotInEulerAngles, 360.0f);
	}

	void Hail::Transform2D::AddToRotation(const float rotInEulerAngles)
	{
		m_rot += rotInEulerAngles;
		m_rot = fmod(m_rot, 360.0f);
		if (m_rot > 180)
			m_rot -= 360;
		else if (m_rot < -180)
			m_rot += 360;
	}

	void Hail::Transform2D::SetScale(const glm::vec2 scl)
	{
		m_scl = scl;
	}

	void Hail::Transform2D::AddToScale(const glm::vec2 scl)
	{
		m_scl += scl;
	}
	void Hail::Transform2D::LookAt(const glm::vec2 normalizedDirection)
	{
		float theta = atan2f(normalizedDirection.y, normalizedDirection.x);
		m_rot = fmod(Math::RadToDegf * theta, 360.0f);
	}
}