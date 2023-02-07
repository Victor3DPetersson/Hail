#include "Shared_PCH.h"
#include "Camera.h"


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
