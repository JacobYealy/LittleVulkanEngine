#ifndef VULKANTEST_SIMPLE_RENDER_SYSTEM_HPP
#define VULKANTEST_SIMPLE_RENDER_SYSTEM_HPP

#include "lve_camera.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_frame_info.hpp"

#include <memory>
#include <vector>

namespace lve {
    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f}; // Is really just a 3x3 matrix, so we will use the extra values to send data.
        // normalMatrix[3][3] will be the textureBinding;
    };

    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem&) = delete;

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        void renderGameObject(FrameInfo &frameInfo, LveGameObject &gameObject, const glm::mat4 &parentTransform);
        static constexpr uint32_t pushConstantDataSize = sizeof(SimplePushConstantData);

        LveDevice& lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;  // Moved to private section
    };
}

#endif // VULKANTEST_SIMPLE_RENDER_SYSTEM_HPP
