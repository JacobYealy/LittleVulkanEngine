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
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f}; // Is really just a 3x3 matrix, so we will use the extra values to send data.
        // normalMatrix[3][3] will be the textureBinding;
    };

    // 16 bytes for offset, 12 bytes for color - aligns to 16 bytes.
    // Each new value must end or begin on a 4 byte boundary.
    uint32_t pushConstantDataSize = sizeof(SimplePushConstantData); //16 + 12;

    SimpleRenderSystem::SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

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

    void SimpleRenderSystem::render(FrameInfo &frameInfo) {
        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0, 1,
                &frameInfo.globalDescriptorSet,
                0, nullptr);

        for (auto &kv : frameInfo.gameObjects) {
            auto &gameObject = kv.second;
            if (gameObject.model == nullptr) continue;
            SimplePushConstantData push{};
            push.modelMatrix = gameObject.transform.mat4();
            push.normalMatrix = gameObject.transform.normalMatrix();
            push.normalMatrix[3][3] = static_cast<float>(gameObject.textureBinding); // Not ideal, but limited with 128 bytes.
            vkCmdPushConstants(
                    frameInfo.commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    pushConstantDataSize,
                    &push);
            gameObject.model->bind(frameInfo.commandBuffer);
            gameObject.model->draw(frameInfo.commandBuffer);
        }
    }

}