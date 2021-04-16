#pragma once
#include <scene/scene.h>
#include <vulkan/vulkan.h>

#include <resourcemanager.h>
#include <vector>

#include "computepass.h"

namespace nevk
{
class Tonemap : public ComputePass
{
public:
    Tonemap(/* args */);
    ~Tonemap();
};
} // namespace nevk
