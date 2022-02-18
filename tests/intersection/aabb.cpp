// #include "glm-wrapper.hpp"
// #include "ray.h"
// #undef float4
// #undef float3
// #undef min
// #undef max

// #include <doctest.h>

// TEST_CASE("aabb Intersection")
// {
//     Ray ray{};
//     glm::float3 offset = { 1e-5, 1e-5, 1e-5 };
//     glm::float3 invdir = {}; // captures the sign of r.direction.xyz
//     glm::float3 pmin = {};
//     glm::float3 pmax = {};
//     glm::float3 direction{};
//     glm::float3 origin{};
//     bool res = false;
//     float t = 0.f;

//     // ray intersect point {0, 0, 0}

//     direction = { -1, -1, 4 };
//     origin = { 1, 1, -4 };
//     ray.d = glm::float4(direction, 0.0);
//     ray.o = glm::float4(origin + offset, 0.0);
//     invdir = { 1.0 / ray.d.x, 1.0 / ray.d.y, 1.0 / ray.d.z };

//     // test 1: 0 volume box

//     pmin = { 0.f, 0.f, 0.f };
//     pmax = { 0.f, 0.f, 0.f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == false);

//     pmin = { 0.f, 0.f, 0.f };
//     pmax = { 1.f, 0.f, 0.f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == false);

//     // test 2: plane box 1 intersection
//     // https://www.geogebra.org/m/pwdwzpjr

//     pmin = { -1.f, 0.f, -1.f };
//     pmax = { 1.f, 0.f, 1.f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == true);

//     // test 3: average box
//     // https://www.geogebra.org/m/xmqeemgx
//     pmin = { -1.f, 0.f, -1.f };
//     pmax = { 1.f, 2.f, 1.f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == true);

//     // test 4: tiny box
//     // the same w/ test 3, but smaller box

//     pmin = { -0.003f, 0.f, -0.003f };
//     pmax = { 0.003f, 0.003f, 0.003f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == true);

//     // test 5: angle intersection
//     // https://www.geogebra.org/m/h4rfrggm

//     pmin = { 0.f, 0.f, 0.f };
//     pmax = { 1.f, 0.f, 1.f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == true);

//     // test 6: average parallel ray
//     // https://www.geogebra.org/m/xrtmv4zm
//     direction = { -1, -1, 4 };
//     origin = { 1, -1, -4 };
//     ray.d = glm::float4(direction, 0.0);
//     ray.o = glm::float4(origin + offset, 0.0);
//     invdir = { 1.0 / ray.d.x, 1.0 / ray.d.y, 1.0 / ray.d.z };

//     pmin = { -1.f, 0.f, -1.f };
//     pmax = { 1.f, 0.f, 1.f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == false);

//     // test 7: close to box parallel ray
//     // https://www.geogebra.org/m/zqfz86m2

//     direction = { -2.03, -0.03, 2.03 };
//     origin = { 2.03, -0.03, -2.03 };
//     ray.d = glm::float4(direction, 0.0);
//     ray.o = glm::float4(origin + offset, 0.0);
//     invdir = { 1.0 / ray.d.x, 1.0 / ray.d.y, 1.0 / ray.d.z };

//     pmin = { -1.f, 0.f, -1.f };
//     pmax = { 1.f, 0.f, 1.f };

//     res = intersectRayBox(ray, invdir, pmin, pmax, t);
//     CHECK(res == false);
// }
