//
// Created by cdgira on 7/19/2023.
//

#ifndef VULKANTEST_SIMPLE_RENDER_SYSTEM_HPP
#define VULKANTEST_SIMPLE_RENDER_SYSTEM_HPP

#include "lve_camera.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"

#include <memory>
#include <vector>

namespace lve {
    class SimpleRenderSystem {

    public:
        SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem&) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects, const LveCamera& camera);
    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        LveDevice& lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}

#endif //VULKANTEST_SIMPLE_RENDER_SYSTEM_HPP
