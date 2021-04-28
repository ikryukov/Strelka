#include <debugUtils.h>
#include <vulkan/vulkan.h>

void nevk::debug::beginLabel(VkCommandBuffer cmdBuffer, const char* labelName, const glm::float4& color)
{
    VkDebugUtilsLabelEXT label = {};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName = labelName;
    label.color[0] = color.r;
    label.color[1] = color.g;
    label.color[2] = color.b;
    label.color[3] = color.a;
    vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &label);
}

void nevk::debug::endLabel(VkCommandBuffer cmdBuffer)
{
    vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
}
