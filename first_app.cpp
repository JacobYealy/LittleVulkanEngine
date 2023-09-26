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
    /* Constructor */
    FirstApp::FirstApp() {
        loadGameObjects();  // Load all game objects during initialization
    }

    /* Destructor */
    FirstApp::~FirstApp() {}

    /* Main application loop */
    void FirstApp::run() {
        // Create simple render system
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        LveCamera camera{};  // Initialize camera

        // Initialize the viewerObject's position
        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation = {3.0f, 3.0f, -5.0f};

        KeyboardMovementController cameraController{}; // For moving camera with keyboard

        // Timing data for spinning functionality
        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            // Time calculations for frame-dependent movement
            auto newTime = std::chrono::high_resolution_clock::now();
            const float spinSpeed = glm::radians(90.0f);  // 90 degrees per second
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // Press 'R' to rotate around the image!
            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_R) == GLFW_PRESS) {
                glm::vec3 objectPosition = {5.0f, 0.0f, 0.0f}; // Setting object position to the origin
                glm::vec3 relativePosition = viewerObject.transform.translation - objectPosition;

                float angle = spinSpeed * frameTime;
                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
                relativePosition = rotation * glm::vec4(relativePosition, 1.0f);

                viewerObject.transform.translation = objectPosition + glm::vec3(relativePosition);

                glm::vec3 newTarget = objectPosition;
                glm::vec3 upVector = glm::vec3(0.0f, -1.0f, 0.0f);  // fixed up-vector for now

                camera.setViewTarget(viewerObject.transform.translation, newTarget, upVector);
            }
            // Default camera movement using keyboard
            else {
                cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
                camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            }
            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(120.f), aspect, 0.1f, 10.f);

            // Render command
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    }

    /* Function to create a colored cube model */
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

                    // Using createColoredCubeModel and fetching the color from colorLookup.
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

            std::shared_ptr<LveModel> lveModel = createColoredCubeModel(lveDevice, colorLookup[currentColor]);

            auto cube = LveGameObject::createGameObject();
            cube.model = lveModel;
            cube.transform.translation = position;
            cube.transform.scale = {width, 1.0f, 1.0f};

            gameObjects.push_back(std::move(cube));
        }
    }
}