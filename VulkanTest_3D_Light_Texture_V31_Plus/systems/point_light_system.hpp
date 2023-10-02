//
// Created by cdgira on 7/19/2023.
//

#ifndef VULKANTEST_POINT_LIGHT_SYSTEM_HPP
#define VULKANTEST_POINT_LIGHT_SYSTEM_HPP

#include "lve_camera.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_frame_info.hpp"

#include <memory>
#include <vector>

namespace lve {
    class PointLightSystem {

    public:
        PointLightSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem &operator=(const PointLightSystem&) = delete;

        void update(FrameInfo &frameInfo, GlobalUbo &ubo);
        void render(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        LveDevice& lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}

#endif //VULKANTEST_POINT_LIGHT_SYSTEM_HPP
