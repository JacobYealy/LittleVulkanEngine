//
// Created by cdgira on 6/30/2023.
//

#ifndef VULKANTEST_FIRST_APP_HPP
#define VULKANTEST_FIRST_APP_HPP

#include "lve_window.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include "lve_descriptors.hpp"
#include "lve_image.hpp"


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

        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};

        // Dragon texture
        std::shared_ptr<LveImage> textureImage = LveImage::createImageFromFile(lveDevice, "../textures/escamas.png");

        // Terrain texture
        std::shared_ptr<LveImage> terrainTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/space.png");

        // Dino texture
        std::shared_ptr<LveImage> dinoTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/scales.png");

        LveRenderer lveRenderer{lveWindow, lveDevice};

        // Order of declaration is important.
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;
    };

} // namespace lve

#endif //VULKANTEST_FIRST_APP_HPP