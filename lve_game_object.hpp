//
// Created by cdgira on 7/13/2023.
//

#ifndef VULKANTEST_LVE_GAME_OBJECT_HPP
#define VULKANTEST_LVE_GAME_OBJECT_HPP

#include "lve_model.hpp"
#include <memory>

namespace lve {

    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.0f, 1.0f};
        float rotation{0.0f};

        glm::mat2 mat2() {
            const float s = sin(rotation);
            const float c = cos(rotation);
            glm::mat2 rotationMat{{c, s},
                                  {-s, c}};
            glm::mat2 scaleMat{{scale.x, 0.0f},
                               {0.0f,    scale.y}};
            return rotationMat * scaleMat;
        };
    };

    class LveGameObject {
    public:
        using id_t = unsigned int;

        static LveGameObject createGameObject() {
            static id_t currentId = 0;
            return LveGameObject{currentId++};
        }

        LveGameObject(const LveGameObject&) = delete;
        LveGameObject &operator=(const LveGameObject&) = delete;
        LveGameObject(LveGameObject&&) = default;
        LveGameObject &operator=(LveGameObject&&) = default;

        id_t getId() const { return id; }

        std::shared_ptr<LveModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d{};

        // Add the clockwise property
        bool clockwise = true;

    private:
        LveGameObject(id_t id) : id(id) {}
        id_t id;
    };
}

#endif //VULKANTEST_LVE_GAME_OBJECT_HPP
