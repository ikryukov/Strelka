#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    float fov;
    float znear, zfar;

    void updateViewMatrix();

public:
    enum CameraType
    {
        lookat,
        firstperson
    };
    CameraType type = CameraType::firstperson;

    glm::quat rotation;
    glm::float3 position;

    float  m_accumupAngle;

    float rotationSpeed;
    float movementSpeed;

    bool updated = false;
    glm::float3 directionVector;

    struct MouseButtons
    {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

    glm::float2 mousePos;

    struct
    {
        glm::float4x4 perspective;
        glm::float4x4 view;
    } matrices;
       

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool forward = false;
        bool back = false;
    } keys;

    bool moving();
    float getNearClip();
    float getFarClip();
    void setPerspective(float fov, float aspect, float znear, float zfar);
    void updateAspectRatio(float aspect);
    void setPosition(glm::float3 position);
    void setRotation(glm::quat rotation);
    void rotate(float, float);
    void setTranslation(glm::float3 translation);
    void translate(glm::float3 delta);
    void update(float deltaTime);
    bool updatePad(glm::float2 axisLeft, glm::float2 axisRight, float deltaTime);
};
