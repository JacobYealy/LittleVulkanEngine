//
// Created by cdgira on 6/30/2023.
//

#ifndef VULKANTEST_FIRST_APP_HPP
#define VULKANTEST_FIRST_APP_HPP

#include "lve_window.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "lve_renderer.hpp"


#include <memory>
#include <vector>

namespace lve {
    class FirstApp {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp &operator=(const FirstApp&) = delete;

        void run();

    private:
        void loadGameObjects();
        void addPixel(float i, float j, glm::vec3 color, std::vector<LveModel::Vertex>& vertices);  // Newly added
        void loadModels();  // Newly added

        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        std::vector<LveGameObject> gameObjects;
    };
}

#endif //VULKANTEST_FIRST_APP_HPP