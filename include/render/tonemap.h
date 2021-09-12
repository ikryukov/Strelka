#pragma once

#include "common.h"
#include "computepass.h"

#include "tonemapparam.h"

#include <vector>

namespace nevk
{
class Tonemap: public ComputePass<Tonemapparam>
{
public:
    Tonemap(const SharedContext& ctx);
    ~Tonemap();
    void initialize();
    void setInputTexture(VkImageView imageView);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
