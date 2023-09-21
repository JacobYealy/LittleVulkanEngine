#include "first_app.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>
#include <map>

namespace lve {

    /**
* Adds a "pixel" (actually a square made up of two triangles) to the vertices array.
* The pixel's position on the grid is determined by (i, j), and its color is specified.
*
* @param i         The i-th row position of the pixel in the grid.
* @param j         The j-th column position of the pixel in the grid.
* @param color     The color of the pixel in RGB format.
* @param vertices  The vector of vertices to which the pixel's vertices will be added.
*/
    void FirstApp::addPixel(float i, float j, glm::vec3 color, std::vector<LveModel::Vertex>& vertices) {
        float x_offset = i * 0.25f;
        float y_offset = j * 0.25f;
        vertices.push_back({{-1.00f + x_offset, -1.00f + y_offset}, color});
        vertices.push_back({{-0.75f + x_offset, -1.00f + y_offset}, color});
        vertices.push_back({{-0.75f + x_offset, -0.75f + y_offset}, color});

        vertices.push_back({{-1.00f + x_offset, -1.00f + y_offset}, color});
        vertices.push_back({{-1.00f + x_offset, -0.75f + y_offset}, color});
        vertices.push_back({{-0.75f + x_offset, -0.75f + y_offset}, color});
    }

    FirstApp::FirstApp() {
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

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
        std::vector<LveModel::Vertex> vertices{};

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

        for (float i = 0.0; i < 8.0; i++) {
            for (float j = 0.0; j < 8.0; j++) {
                glm::vec3 color = colorLookup[colorGrid[int(j)][int(i)]];
                addPixel(i, j, color, vertices);
            }
        }

        auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);
        auto triangleGameObject = LveGameObject::createGameObject();
        triangleGameObject.model = lveModel;
        gameObjects.push_back(std::move(triangleGameObject));
    }
}
