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

    /**
 * @function run
 * @brief Main loop of the application, responsible for rendering and handling user input.
 *
 * This function manages the main execution flow, performing tasks like:
 * - Polling for window events.
 * - Updating camera and viewer object positions based on user input.
 * - Executing the render pipeline.
 *
 * Rotation functionality:
 * - When the 'R' key is pressed, the viewer object begins to rotate around a fixed object.
 * - The viewer object completes a full 360-degree rotation around the object and then stops.
 * - During the rotation, other camera controls are locked to ensure smooth transition.
 * - The rotation speed is set to 90 degrees per second, ensuring a complete rotation in 4 seconds.
 *
 * @return void
 */
    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        LveCamera camera{};

        // Initialize the viewerObject's position
        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation = {3.0f, 3.0f, -5.0f};

        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        // State variables for R key rotation behavior
        float accumulatedRotation = 0.0f;  // Accumulator for total rotation done
        bool rotating = false;             // Flag to indicate if rotation is in progress

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            const float spinSpeed = glm::radians(90.0f);  // 90 degrees per second
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // Handle 'R' key press for starting the rotation
            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_R) == GLFW_PRESS && !rotating) {
                rotating = true;      // Set the rotating flag to true
                accumulatedRotation = 0.0f;  // Reset accumulated rotation
            }

            if (rotating) {
                glm::vec3 objectPosition = {5.0f, 0.0f, 0.0f}; // Setting object position to the origin
                glm::vec3 relativePosition = viewerObject.transform.translation - objectPosition;

                float angle = spinSpeed * frameTime;
                accumulatedRotation += angle;  // Add to our accumulator

                if (accumulatedRotation >= glm::radians(360.0f)) {  // Check if a full rotation is done
                    rotating = false;  // Stop rotating
                    angle -= (accumulatedRotation - glm::radians(360.0f));  // Adjust to ensure only 360 degrees in total
                }

                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 10.0f));
                relativePosition = rotation * glm::vec4(relativePosition, 1.0f);

                viewerObject.transform.translation = objectPosition + glm::vec3(relativePosition);

                glm::vec3 newTarget = objectPosition;
                glm::vec3 upVector = glm::vec3(0.0f, -1.0f, 0.0f);  // fixed up-vector for now

                camera.setViewTarget(viewerObject.transform.translation, newTarget, upVector);
            }
            else {
                cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
                camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            }

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(120.f), aspect, 0.1f, 10.f);

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

    /**
 * @function loadGameObjects
 * @brief Populates the game scene with colored cube objects based on a predefined color grid.
 *
 * The function constructs game objects in the scene using two primary elements:
 * 1. A color lookup map that defines RGB color values based on unique integer keys.
 * 2. A 2D color grid that dictates the layout of the colored cubes in the scene.
 *
 * Functionality:
 * - The colorGrid defines the layout of the scene where each cell represents a colored cube.
 * - The color of each cube is determined by mapping its integer value in the colorGrid to the RGB value from colorLookup.
 * - Consecutive blocks of the same color in a row are merged to form a larger cube, optimizing the number of game objects in the scene.
 * - The position and scale of each cube (or merged block of cubes) are calculated based on its placement and width in the colorGrid.
 * - Once the cubes are defined with their color, position, and scale, they are added to the gameObjects list for rendering.
 *
 * @return void
 */
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