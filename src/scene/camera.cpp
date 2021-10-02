#include "camera.h"

#include <glm/gtx/quaternion.hpp>

#include <scene/glm-wrapper.hpp>

namespace nevk
{

void Camera::updateViewMatrix()
{
    //glm::mat4 rotM = mat4_cast(mOrientation);
    const glm::float4x4 rotM{ mOrientation };
    glm::float4x4 transM = glm::translate(glm::float4x4(1.0f), -position);
    if (type == CameraType::firstperson)
    {
        matrices.view = rotM * transM;
    }
    else
    {
        matrices.view = transM * rotM;
    }
    updated = true;
}

glm::float3 Camera::getFront()
{
    return glm::conjugate(mOrientation) * glm::float3(0.0f, 0.0f, -1.0f);
}

glm::float3 Camera::getUp()
{
    return glm::conjugate(mOrientation) * glm::float3(0.0f, 1.0f, 0.0f);
}

glm::float3 Camera::getRight()
{
    return glm::conjugate(mOrientation) * glm::float3(1.0f, 0.0f, 0.0f);
}

bool Camera::moving()
{
    return keys.left || keys.right || keys.up || keys.down || keys.forward || keys.back;
}

float Camera::getNearClip()
{
    return znear;
}

float Camera::getFarClip()
{
    return zfar;
}

void Camera::setFov(float fov)
{
    this->fov = fov;
}

glm::float4x4 perspective(float fov, float aspect_ratio, float n, float f, glm::float4x4* inverse)
//{
//    float h = 1.0 / std::tan(glm::radians(fov) * 0.5f);
//    float w = h / aspect_ratio;
//    float a = -n / (f - n);
//    float b = (n * f) / (f - n);
//    glm::float4x4 proj = {
//        w, 0, 0, 0,
//        0, -h, 0, 0,
//        0, 0, a, 1.0,
//        0, 0, b, 0
//    };
//
//    return proj;
//}
{
    float focal_length = 1.0f / std::tan(glm::radians(fov) / 2.0f);

    float x = focal_length / aspect_ratio;
    float y = -focal_length;
    float A = n / (f - n);
    float B = f * A;

    glm::float4x4 projection({
        x,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        y,
        0.0f,
        0.0f,
        
        0.0f,
        0.0f,
        A,
        B,
        
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
    });

    if (inverse)
    {
        *inverse = glm::transpose(glm::float4x4({
            1 / x,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1 / y,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            -1.0f,
            0.0f,
            0.0f,
            1 / B,
            A / B,
        }));
    }

    return glm::transpose(projection);
    //return projection;
}

void Camera::setPerspective(float _fov, float _aspect, float _znear, float _zfar)
{
    fov = _fov;
    znear = _znear;
    zfar = _zfar;
    // swap near and far plane for reverse z
    matrices.perspective = perspective(fov, _aspect, _zfar, _znear, &matrices.invPerspective);
}

glm::float4x4& Camera::getPerspective()
{
    return matrices.perspective;
}

glm::float4x4 Camera::getView()
{
    return matrices.view;
}

void Camera::updateAspectRatio(float _aspect)
{
    //matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    setPerspective(fov, _aspect, znear, zfar);
}

void Camera::setPosition(glm::float3 _position)
{
    position = _position;
    updateViewMatrix();
}

glm::float3 Camera::getPosition()
{
    return position;
}

void Camera::setRotation(glm::quat rotation)
{
    mOrientation = rotation;
    updateViewMatrix();
}

void Camera::rotate(float rightAngle, float upAngle)
{
    glm::quat a = glm::angleAxis(glm::radians(upAngle) * rotationSpeed, glm::float3(1.0f, 0.0f, 0.0f));
    glm::quat b = glm::angleAxis(glm::radians(rightAngle) * rotationSpeed, glm::float3(0.0f, 1.0f, 0.0f));
    mOrientation = glm::normalize(a * mOrientation * b);
    updateViewMatrix();
}

void Camera::setTranslation(glm::float3 translation)
{
    position = translation;
    updateViewMatrix();
}

void Camera::translate(glm::float3 delta)
{
    position += glm::conjugate(mOrientation) * delta;
    updateViewMatrix();
}

void Camera::update(float deltaTime)
{
    updated = false;
    if (type == CameraType::firstperson)
    {
        if (moving())
        {
            float moveSpeed = deltaTime * movementSpeed;
            if (keys.up)
                position += getUp() * moveSpeed;
            if (keys.down)
                position -= getUp() * moveSpeed;
            if (keys.left)
                position -= getRight() * moveSpeed;
            if (keys.right)
                position += getRight() * moveSpeed;
            if (keys.forward)
                position += getFront() * moveSpeed;
            if (keys.back)
                position -= getFront() * moveSpeed;
            updateViewMatrix();
        }
    }
}

} // namespace nevk
