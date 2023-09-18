//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"
#include "simple_render_system.hpp"
#include <cmath> // For trigonometric and other math functions

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>
#include <unordered_map>

namespace lve {

    FirstApp::FirstApp() {
        loadGameObjects();
    }

    FirstApp::~FirstApp() { }

    /**
 * Main application loop.
 *
 * In this function, the application's main event loop is run. The loop is responsible for
 * rendering game objects and handling any other tasks that should be continuously executed.
 *
 * The character will rotate and shrink to 50% of its original size while
 * rotating clockwise. Once it reaches 50%, it will start to grow back to its original size
 * while rotating counter-clockwise. This behavior repeats indefinitely.
 */
    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        float time = 0.0f; // Initialize a time variable outside of your loop
        float cycleTime = 15.0f; // Time it takes to complete a full shrink-and-grow cycle in seconds
        float maxScale = 1.0f; // Max scale (original size)
        float minScale = 0.5f; // Min scale (50% size)

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            float dt = 0.00466f; // Timestep (Adjust to make faster or slower progression)
            time += dt;

            // Normalize the time to [0, 1] for one complete cycle
            float normalizedTime = std::fmod(time, cycleTime) / cycleTime;

            // Compute the new scale, swinging between maxScale and minScale
            float scale = maxScale - normalizedTime * (maxScale - minScale);
            if (normalizedTime > 0.5f) {
                scale = minScale + (normalizedTime - 0.5f) * 2.0f * (maxScale - minScale);
            }

            // Compute the rotation direction (-1 or 1)
            float rotationDirection = (normalizedTime <= 0.5f) ? 1.0f : -1.0f;

            // Compute the new rotation
            float rotation = std::fmod(time, cycleTime) * glm::two_pi<float>() / cycleTime * rotationDirection;

            // Assuming gameObjects[0] is your character, adjust its transformation
            gameObjects[0].transform2d.rotation = rotation;
            gameObjects[0].transform2d.scale = {scale, scale};

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
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
 * Load Game Objects into the Application.
 *
 * This function is responsible for loading all the game objects that will be rendered
 * in the application window. It creates a character model using vertex
 * data generated from a predefined grid and color map.
 *
 * The vertices for the character model are generated based on a grid of "pixels," where
 * each "pixel" is a square made up of two triangles. The color of each "pixel" is looked
 * up from a predefined color map.
 *
 * Once the character model has been created, it's added to the list of game objects
 * to be rendered.
 */
    void FirstApp::loadGameObjects() {
        std::vector<LveModel::Vertex> vertices;
        std::unordered_map<int, glm::vec3> colorLookup = {
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


        // Generate the vertices for your character model.
        for (float i = 0.0; i < 8.0; i++) {
            for (float j = 0.0; j < 8.0; j++) {
                glm::vec3 color = colorLookup[colorGrid[int(j)][int(i)]];
                addPixel(i, j, color, vertices);
            }
        }

        auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

        auto characterGameObject = LveGameObject::createGameObject();
        characterGameObject.model = lveModel;
        characterGameObject.color = {1.0f, 1.0f, 1.0f}; // Assign some color here
        characterGameObject.transform2d.translation = {0.0f, 0.0f}; // Change these parameters as you see fit
        characterGameObject.transform2d.scale = {1.0f, 1.0f};
        characterGameObject.transform2d.rotation = 0.0f; // Rotation angle

        gameObjects.push_back(std::move(characterGameObject));
    }

}
