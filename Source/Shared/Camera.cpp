#include "Shared_PCH.h"
#include "Camera.h"


Hail::Camera::Camera() :
    m_fov(90.f),
    m_far(100000.f),
    m_near(1.f),
    m_orthographic(false)
{
}

void Hail::Camera::SetFar(float far)
{
    m_far = far;
}

void Hail::Camera::SetNear(float near)
{
    m_near = near;
}

void Hail::Camera::SetOrthographic(float orthoState)
{
    m_orthographic = orthoState;
}

Hail::Camera Hail::Camera::LerpCamera(const Camera cam1, const Camera cam2, float t)
{
    Camera returnCam;
    returnCam.m_orthographic = cam2.m_orthographic;
    returnCam.m_fov = Math::Lerp(cam1.m_fov, cam2.m_fov, t);
    returnCam.m_far = Math::Lerp(cam1.m_far, cam2.m_far, t);
    returnCam.m_near = Math::Lerp(cam1.m_near, cam2.m_near, t);
    returnCam.m_transform = Transform3D::LerpTransforms_t(cam1.m_transform, cam2.m_transform, t);
    return returnCam;
}

Hail::Camera2D::Camera2D() :
    m_zoom(1.f),
    m_position(0.f)
{
}

void Hail::Camera2D::SetPosition(glm::vec2 position)
{
    m_position = position;
}

void Hail::Camera2D::SetZoom(float zoom)
{
    m_zoom = zoom;
}

void Hail::Camera2D::TransformToCameraSpace(Transform2D& transformToTransform) const
{
    const glm::vec2 transformedPosition = transformToTransform.GetPosition() * m_zoom - m_position;
    transformToTransform.SetPosition(glm::vec2(transformedPosition.x / m_screenResolution.x + 0.5f, transformedPosition.y / m_screenResolution.y + 0.5f));
    transformToTransform.SetScale(transformToTransform.GetScale() * m_zoom);
}

void Hail::Camera2D::TransformLineToCameraSpace(glm::vec3& start, glm::vec3& end) const
{
    glm::vec2 transformedPosition1 = glm::vec2(start) * m_zoom - m_position;
    glm::vec2 transformedPosition2 = glm::vec2(end) * m_zoom - m_position;
    transformedPosition1 = glm::vec2(transformedPosition1.x / m_screenResolution.x + 0.5f, transformedPosition1.y / m_screenResolution.y + 0.5f);
    transformedPosition2 = glm::vec2(transformedPosition2.x / m_screenResolution.x + 0.5f, transformedPosition2.y / m_screenResolution.y + 0.5f);
    start.x = transformedPosition1.x;
    start.y = transformedPosition1.y;
    end.x = transformedPosition2.x;
    end.y = transformedPosition2.y;
}
