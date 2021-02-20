#include <scene/glm-wrapper.hpp>
#include "camera.h"

void Camera::updateViewMatrix()
{

    glm::float4x4 transM;

    /*glm::quat qPitch = glm::angleAxis(rotation.x, glm::float3(1.0f, 0.0f, 0.0f));
    glm::quat qYaw = glm::angleAxis(rotation.y, glm::float3(0.0f, 1.0f, 0.0f));*/
    glm::quat q = rotation;
    q.x *= -1.0f;
    q.y *= -1.0f;
    q.z *= 0.0f;
    glm::mat4 rotM = mat4_cast(q);

    transM = glm::translate(glm::float4x4(1.0f), position * glm::float3(1.0f, 1.0f, -1.0f));

    if (type == CameraType::firstperson)
    {
        matrices.view = transM * rotM;
    }
    else
    {
        matrices.view = transM * rotM;
    }
    updated = true;
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

void Camera::setPerspective(float fov, float aspect, float znear, float zfar)
{
    this->fov = fov;
    this->znear = znear;
    this->zfar = zfar;
    matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
}

void Camera::updateAspectRatio(float aspect)
{
    matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
}

void Camera::setPosition(glm::float3 position)
{
    this->position = position;
    updateViewMatrix();
}

void Camera::setRotation(glm::quat rotation)
{
    this->rotation = rotation;
    updateViewMatrix();
}

void Camera::rotate(float upAngle, float rightAngle)
{
    rotation *= glm::angleAxis(glm::radians(rightAngle), glm::float3(matrices.view[1]));
    rotation *= glm::angleAxis(glm::radians(upAngle), glm::float3(matrices.view[0]));
    rotation = glm::normalize(rotation);
    updateViewMatrix();
}

void Camera::setTranslation(glm::float3 translation)
{
    this->position = translation;
    updateViewMatrix();
}

void Camera::translate(glm::float3 delta)
{
    this->position += delta;
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
                position += glm::float3(matrices.view[1]) * moveSpeed;
            if (keys.down)
                position -= glm::float3(matrices.view[1]) * moveSpeed;
            if (keys.left)
              position += glm::float3(matrices.view[0]) * moveSpeed;
            if (keys.right)
                position -= glm::float3(matrices.view[0]) * moveSpeed;
             if (keys.forward)
                position -= glm::float3(matrices.view[2]) * moveSpeed;
            if (keys.back)
                position += glm::float3(matrices.view[2]) * moveSpeed;
            updateViewMatrix();
        }
    }
}

// Update camera passing separate axis data (gamepad)
// Returns true if view or position has been changed
bool Camera::updatePad(glm::float2 axisLeft, glm::float2 axisRight, float deltaTime)
{
    bool retVal = false;

    if (type == CameraType::firstperson)
    {
        // Use the common console thumbstick layout
        // Left = view, right = move

        const float deadZone = 0.0015f;
        const float range = 1.0f - deadZone;

        glm::float3 camFront;
        camFront.x = position.z * sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
        camFront.y = position.z * sin(glm::radians(rotation.x));
        camFront.z = position.z * cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
        camFront = glm::normalize(camFront);

        float moveSpeed = deltaTime * movementSpeed * 2.0f;
        float rotSpeed = deltaTime * rotationSpeed * 50.0f;

        // Move
        if (fabsf(axisLeft.y) > deadZone)
        {
            float pos = (fabsf(axisLeft.y) - deadZone) / range;
            position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            retVal = true;
        }
        if (fabsf(axisLeft.x) > deadZone)
        {
            float pos = (fabsf(axisLeft.x) - deadZone) / range;
            position += glm::normalize(glm::cross(camFront, glm::float3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            retVal = true;
        }

        // Rotate
        if (fabsf(axisRight.x) > deadZone)
        {
            float pos = (fabsf(axisRight.x) - deadZone) / range;
            rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
            retVal = true;
        }
        if (fabsf(axisRight.y) > deadZone)
        {
            float pos = (fabsf(axisRight.y) - deadZone) / range;
            rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
            retVal = true;
        }
    }
    else
    {
        // todo: move code from example base class for look-at
    }

    if (retVal)
    {
        updateViewMatrix();
    }
    return retVal;
}
