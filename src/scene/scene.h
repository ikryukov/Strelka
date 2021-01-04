#ifndef NEVK_SCENE_H
#define NEVK_SCENE_H

#include <vector>
#include <cstdint>
#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>


typedef struct float2
{
    float x;
    float y;
} float2;

typedef struct float3
{
    float x;
    float y;
    float z;
} float3;

typedef struct float4
{
    float x;
    float y;
    float z;
    float w;
} float4;

struct Vertex
{
    glm::vec4 pos;
    glm::vec2 uv;
    glm::vec3 normal;
};


struct Mesh
{
    uint32_t mIndex;  //  ind of 1st vertex
    uint32_t mCount;  // amount of vertices
};

struct Material
{
    glm::vec4 color;
};

struct Instance
{
//    Instance* parent;
//    std::vector<Instance> children;
//    std::string name;
    struct Mesh& mesh_id;  // !!
    glm::mat4 transform;
    struct Material material; // !!
};


class Scene
{
private:
    Scene();
//    Camera camera;
    std::vector<struct Vertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<struct Mesh> mesh;
    std::vector<struct Material> material;
    std::vector<struct Instance> instance;
    float4 light;

public:
    void update_mesh();
    void add_mesh();
    void add();     // instance, mesh, material
    void remove();  // instance, mesh, material
//    void update_camera();
    ~Scene();
};


#endif //NEVK_SCENE_H