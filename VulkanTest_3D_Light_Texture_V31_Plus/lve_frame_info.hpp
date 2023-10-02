//
// Created by cdgira on 8/1/2023.
//

#ifndef VULKANTEST_LVE_FRAME_INFO_HPP
#define VULKANTEST_LVE_FRAME_INFO_HPP

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

#include <vulkan/vulkan.h>

namespace lve {

    #define MAX_LIGHTS 10

    struct PointLight {
        glm::vec4 position{};
        glm::vec4 color{};
    };

    struct GlobalUbo {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f}; //camera info
        glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        LveCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        LveGameObject::Map &gameObjects;
    };
}

#endif //VULKANTEST_LVE_FRAME_INFO_HPP
