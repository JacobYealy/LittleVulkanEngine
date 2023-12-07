#ifndef VULKANTEST_LVE_GAME_OBJECT_HPP
#define VULKANTEST_LVE_GAME_OBJECT_HPP


#include "lve_model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>


namespace lve {

    struct AnimationFrame {
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;
        float timeStamp; // In seconds
    };


    struct AnimationSequence {
        std::vector<AnimationFrame> Frames;
        float duration; // In seconds
    };

    struct TransformComponent {
        bool isPlaying = false; // Flag to indicate if the animation is currently playing
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{0.0f};
        AnimationSequence animationSequence;
        float currentTime = 0.0f;

        glm::mat4 mat4();

        glm::mat4 normalMatrix();

        bool update(float deltaTime);

        glm::mat4 localMatrix() const;

        glm::mat4 globalMatrix(const glm::mat4 &parentMatrix = glm::mat4(1.0f)) const;
    };


    struct PointLightComponent {
        float lightIntensity = 1.0f;
    };


    class LveGameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, LveGameObject>;


        static LveGameObject createGameObject() {
            static id_t currentId = 0;
            return LveGameObject{currentId++};
        }


        static LveGameObject
        makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));


        LveGameObject(const LveGameObject &) = delete;

        LveGameObject &operator=(const LveGameObject &) = delete;

        LveGameObject(LveGameObject &&) = default;

        LveGameObject &operator=(LveGameObject &&) = default;


        id_t getId() const { return id; }


        glm::vec3 color{};
        TransformComponent transform{};
        int32_t textureBinding = -1;
        std::shared_ptr<LveModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

        // New methods for hierarchy
        void setParent(LveGameObject* newParent);

        void addChild(LveGameObject* child);

        const std::vector<LveGameObject *> &getChildren() const;

        glm::mat4 getGlobalTransformMatrix() const {
            glm::mat4 parentMatrix = (parent != nullptr) ? parent->getGlobalTransformMatrix() : glm::mat4(1.0f);
            return parentMatrix * transform.localMatrix();
        }

    private:
        LveGameObject(id_t objectId) : id(objectId) {}

        id_t id;
        LveGameObject *parent = nullptr;
        std::vector<LveGameObject *> children;

        // Helper method to update global transform
        void updateGlobalTransform();
    };
}


#endif //VULKANTEST_LVE_GAME_OBJECT_HPP