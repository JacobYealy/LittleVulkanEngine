//
// Created by cdgira on 7/19/2023.
//
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>

namespace lve {

    struct SimplePushConstantData {
        glm::mat2 transform{1.f};  // 2D transformation matrix
        glm::vec2 offset;          // Offset for the 2D transformation
        alignas(16) glm::vec3 color;  // Color data, aligned to 16 bytes

    };

    // 16 bytes for offset, 12 bytes for color - aligns to 16 bytes.
    // Each new value must end or begin on a 4 byte boundary.
    uint32_t pushConstantDataSize = sizeof(SimplePushConstantData); //16 + 12;

    SimpleRenderSystem::SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass) : lveDevice{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = pushConstantDataSize;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pSetLayouts = nullptr;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert (pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
                lveDevice,
                "../shaders/simple_shader.vert.spv",
                "../shaders/simple_shader.frag.spv",
                pipelineConfig
        );
    }

    void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject> &gameObjects) {
        /***
        * Render the game objects to the command buffer.
        * This function iterates over each game object, calculates their transformation and color information,
        * pushes these constants to the graphics pipeline, binds the model associated with each game object,
        * and then issues a draw command to the Vulkan command buffer.
        *
        * Parameters:
        *   - commandBuffer: The Vulkan command buffer to which the render commands are recorded.
        *   - gameObjects: A vector containing all the game objects to be rendered.
        */
        lvePipeline->bind(commandBuffer);

        for (auto &gameObject : gameObjects) {

            // If clockwise
            if (gameObject.clockwise) {
                // Rotate clockwise and scale down
                gameObject.transform2d.rotation = glm::mod(gameObject.transform2d.rotation + 0.001f, glm::two_pi<float>());
                gameObject.transform2d.scale -= glm::vec2(0.0001f); //edit speed

                // Check if the scale is less than or equal to 0.5
                if (gameObject.transform2d.scale.x <= 0.5f) {
                    // Reverse the direction
                    gameObject.clockwise = false;
                }
            }
                // If counter-clockwise
            else {
                // Rotate counter-clockwise and scale up
                gameObject.transform2d.rotation = glm::mod(gameObject.transform2d.rotation - 0.001f, glm::two_pi<float>());
                gameObject.transform2d.scale += glm::vec2(0.0001f); //edit speed

                // Check if the scale is greater than or equal to 1.0
                if (gameObject.transform2d.scale.x >= 1.0f) {
                    // Reverse the direction
                    gameObject.clockwise = true;
                }
            }

            SimplePushConstantData push{};
            push.offset = gameObject.transform2d.translation;
            push.color = gameObject.color;
            push.transform = gameObject.transform2d.mat2();

            // Push the constants to the command buffer
            vkCmdPushConstants(
                    commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    pushConstantDataSize,
                    &push);
            gameObject.model->bind(commandBuffer);
            gameObject.model->draw(commandBuffer);
        }
    }

}
