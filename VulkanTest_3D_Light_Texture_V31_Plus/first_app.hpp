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
        bool isAnimatingDragon1 = false;
        bool isAnimatingDragon2 = false;
        int DRAGON1_ID, DRAGON2_ID;
        float animationProgressDragon1 = 0.0f;
        float animationProgressDragon2 = 0.0f;  // Added for Dragon 2's animation
        glm::vec3 dragon1OriginalPosition;
        glm::vec3 dragon1OriginalRotation;
        glm::vec3 dragon2OriginalPosition;      // Added for Dragon 2
        glm::vec3 dragon2OriginalRotation;      // Added for Dragon 2
        glm::vec3 dragon1TargetPosition;        // Target position for Dragon 1's movement
        glm::vec3 dragon2TargetPosition;        // Target position for Dragon 2's movement
        float frameTime;                        // Elapsed time since the last frame (ensure this is updated every frame)



        void loadGameObjects();

        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};

        // Texture for dragon
        std::shared_ptr<LveImage> textureImage = LveImage::createImageFromFile(lveDevice, "../textures/escamas.png");

        // Texture for eaten planet
        std::shared_ptr<LveImage> terrainTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/space.png");

        //Texture for blue dragon
        std::shared_ptr<LveImage> dinoTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/bluedragon.png");

        //Texture for sky
        std::shared_ptr<LveImage> skyTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/sky.png");

        std::vector<std::shared_ptr<LveImage>> textureImages; // For maintaining the list of textures

        LveRenderer lveRenderer{lveWindow, lveDevice};
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;
    };

} // namespace lve

#endif //VULKANTEST_FIRST_APP_HPP