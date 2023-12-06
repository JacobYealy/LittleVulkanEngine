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

    // Constructor
    SimpleRenderSystem::SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass,
                                           VkDescriptorSetLayout globalSetLayout)
            : lveDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    // Destructor
    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    // Create pipeline layout
    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = pushConstantDataSize;

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    // Create pipeline
    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

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

    // Render method
    void SimpleRenderSystem::render(FrameInfo &frameInfo) {
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0, 1,
                &frameInfo.globalDescriptorSet,
                0, nullptr);

        for (auto &kv: frameInfo.gameObjects) {
            renderGameObject(frameInfo, kv.second, glm::mat4(1.0f));
        }
    }

    void SimpleRenderSystem::renderGameObject(FrameInfo &frameInfo, LveGameObject &gameObject, const glm::mat4 &parentTransform) {
        if (gameObject.model == nullptr) return;

        SimplePushConstantData push{};
        push.modelMatrix = gameObject.transform.mat4(parentTransform);
        push.normalMatrix = gameObject.transform.normalMatrix();
        push.normalMatrix[3][3] = static_cast<float>(gameObject.textureBinding);

        vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                pushConstantDataSize,
                &push);

        gameObject.model->bind(frameInfo.commandBuffer);
        gameObject.model->draw(frameInfo.commandBuffer);

        for (auto &child : gameObject.getChildren()) {
            renderGameObject(frameInfo, *child, push.modelMatrix);
        }
    }
}