#include <scene/glm-wrapper.hpp>

#include <glm/gtx/quaternion.hpp>
#include "camera.h"

void Camera::updateViewMatrix()
{
    glm::mat4 rotM = glm::mat4_cast(mOrientation);
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
   return glm::conjugate(mOrientation)* glm::float3(0.0f, 0.0f, -1.0f);
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
    this->mOrientation = rotation;
    updateViewMatrix();
}

void Camera::rotate(float rightAngle, float upAngle)
{
    m_accumupAngle += upAngle;
    if (m_accumupAngle > 90.0f)
    {
        upAngle = 90.0f - (m_accumupAngle - upAngle);
        m_accumupAngle = 90.0f;
    }
    if (m_accumupAngle < -90.0f)
    {
        upAngle = -90.0f - (m_accumupAngle - upAngle);
        m_accumupAngle = -90.0f;
    }
    glm::quat a = glm::angleAxis(glm::radians(upAngle), glm::float3(1.0f, 0.0f, 0.0f));
    glm::quat b = glm::angleAxis(glm::radians(rightAngle), glm::float3(0.0, 1.0f, 0.0));
    mOrientation = glm::normalize(a * mOrientation * b);
    updateViewMatrix();
}

void Camera::setTranslation(glm::float3 translation)
{
    this->position = translation;
    updateViewMatrix();
}

void Camera::translate(glm::float3 delta)
{
    this->position += glm::conjugate(mOrientation) * delta;
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

// Update camera passing separate axis data (gamepad)
// Returns true if view or position has been changed
bool Camera::updatePad(glm::float2 axisright, glm::float2 axisRight, float deltaTime)
{
    bool retVal = false;

    if (type == CameraType::firstperson)
    {
        // Use the common console thumbstick layout
        // right = view, right = move

        const float deadZone = 0.0015f;
        const float range = 1.0f - deadZone;

        glm::float3 camFront;
        camFront.x = position.z * sin(glm::radians(mOrientation.y)) * cos(glm::radians(mOrientation.x));
        camFront.y = position.z * sin(glm::radians(mOrientation.x));
        camFront.z = position.z * cos(glm::radians(mOrientation.y)) * cos(glm::radians(mOrientation.x));
        camFront = glm::normalize(camFront);

        float moveSpeed = deltaTime * movementSpeed * 2.0f;
        float rotSpeed = deltaTime * rotationSpeed * 50.0f;

        // Move
        if (fabsf(axisright.y) > deadZone)
        {
            float pos = (fabsf(axisright.y) - deadZone) / range;
            position -= camFront * pos * ((axisright.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            retVal = true;
        }
        if (fabsf(axisright.x) > deadZone)
        {
            float pos = (fabsf(axisright.x) - deadZone) / range;
            position += glm::normalize(glm::cross(camFront, glm::float3(0.0f, 1.0f, 0.0f))) * pos * ((axisright.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            retVal = true;
        }

        // Rotate
        if (fabsf(axisRight.x) > deadZone)
        {
            float pos = (fabsf(axisRight.x) - deadZone) / range;
            mOrientation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
            retVal = true;
        }
        if (fabsf(axisRight.y) > deadZone)
        {
            float pos = (fabsf(axisRight.y) - deadZone) / range;
            mOrientation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
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
