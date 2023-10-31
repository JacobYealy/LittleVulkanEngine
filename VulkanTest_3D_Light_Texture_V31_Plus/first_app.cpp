//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "lve_buffer.hpp"
#include <glm/glm.hpp>
#include <array>
#include <chrono>
#include <map>

namespace lve {

    const float X_OFFSET = 1.0f;  // Adjust as needed
    const float Y_OFFSET = 1.0f;  // Adjust as needed

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


    /**
 * @brief Main rendering loop of the application.
 *
 * This function handles:
 * - Initialization of uniform buffers and descriptor sets
 * - Creation and setup of render systems and camera
 * - Main application loop for updating and rendering the scene
 */
    void FirstApp::run() {

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

            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_1) == GLFW_PRESS && !isAnimatingDragon1) {
                isAnimatingDragon1 = true;
                auto& dragon1 = gameObjects.at(DRAGON1_ID);
                dragon1OriginalPosition = dragon1.transform.translation;
                dragon1OriginalRotation = dragon1.transform.rotation;
                dragon1TargetPosition = dragon1OriginalPosition - glm::vec3(X_OFFSET, Y_OFFSET, 0.0f);
                animationProgressDragon1 = 0.0f;
            }


            if (isAnimatingDragon1) {
                auto& dragon1 = gameObjects.at(DRAGON1_ID);
                animationProgressDragon1 += frameTime;

                if (animationProgressDragon1 <= 1.0f) {
                    dragon1.transform.translation = glm::mix(dragon1OriginalPosition, dragon1TargetPosition, animationProgressDragon1);
                } else {
                    dragon1.transform.translation = dragon1TargetPosition;
                    isAnimatingDragon1 = false;

                    // Swap the original and target positions
                    std::swap(dragon1OriginalPosition, dragon1TargetPosition);
                }
            }


            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_2) == GLFW_PRESS && !isAnimatingDragon2) {
                isAnimatingDragon2 = true;
                auto& dragon2 = gameObjects.at(DRAGON2_ID);
                dragon2OriginalPosition = dragon2.transform.translation;
                dragon2OriginalRotation = dragon2.transform.rotation;
                dragon2TargetPosition = dragon2OriginalPosition + glm::vec3(X_OFFSET, Y_OFFSET, 0.0f);
                animationProgressDragon2 = 0.0f;
            }


            if (isAnimatingDragon2) {
                auto& dragon2 = gameObjects.at(DRAGON2_ID);
                animationProgressDragon2 += frameTime;

                if (animationProgressDragon2 <= 1.0f) {
                    dragon2.transform.translation = glm::mix(dragon2OriginalPosition, dragon2TargetPosition, animationProgressDragon2);
                } else {
                    dragon2.transform.translation = dragon2TargetPosition;
                    isAnimatingDragon2 = false;

                    // Swap the original and target positions
                    std::swap(dragon2OriginalPosition, dragon2TargetPosition);
                }
            }





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
                // pointLightSystem.render(frameInfo);  // Transparent lights
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    }


/**
 * Loads all the game objects required for the scene.
 * Describes the scene of dueling dragons.
 * This includes the Dragon, Planet, and Sky models,
 * as well as the configuration for point lights used in the scene.
 */
    void FirstApp::loadGameObjects() {

        // Load the planet model and set its properties
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "../models/venus.obj");
        auto planet = LveGameObject::createGameObject();
        planet.model = lveModel;
        planet.transform.translation = {-25.f, 5.f, -3.5f};
        planet.transform.scale = {1.f, 1.f, 1.f};
        planet.textureBinding = 2;
        gameObjects.emplace(planet.getId(), std::move(planet));

        // Dragon 1
        lveModel = LveModel::createModelFromFile(lveDevice, "../models/dragon.obj");
        auto dragon1 = LveGameObject::createGameObject();
        dragon1.model = lveModel;
        dragon1.transform.translation = {-23.f, 10.f, 2.5f};
        dragon1.transform.scale = {-1.f, -1.f, -1.f};
        dragon1.textureBinding = 1;
        DRAGON1_ID = dragon1.getId();
        gameObjects.emplace(DRAGON1_ID, std::move(dragon1));


        // Dragon 2
        lveModel = LveModel::createModelFromFile(lveDevice, "../models/dragon.obj");
        auto dragon2 = LveGameObject::createGameObject();
        dragon2.model = lveModel;
        dragon2.transform.translation = {-27.f, 0.f, 2.5f};
        dragon2.transform.scale = {1.f, 1.f, -1.f};
        dragon2.textureBinding = 3;
        dragon2.textureBinding = 3;
        DRAGON2_ID = dragon2.getId();
        gameObjects.emplace(DRAGON2_ID, std::move(dragon2));

        // Load the sky model and set its properties
        lveModel = LveModel::createModelFromFile(lveDevice, "../models/sky.obj");
        auto sky = LveGameObject::createGameObject();
        sky.model = lveModel;
        sky.transform.translation = {50.0f, 45.0f, 30.0f};
        sky.transform.scale = {-50.f, -30.f, -30.f};
        sky.textureBinding = 4;
        gameObjects.emplace(sky.getId(),std::move(sky));

        // Define light colors
        std::map<int, glm::vec3> lightColorsMap{
                {0, {.1f, .1f, 1.f}},  // Blue
                {1, {1.f, .1f, .1f}},  // Red
                {2, {0.7f, 0.7f, 0.7f}}   // White
        };

        // Define light positions and their corresponding colors
        std::vector<std::pair<int, glm::vec3>> lightPositionsAndColors = {
                // Blue dragon
                {0, {-27.5f, 6.5f, -0.5f}},  // Chin
                {0, {-26.5f, 7.f, -2.5f}},  // Eye
                {0, {-27.f, 3.f, -2.5f}},   // Tail
                {0, {-28.f, 6.5f, -1.5f}},   // Left arm

                // Red dragon
                {1, {-23.2f, 3.f, -2.5f}},          // Right eye
                {1, {-23.8f, 3.f, -2.f}},          // Left eye
                {1, {-22.f, 4.f, -1.5f}},          // Under chin
                {1, {-24.5f, 2.f, 1.8f}},          // Left arm
                {1, {-23.f, 6.f, 3.f}},           // Tail

                // Planet
                {2, {-26.f, 5.f, -4.5f}},
        };

        // Instantiate the lights with the given positions and colors
        for (const auto& [colorIndex, position] : lightPositionsAndColors) {
            auto pointLight = LveGameObject::makePointLight(3.f);
            pointLight.color = lightColorsMap[colorIndex];
            pointLight.transform.translation = position;
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }


        // For Dragon 1:
        dragon1TargetPosition = dragon1OriginalPosition - glm::vec3(X_OFFSET, Y_OFFSET, 0.0f);

        // For Dragon 2:
        dragon2TargetPosition = dragon2OriginalPosition + glm::vec3(X_OFFSET, Y_OFFSET, 0.0f);

    }
}