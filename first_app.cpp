//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>

namespace lve {

    FirstApp::FirstApp() {
        loadGameObjects();
    }

    FirstApp::~FirstApp() { }

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects() {
        std::vector<LveModel::Vertex> vertices{
                {{0.0f, -0.5f},{1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f},{0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f},{0.0f, 0.0f, 1.0f}}
        };

        auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);
        auto triangleGameObject = LveGameObject::createGameObject();
        triangleGameObject.model = lveModel;
        triangleGameObject.color = {0.1f, 0.8f, 0.1f};
        triangleGameObject.transform2d.translation.x = 0.2f;
        triangleGameObject.transform2d.scale = {2.0f, 0.5f};
        triangleGameObject.transform2d.rotation = 0.25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangleGameObject));
    }

}

