//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "lve_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>
#include <chrono>
#include <map>

namespace lve {

    FirstApp::FirstApp() {
        // We need to add a pool for the textureImages.
        globalPool = LveDescriptorPool::Builder(lveDevice)
                .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 * LveSwapChain::MAX_FRAMES_IN_FLIGHT) // Adjusted for 3 textures
                .build();

        loadGameObjects();
        // Texture Image loaded in first_app.hpp file.
    }

    FirstApp::~FirstApp() { }

    void FirstApp::run() {

        // The GlobalUbo is a fixed size so we can setup this up here then load in the data later.
        // So I need to load in the images first.
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i=0;i<uboBuffers.size();i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                    lveDevice,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT) // Added 2nd texture
                .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT) // Added 3rd texture
                .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT) // Added 4th texture
                .build();

        // Need to see if anything needs to be done here for the texture maps.
        // Something isn't beting setup right for the Image Info, information is not getting freed correctly.
        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            auto imageInfo1 = textureImage->descriptorImageInfo();
            auto imageInfo2 = terrainTextureImage->descriptorImageInfo();  // Added
            auto imageInfo3 = dinoTextureImage->descriptorImageInfo();      // Added
            auto imageInfo4 = skyTextureImage->descriptorImageInfo();      // Added
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &imageInfo1)
                    .writeImage(2, &imageInfo2)       // Added
                    .writeImage(3, &imageInfo3)       // Added
                    .writeImage(4, &imageInfo4)       // Added
                    .build(globalDescriptorSets[i]);
        }


        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        LveCamera camera{};
        camera.setViewTarget(glm::vec3(0.f, 0.f, -2.5f), glm::vec3(0.f, 5.f, 1.5f));

        auto viewerObject = LveGameObject::createGameObject();


        viewerObject.transform.translation.x = -25.0f; //left or right
        viewerObject.transform.translation.y = 5.0f; // height
        viewerObject.transform.translation.z = -15.0f; //forward backward
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(30.f), aspect, 1.1f, 100.f);
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer,camera, globalDescriptorSets[frameIndex], gameObjects};
                //update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                //render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.render(frameInfo); // Solid Objects
                pointLightSystem.render(frameInfo);  // Transparent Objects
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    }

    /*
     * loadGameObjects()
     *
     */
    void FirstApp::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "../models/venus.obj");
        auto planet = LveGameObject::createGameObject();
        planet.model = lveModel;
        planet.transform.translation = {-25.f, 5.f, -3.5f}; //-5 from dragon Z
        planet.transform.scale = {1.f, 1.f, 1.f};
        planet.textureBinding = 2;
        gameObjects.emplace(planet.getId(), std::move(planet));

        lveModel = LveModel::createModelFromFile(lveDevice, "../models/dragon.obj");
        auto dragon = LveGameObject::createGameObject();
        dragon.model = lveModel;
        dragon.transform.translation = {-23.f, 10.f, 2.5f};
        dragon.transform.scale = {-1.f, -1.f, -1.f};
        dragon.textureBinding = 1;
        gameObjects.emplace(dragon.getId(),std::move(dragon));


        lveModel = LveModel::createModelFromFile(lveDevice, "../models/dragon.obj");
        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = {-27.f, 0.f, 2.5f};
        floor.transform.scale = {1.f, 1.f, -1.f};
        floor.textureBinding = 3;
        gameObjects.emplace(floor.getId(),std::move(floor));

        lveModel = LveModel::createModelFromFile(lveDevice, "../models/sky.obj");
        auto sky = LveGameObject::createGameObject();
        sky.model = lveModel;
        sky.transform.translation = {70.0f, 45.0f, 30.0f};
        sky.transform.scale = {-50.f, -30.f, -30.f};
        sky.textureBinding = 4;
        gameObjects.emplace(sky.getId(),std::move(sky));


        std::map<int, glm::vec3> lightColorsMap{
                {0, {.1f, .1f, 1.f}},  // Blue
                {1, {1.f, .1f, .1f}},   // Red
                {2, {0.7f, 0.7f, 0.7f}}   // White
        };

        // lower Y = higher up, X = CLOSER or FURTHER, Z = LEFT to RIGHT
        std::vector<std::pair<int, glm::vec3>> lightPositionsAndColors = {
                {0, {-27.5f, 6.5f, -0.5f}},  // BLUE chin
                {0, {-26.5f, 7.f, -2.5f}},  // Blue eye
                {1, {-23.2f, 3.f, -2.5f}},          // Right eyeball red dragon
                {1, {-23.8f, 3.f, -2.f}},          // Left eyeball red
                {1, {-22.f, 4.f, -1.5f}},          // Under chin RED
                {1, {-24.5f, 2.f, 1.8f}},          // Left arm RED
                {1, {-23.f, 6.f, 3.f}},           // Red light below the dragon
                {2, {-26.f, 5.f, -4.5f}},   // Planet
                {0, {-27.f, 3.f, -2.5f}},   // Blue dragon tail
                {0, {-28.f, 6.5f, -1.5f}},   // Blue dragon tail
        };

        for (const auto& [colorIndex, position] : lightPositionsAndColors) {
            auto pointLight = LveGameObject::makePointLight(2.f);
            pointLight.color = lightColorsMap[colorIndex];
            pointLight.transform.translation = position;
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

    }
}