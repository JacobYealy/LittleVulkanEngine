//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"
#include <map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <iostream>
#include <array>

namespace lve {

    struct SimplePushConstantData {
        glm::vec2 offset;
        alignas(16) glm::vec3 color;

    };

    // 16 bytes for offset, 12 bytes for color - aligns to 16 bytes.
    // Each new value must end or begin on a 4 byte boundary.
    uint32_t pushConstantDataSize = sizeof(SimplePushConstantData); //16 + 12;

    FirstApp::FirstApp() {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        recreateSwapChain();
        createCommandBuffers();
    }

    FirstApp::~FirstApp() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::run() {
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    /**
 * Adds a "pixel" (actually a square made up of two triangles) to the vertices array.
 * The pixel's position on the grid is determined by (i, j), and its color is specified.
 *
 * @param i         The i-th row position of the pixel in the grid.
 * @param j         The j-th column position of the pixel in the grid.
 * @param color     The color of the pixel in RGB format.
 * @param vertices  The vector of vertices to which the pixel's vertices will be added.
 */
    void addPixel(float i, float j, glm::vec3 color, std::vector<LveModel::Vertex>& vertices) {
        float x_offset = i * 0.25f;
        float y_offset = j * 0.25f;
        vertices.push_back({{-1.00f + x_offset, -1.00f + y_offset}, color});
        vertices.push_back({{-0.75f + x_offset, -1.00f + y_offset}, color});
        vertices.push_back({{-0.75f + x_offset, -0.75f + y_offset}, color});

        vertices.push_back({{-1.00f + x_offset, -1.00f + y_offset}, color});
        vertices.push_back({{-1.00f + x_offset, -0.75f + y_offset}, color});
        vertices.push_back({{-0.75f + x_offset, -0.75f + y_offset}, color});
    }


    /**
 * Populates a grid of "pixels" by specifying their colors and locations.
 * The grid is represented by a 2D array, where each integer corresponds to a color.
 * A map is used to look up the RGB color values corresponding to these integers (from instructions).
 * The color and position information for each pixel is then added to the vertices vector.
 * Finally, a unique pointer to the LveModel is created to contain these vertices.
 */
    void FirstApp::loadModels() {
        std::vector<LveModel::Vertex> vertices;

        std::map<int, glm::vec3> colorLookup = {
                {1, {0.0f, 1.0f, 1.0f}},     // Background
                {2, {0.05f, 0.27f, 0.64f}},  // Dark Blue
                {3, {0.3f, 0.3f, 0.3f}},     // Darker Gray
                {4, {1.0f, 1.0f, 1.0f}},     // White
                {5, {0.4f, 0.1f, 0.1f}},     // Brown
                {6, {0.0f, 0.0f, 0.0f}},     // Black
                {7, {0.04f, 0.18f, 0.44f}},  // Darker Blue 1
                {8, {0.03f, 0.13f, 0.34f}},   // Darker Blue 2
                {9, {0.82f, 0.71f, 0.55f}}   // Skin Tone
        };

        int colorGrid[8][8] = {{1, 1, 8, 1, 1, 1, 1, 1},
                               {1, 8, 7, 2, 1, 1, 5, 1},
                               {8, 7, 7, 7, 2, 2, 5, 1},
                               {1, 3, 6, 9, 6, 1, 5, 1},
                               {1, 3, 4, 4, 4, 1, 5, 1},
                               {1, 8, 4, 4, 8, 9, 5, 1},
                               {9, 7, 4, 8, 7, 1, 5, 1},
                               {8, 8, 8, 7, 7, 1, 5, 1}};


        for(float i = 0.0; i < 8.0; i++) {
            for(float j = 0.0; j < 8.0; j++) {
                glm::vec3 color = colorLookup[colorGrid[int(j)][int(i)]];
                addPixel(i, j, color, vertices);
            }
        }

        lveModel = std::make_unique<LveModel>(lveDevice, vertices);
    }


    void FirstApp::createPipelineLayout() {
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

    void FirstApp::createPipeline() {
        assert (lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
        assert (pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.renderPass = lveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
                lveDevice,
                "../shaders/simple_shader.vert.spv",
                "../shaders/simple_shader.frag.spv",
                pipelineConfig
        );
    }

    void FirstApp::recreateSwapChain() {
        auto extent = lveWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = lveWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(lveDevice.device());

        if (lveSwapChain == nullptr) {
            lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
        } else {
            std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
            lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
            if (lveSwapChain->imageCount() != commandBuffers.size()) {
                if (commandBuffers.size() > 0)
                    freeCommandBuffers();
                createCommandBuffers();
            }
        }

        createPipeline();
    }

    void FirstApp::createCommandBuffers() {

        commandBuffers.resize(lveSwapChain->imageCount());

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = lveDevice.getCommandPool();
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(lveDevice.device(), &commandBufferAllocateInfo, commandBuffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

    }

    void FirstApp::freeCommandBuffers() {
        vkFreeCommandBuffers(
                lveDevice.device(),
                lveDevice.getCommandPool(),
                static_cast<uint32_t>(commandBuffers.size()),
                commandBuffers.data()
        );
        commandBuffers.clear();
    }

    void FirstApp::recordCommandBuffer(int imageIndex) {
        static int frame = 0;
        frame = (frame + 1) % 1000;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);


        lvePipeline->bind(commandBuffers[imageIndex]);
        lveModel->bind(commandBuffers[imageIndex]);

        for (int j=0;j<4;j++) {
            SimplePushConstantData push{};
            push.offset = {-0.5f+frame*0.001f, -0.4f + 0.25f*j};
            push.color = {0.0f, 0.0f, 0.2f+0.2f*j};

            vkCmdPushConstants(
                commandBuffers[imageIndex],
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                pushConstantDataSize,
                &push
            );
            lveModel->draw(commandBuffers[imageIndex]);
        }

        vkCmdEndRenderPass(commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void FirstApp::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        recordCommandBuffer(imageIndex);
        result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

}

