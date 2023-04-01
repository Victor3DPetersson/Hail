#pragma once
#include "glm\vec3.hpp"
#include "glm\vec2.hpp"
#include "glm\ext\quaternion_float.hpp"



namespace Hail
{
	class Transform3D
	{
	public:
		Transform3D() {
			m_pos = glm::vec3();
			m_scl = glm::vec3(1, 1, 1);
			m_rot = glm::vec3();
			m_quat = glm::qua<float, glm::packed_highp>();
		}
		Transform3D(const Transform3D& matrix);
		Transform3D(const glm::highp_mat4& matrix);

		const Transform3D& operator=(const Transform3D& transform);
		const Transform3D& operator=(const glm::highp_mat4& matrix);

		__forceinline const glm::vec3 GetPosition() const { return m_pos; };
		__forceinline const glm::vec3 GetRotation() const { return m_rot; };
		__forceinline const glm::qua<float, glm::packed_highp> GetRotationQ() const { return m_quat; };
		__forceinline const glm::vec3 GetScale()	const { return m_scl; };
		__forceinline glm::vec3 GetPosition() { return m_pos; };
		__forceinline glm::vec3 GetRotation() { return m_rot; };
		__forceinline glm::qua<float, glm::packed_highp> GetRotationQ() { return m_quat; };
		__forceinline glm::vec3 GetScale() { return m_scl; };


		void LookAt(const glm::vec3 worldPositionToLookAt, const glm::vec3 fromPos);
		static const glm::highp_mat4 LerpTransforms_mat(const Transform3D& tr1, const Transform3D& tr2, const float t);
		static const Transform3D LerpTransforms_t(const Transform3D& tr1, const Transform3D& tr2, const float t);
		static const glm::highp_mat4 GetMatrix(const Transform3D& transform);
		const glm::vec3 GetForward();
		const glm::vec3 GetRight();
		const glm::vec3 GetUp();

		void SetPosition(const glm::vec3& pos);
		void AddToPosition(const glm::vec3& pos);
		void SetRotation(const glm::vec3& rot);
		void SetRotation(const glm::qua<float, glm::packed_highp>& rot);
		//Set rotation in Eueler angles
		void AddToRotation(const glm::vec3& rotEuler);
		void SetScale(const glm::vec3& scl);
		void AddToScale(const glm::vec3& scl);

	private:
		glm::vec3 m_pos, m_scl, m_rot;
		glm::qua<float, glm::packed_highp> m_quat;
	};


	class Transform2D
	{
	public:
		Transform2D() {
			m_pos = glm::vec2();
			m_scl = glm::vec2(1, 1);
			m_rot = 90.0f;
		}
		const Transform2D& operator=(const Transform2D& transform);

		__forceinline const glm::vec2 GetPosition() const { return m_pos; };
		//Returns rotation in EulerAngles
		__forceinline const float GetRotation() const { return m_rot; };
		__forceinline const glm::vec2 GetScale()	const { return m_scl; };
		__forceinline glm::vec2 GetPosition() { return m_pos; };
		__forceinline float GetRotation() { return m_rot; };
		__forceinline glm::vec2 GetScale() { return m_scl; };

		static const Transform2D LerpTransforms(const Transform2D& tr1, const Transform2D& tr2, const float t);

		void SetPosition(const glm::vec2 pos);
		void AddToPosition(const glm::vec2 pos);
		void SetRotation(const float rotInEulerAngles);
		void AddToRotation(const float rotInEulerAngles);
		void SetScale(const glm::vec2 scl);
		void AddToScale(const glm::vec2 scl);
		void LookAt(const glm::vec2 normalizedDirection);

	private:
		glm::vec2 m_pos, m_scl;
		float m_rot;
	};
}
