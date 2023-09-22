//
// Created by cdgira on 7/13/2023.
//

#ifndef VULKANTEST_LVE_GAME_OBJECT_HPP
#define VULKANTEST_LVE_GAME_OBJECT_HPP

#include "lve_model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace lve {

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{0.0f};

        // Need to go over base form of each in class.
        // Need to show standard rotation matrix found in most books.
        glm::mat4 mat4() {
            glm::mat4 matrix{1.0f};
            matrix = glm::translate(matrix, translation);
            matrix = glm::rotate(matrix, rotation.x, glm::vec3{1.0f, 0.0f, 0.0f});
            matrix = glm::rotate(matrix, rotation.y, glm::vec3{0.0f, 1.0f, 0.0f});
            matrix = glm::rotate(matrix, rotation.z, glm::vec3{0.0f, 0.0f, 1.0f});
            matrix = glm::scale(matrix, scale);
            return matrix;
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
        TransformComponent transform{};

    private:
        LveGameObject(id_t id) : id(id) {}
        id_t id;
    };
}

#endif //VULKANTEST_LVE_GAME_OBJECT_HPP
