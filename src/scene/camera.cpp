#include "camera.h"

#include <glm/gtx/quaternion.hpp>

#include <scene/glm-wrapper.hpp>

namespace nevk
{

void Camera::updateViewMatrix()
{
    // save curr to prev
    prevMatrices = matrices;
    // calc new
    glm::mat4 rotM = mat4_cast(mOrientation);
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

void Camera::setPerspective(float _fov, float _aspect, float _znear, float _zfar)
{
    fov = _fov;
    znear = _znear;
    zfar = _zfar;
    matrices.perspective = glm::perspective(glm::radians(fov), _aspect, znear, zfar);
}

glm::float4x4& Camera::getPerspective()
{
    return matrices.perspective;
}

glm::float4x4 Camera::getView()
{
    return matrices.view;
}

void Camera::updateAspectRatio(float aspect)
{
    matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
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
