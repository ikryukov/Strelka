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
    CameraType type = CameraType::lookat;

    glm::float3 rotation = glm::float3();
    glm::float3 position = glm::float3();

    float rotationSpeed = 1.0f;
    float movementSpeed = 1.0f;

    bool updated = false;

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
    } keys;

    bool moving();
    float getNearClip();
    float getFarClip();
    void setPerspective(float fov, float aspect, float znear, float zfar);
    void updateAspectRatio(float aspect);
    void setPosition(glm::float3 position);
    void setRotation(glm::float3 rotation);
    void rotate(glm::float3 delta);
    void setTranslation(glm::float3 translation);
    void translate(glm::float3 delta);
    void update(float deltaTime);
    bool updatePad(glm::float2 axisLeft, glm::float2 axisRight, float deltaTime);
};
