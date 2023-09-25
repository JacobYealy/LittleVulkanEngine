//
// Created by cdgira on 6/30/2023.
//
#include <memory>
#include <chrono>
#include <map>
#include "lve_model.hpp"
#include "first_app.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"

namespace lve {
    FirstApp::FirstApp() {
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        LveCamera camera{};

        // Initialize the viewerObject's position
        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation = {0.0f, -5.0f, 2.5f};

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
            camera.setPerspectiveProjection(glm::radians(100.f), aspect, 0.1f, 10.f);
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    std::unique_ptr<LveModel> createColoredCubeModel(LveDevice &device, glm::vec3 color) {
        std::vector<LveModel::Vertex> vertices{
                // left face
                {{-.5f, -.5f, -.5f},  color},
                {{-.5f, .5f,  .5f},   color},
                {{-.5f, -.5f, .5f},   color},
                {{-.5f, -.5f, -.5f},  color},
                {{-.5f, .5f,  -.5f},  color},
                {{-.5f, .5f,  .5f},   color},

                // right face
                {{.5f,  -.5f, -.5f},  color},
                {{.5f,  .5f,  .5f},   color},
                {{.5f,  -.5f, .5f},   color},
                {{.5f,  -.5f, -.5f},  color},
                {{.5f,  .5f,  -.5f},  color},
                {{.5f,  .5f,  .5f},   color},

                // top face
                {{-.5f, -.5f, -.5f},  color},
                {{.5f,  -.5f, .5f},   color},
                {{-.5f, -.5f, .5f},   color},
                {{-.5f, -.5f, -.5f},  color},
                {{.5f,  -.5f, -.5f},  color},
                {{.5f,  -.5f, .5f},   color},

                // bottom face
                {{-.5f, .5f,  -.5f},  color},
                {{.5f,  .5f,  .5f},   color},
                {{-.5f, .5f,  .5f},   color},
                {{-.5f, .5f,  -.5f},  color},
                {{.5f,  .5f,  -.5f},  color},
                {{.5f,  .5f,  .5f},   color},

                // nose face
                {{-.5f, -.5f, 0.5f},  color},
                {{.5f,  .5f,  0.5f},  color},
                {{-.5f, .5f,  0.5f},  color},
                {{-.5f, -.5f, 0.5f},  color},
                {{.5f,  -.5f, 0.5f},  color},
                {{.5f,  .5f,  0.5f},  color},

                // tail face
                {{-.5f, -.5f, -0.5f}, color},
                {{.5f,  .5f,  -0.5f}, color},
                {{-.5f, .5f,  -0.5f}, color},
                {{-.5f, -.5f, -0.5f}, color},
                {{.5f,  -.5f, -0.5f}, color},
                {{.5f,  .5f,  -0.5f}, color},
        };

        // Create and return the model
        return std::make_unique<LveModel>(device, vertices);
    }

    void FirstApp::loadGameObjects() {
        std::map<int, glm::vec3> colorLookup = {
                {1, {0.0f,  1.0f,  1.0f}},
                {2, {0.05f, 0.27f, 0.64f}},
                {3, {0.3f,  0.3f,  0.3f}},
                {4, {1.0f,  1.0f,  1.0f}},
                {5, {0.4f,  0.1f,  0.1f}},
                {6, {0.0f,  0.0f,  0.0f}},
                {7, {0.04f, 0.18f, 0.44f}},
                {8, {0.03f, 0.13f, 0.34f}},
                {9, {0.82f, 0.71f, 0.55f}}
        };

        int colorGrid[8][8] = {
                {1, 1, 8, 1, 1, 1, 1, 1},
                {1, 8, 7, 2, 1, 1, 5, 1},
                {8, 7, 7, 7, 2, 2, 5, 1},
                {1, 3, 6, 9, 6, 1, 5, 1},
                {1, 3, 4, 4, 4, 1, 5, 1},
                {1, 8, 4, 4, 8, 9, 5, 1},
                {9, 7, 4, 8, 7, 1, 5, 1},
                {8, 8, 8, 7, 7, 1, 5, 1}
        };

        // Iterating over each row in colorGrid
        for (int i = 0; i < 8; i++) {
            // Initialize start column of the current consecutive color block
            int startCol = 0;
            // Set the initial color to the color of the first block in the row
            int currentColor = colorGrid[i][0];

            // Iterate over each column in colorGrid starting from the second column
            for (int j = 1; j < 8; j++) {
                // If the color of the current block is different from the currentColor,
                // it means a new color block has started
                if (colorGrid[i][j] != currentColor) {
                    float width = j - startCol;
                    float centerPositionX = startCol + width / 2.0f;
                    glm::vec3 position = {centerPositionX, i, 0.f};

                    // Here's the change. Using createColoredCubeModel and fetching the color from colorLookup.
                    std::shared_ptr<LveModel> lveModel = createColoredCubeModel(lveDevice, colorLookup[currentColor]);

                    auto cube = LveGameObject::createGameObject();
                    cube.model = lveModel;
                    cube.transform.translation = position;
                    cube.transform.scale = {width, 1.0f, 1.0f};

                    gameObjects.push_back(std::move(cube));

                    startCol = j;
                    currentColor = colorGrid[i][j];
                }
            }

            float width = 8 - startCol;
            float centerPositionX = startCol + width / 2.0f;
            glm::vec3 position = {centerPositionX, i, 0.f};

            // Here's another change. Using createColoredCubeModel and fetching the color from colorLookup.
            std::shared_ptr<LveModel> lveModel = createColoredCubeModel(lveDevice, colorLookup[currentColor]);

            auto cube = LveGameObject::createGameObject();
            cube.model = lveModel;
            cube.transform.translation = position;
            cube.transform.scale = {width, 1.0f, 1.0f};

            gameObjects.push_back(std::move(cube));
        }
    }
}