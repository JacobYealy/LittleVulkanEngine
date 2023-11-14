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
        int DRAGON1_ID, DRAGON2_ID, PLANET_ID;

        // Planet animation vars
        glm::vec3 planetOriginalScale;
        glm::vec3 planetOriginalPosition;

        // Dragon 1 animation vars
        glm::vec3 dragon1OriginalPosition;
        glm::vec3 dragon1OriginalRotation;
        glm::vec3 dragon1TargetPosition;
        glm::vec3 dragon1TargetRotation;
        glm::vec3 dragon1OriginalScale, dragon1TargetScale;

        // Dragon 2 animation vars
        glm::vec3 dragon2OriginalPosition;
        glm::vec3 dragon2OriginalRotation;
        glm::vec3 dragon2TargetPosition;
        glm::vec3 dragon2TargetRotation;
        glm::vec3 dragon2OriginalScale, dragon2TargetScale;



        void loadGameObjects();

        LveWindow lveWindow{WIDTH, HEIGHT, "Dueling Dragons!"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;

        // Texture for dragon
        std::shared_ptr<LveImage> textureImage = LveImage::createImageFromFile(lveDevice, "../textures/escamas.png");

        // Texture for eaten planet
        std::shared_ptr<LveImage> terrainTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/space.png");

        //Texture for blue dragon
        std::shared_ptr<LveImage> dinoTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/bluedragon.png");

        //Texture for sky
        std::shared_ptr<LveImage> skyTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/sky.png");

        //Texture for black hole
        std::shared_ptr<LveImage> holeTextureImage = LveImage::createImageFromFile(lveDevice, "../textures/galaxy.jpg");

        std::vector<std::shared_ptr<LveImage>> textureImages; // For maintaining the list of textures

    };

} // namespace lve

#endif //VULKANTEST_FIRST_APP_HPP