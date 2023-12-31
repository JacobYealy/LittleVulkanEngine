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

    const float X_OFFSET = 4.0f;  // Adjust as needed
    const float Y_OFFSET = -3.0f;  // Adjust as needed

    FirstApp::FirstApp() {
        // We need to add a pool for the textureImages.
        globalPool = LveDescriptorPool::Builder(lveDevice)
                .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5 * LveSwapChain::MAX_FRAMES_IN_FLIGHT) // Adjusted for 5 textures
                .build();

        loadGameObjects();

        // Ensure dragons' animations are not playing initially
        gameObjects.at(DRAGON1_ID).transform.isPlaying = false;
        gameObjects.at(DRAGON2_ID).transform.isPlaying = false;
    }

    FirstApp::~FirstApp() { }


    /**
 * @brief Main rendering loop of the application.
 *
 * This function is responsible for the core operation of the application. It handles:
 * - Initialization of uniform buffers and descriptor sets to manage the shader inputs.
 * - Setup and maintenance of render systems for drawing the scene and the camera for viewing.
 * - Execution of the main application loop, which includes:
 *   - Processing input events.
 *   - Updating the scene based on user interactions, such as starting animations for dragons on specific key presses.
 *   - Managing the update cycle of all game objects, including dragons, planets, and other entities.
 *   - Performing necessary calculations for camera perspective and aspect ratio.
 *   - Initiating the rendering process for each frame, utilizing Vulkan command buffers.
 *   - Handling frame synchronization and ensuring smooth rendering operations.
 *
 * The function enables interactive animations where the dragons and their child planets can be animated based on user input.
 * It also ensures that all objects in the scene are updated and rendered correctly in each frame.
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
                .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT) // Added 5th texture
                .build();


        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            auto imageInfo1 = textureImage->descriptorImageInfo();
            auto imageInfo2 = terrainTextureImage->descriptorImageInfo();  // Added
            auto imageInfo3 = dinoTextureImage->descriptorImageInfo();      // Added
            auto imageInfo4 = skyTextureImage->descriptorImageInfo();      // Added
            auto imageInfo5 = skyTextureImage->descriptorImageInfo();      // Added
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &imageInfo1)
                    .writeImage(2, &imageInfo2)       // Added
                    .writeImage(3, &imageInfo3)       // Added
                    .writeImage(4, &imageInfo4)       // Added
                    .writeImage(5, &imageInfo4)       // Added
                    .build(globalDescriptorSets[i]);
        }


        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        LveCamera camera{};
        camera.setViewTarget(glm::vec3(0.f, 0.f, -2.5f), glm::vec3(0.f, 5.f, 1.5f));

        auto viewerObject = LveGameObject::createGameObject();


        viewerObject.transform.translation.x = -25.0f; //left or right
        viewerObject.transform.translation.y = 5.0f; // height
        viewerObject.transform.translation.z = -35.0f; //forward backward
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // Update camera and aspect ratio
            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(30.f), aspect, 1.1f, 100.f);

            // Start Dragon 1 animation on button press
            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_1) == GLFW_PRESS) {
                gameObjects.at(DRAGON1_ID).transform.isPlaying = true;
            }

            // Start Dragon 2 animation on button press
            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_2) == GLFW_PRESS) {
                gameObjects.at(DRAGON2_ID).transform.isPlaying = true;
            }

            // Update all game objects
            for (auto& kv : gameObjects) {
                auto& obj = kv.second;
                if (obj.transform.isPlaying) {
                    bool continuePlaying = obj.transform.update(frameTime);
                    if (!continuePlaying) {
                        obj.transform.isPlaying = false; // Stop the animation when it's done
                    }
                }
            }

            aspect = lveRenderer.getAspectRatio();
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
 * This scene features dueling dragons, each with their own set of orbiting planets.
 * The setup includes:
 * - Two dragons, each with unique animations and textures.
 * - A series of rotating planets, some of which are parented to the dragons, creating a dynamic orbit effect.
 * - A background sky model to complete the scene.
 * - Configurations for various point lights to enhance the visual experience.
 *
 * The function handles the following:
 * - Loading and setting up models for the dragons, planets, and sky.
 * - Configuring animations for the dragons and the rotating planets.
 * - Assigning textures and parenting certain planets to the dragons.
 * - Setting up point lights with specified positions and colors.
 */
    void FirstApp::loadGameObjects() {

        // Load the planet model and set its properties
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "../models/venus.obj");
        LveGameObject planet = LveGameObject::createGameObject();
        planet.model = lveModel;
        planet.transform.isPlaying = true;
        planet.transform.translation = {-25.f, 5.f, -3.5f};
        planet.transform.scale = {1.f, 1.f, 1.f};
        planet.textureBinding = 2;

        // Planet animation setup
        planet.transform.animationSequence = {
                {
                        // Start frame
                        {planet.transform.translation, glm::vec3(0.0f), planet.transform.scale, 0.0f},
                        // End frame - full rotation over 10 seconds
                        {planet.transform.translation, glm::vec3(0.0f, 0.0f, glm::two_pi<float>()), planet.transform.scale, 10.0f},
                },
                100.0f // Duration of the animation in seconds
        };

        PLANET_ID = planet.getId();
        gameObjects.emplace(PLANET_ID, std::move(planet));

        // Dragon 1
        lveModel = LveModel::createModelFromFile(lveDevice, "../models/dragon.obj");
        LveGameObject dragon1 = LveGameObject::createGameObject();
        dragon1.model = lveModel;
        dragon1.transform.translation = {-23.f, 10.f, 2.5f};
        dragon1.transform.scale = {-1.f, -1.f, -1.f};
        dragon1.textureBinding = 1;
        // Dragon 1 animation setup
        dragon1.transform.animationSequence = {
                {
                        // Start frame
                        {dragon1.transform.translation, glm::vec3(0.0f), dragon1.transform.scale, 0.0f},
                        // Mid frame - Dragon 1 moves
                        {dragon1.transform.translation - glm::vec3(4.0f, -3.0f, 7.0f), glm::vec3(0.0f), glm::vec3(-dragon1.transform.scale.x, dragon1.transform.scale.y, dragon1.transform.scale.z), 1.0f},
                        // End frame - Dragon 1 returns to start
                        {dragon1.transform.translation, glm::vec3(0.0f), dragon1.transform.scale, 2.0f},
                },
                2.0f // Duration of the animation in seconds
        };
        DRAGON1_ID = dragon1.getId();
        gameObjects.emplace(DRAGON1_ID, std::move(dragon1));

        // Dragon 2
        LveGameObject dragon2 = LveGameObject::createGameObject();
        dragon2.model = lveModel; // Reuse the model loaded for dragon 1
        dragon2.transform.translation = {-27.f, 0.f, 2.5f};
        dragon2.transform.scale = {1.f, 1.f, -1.f};
        dragon2.textureBinding = 3;
        // Dragon 2 animation setup
        dragon2.transform.animationSequence = {
                {
                        // Start frame
                        {dragon2.transform.translation, glm::vec3(0.0f), dragon2.transform.scale, 0.0f},
                        // Mid frame - Dragon 2 moves
                        {dragon2.transform.translation + glm::vec3(X_OFFSET, Y_OFFSET, -7.0f), glm::vec3(0.0f), glm::vec3(-dragon2.transform.scale.x, dragon2.transform.scale.y, dragon2.transform.scale.z), 1.0f},
                        // End frame - Dragon 2 returns to start
                        {dragon2.transform.translation, glm::vec3(0.0f), dragon2.transform.scale, 2.0f},
                },
                2.0f // Duration of the animation in seconds
        };
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

        // Function to create and configure a planet
        auto createPlanet = [&](float x, float y, float z, int textureBind, float animDuration, TransformComponent* parentTransform) {
            std::shared_ptr<LveModel> lveModelPlanet = LveModel::createModelFromFile(lveDevice, "../models/venus.obj");
            LveGameObject planet = LveGameObject::createGameObject();
            planet.model = lveModelPlanet;
            planet.transform.translation = {x, y, z};
            planet.transform.scale = {1.f, 1.f, 1.f};
            planet.textureBinding = textureBind;
            planet.transform.parent = parentTransform; // Set the parent's transform

            // Planet animation setup
            planet.transform.animationSequence = {
                    {
                            // Start frame
                            {planet.transform.translation, glm::vec3(0.0f), planet.transform.scale, 0.1f},
                            // End frame - full rotation over animDuration seconds
                            {planet.transform.translation, glm::vec3(0.0f, 0.0f, glm::two_pi<float>()), planet.transform.scale, animDuration},
                    },
                    animDuration * 100 // Duration of the animation in seconds
            };

            return planet;
        };

        // Create and add four new planets as children of the dragons
        std::vector<LveGameObject> planets;
        planets.push_back(createPlanet(-5.f, 8.f, 0.f, 2, 12.0f, &gameObjects.at(DRAGON1_ID).transform));    // Top right
        planets.push_back(createPlanet(-5.f, 2.f, -5.f, 2, 8.0f, &gameObjects.at(DRAGON1_ID).transform));    // New planet 2
        planets.push_back(createPlanet(-5.f, 6.f, 2.f, 2, 9.0f, &gameObjects.at(DRAGON1_ID).transform));    // New planet 3
        planets.push_back(createPlanet(-5.f, 10.f, 3.f, 2, 14.0f, &gameObjects.at(DRAGON2_ID).transform));   // New planet 4
        planets.push_back(createPlanet(-5.f, 6.f, 7.f, 2, 12.0f, &gameObjects.at(DRAGON2_ID).transform));    // New planet 5
        planets.push_back(createPlanet(-5.f, 4.f, 10.f, 2, 9.0f, &gameObjects.at(DRAGON2_ID).transform));    // New planet 6

        for (auto& planet : planets) {
            auto planetId = planet.getId();
            gameObjects.emplace(planetId, std::move(planet));
        }


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
            auto pointLight = LveGameObject::makePointLight(50.f);
            pointLight.color = lightColorsMap[colorIndex];
            pointLight.transform.translation = position;
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }


    }
}