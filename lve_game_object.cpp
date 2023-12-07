#include "lve_game_object.hpp"

namespace lve {

    void LveGameObject::setParent(LveGameObject* newParent) {
        parent = newParent;
        if (newParent) {
            newParent->addChild(this);
        }
    }

    void LveGameObject::addChild(LveGameObject* child) {
        children.push_back(child);
    }

    void LveGameObject::updateGlobalTransform() {
        glm::mat4 parentMatrix = (parent != nullptr) ? parent->getGlobalTransformMatrix() : glm::mat4(1.0f);
        transform.mat4() = parentMatrix * transform.localMatrix();
        for (auto* child : children) {
            child->updateGlobalTransform();
        }
    }

    glm::mat4 LveGameObject::getGlobalTransformMatrix() const {
        glm::mat4 parentMatrix = (parent != nullptr) ? parent->getGlobalTransformMatrix() : glm::mat4(1.0f);
        return parentMatrix * transform.localMatrix();
    }



    glm::mat4 TransformComponent::mat4(const glm::mat4& parentGlobalMatrix) {
        glm::mat4 localTransformMatrix = localMatrix();
        return parentGlobalMatrix * localTransformMatrix;
    }

    glm::mat4 TransformComponent::normalMatrix(const glm::mat4& parentGlobalMatrix) {
        glm::mat4 globalTransformMatrix = parentGlobalMatrix * localMatrix();
        const glm::vec3 invScale = 1.0f / scale;
        return glm::mat4{
                {invScale.x * globalTransformMatrix[0]},
                {invScale.y * globalTransformMatrix[1]},
                {invScale.z * globalTransformMatrix[2]},
                {0.0f, 0.0f, 0.0f, 1.0f}
        };
    }

    LveGameObject LveGameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
        LveGameObject gameObj = LveGameObject::createGameObject();
        gameObj.color = color;
        gameObj.transform.scale.x = radius;
        gameObj.pointLight = std::make_unique<PointLightComponent>();
        gameObj.pointLight->lightIntensity = intensity;
        return gameObj;
    }

    /**
     * Animates an object and the animation sequence.
     * This means it controls translation, scale, and rotation.
     * @param deltaTime: The time since the last frame.
     */
    bool TransformComponent::update(float deltaTime) {
        if (!isPlaying) {
            return false;
        }
        currentTime += deltaTime;
        if (currentTime > animationSequence.duration) {
            currentTime = 0.0f;
            return false;
        }
        AnimationFrame* prevFrame = nullptr;
        AnimationFrame* nextFrame = nullptr;
        if (animationSequence.Frames.size() < 2) {
            return false;
        }
        for (size_t i = 0; i < animationSequence.Frames.size() - 1; i++) {
            if (animationSequence.Frames[i].timeStamp <= currentTime && animationSequence.Frames[i+1].timeStamp > currentTime) {
                prevFrame = &animationSequence.Frames[i];
                nextFrame = &animationSequence.Frames[i+1];
                break;
            }
        }


        // Linear Interpolation (mix)
        if (prevFrame && nextFrame) {
            float alpha = (currentTime - prevFrame->timeStamp) / (nextFrame->timeStamp - prevFrame->timeStamp); // Progress between frames
            translation = glm::mix(prevFrame->translation, nextFrame->translation, alpha);
            rotation = glm::mix(prevFrame->rotation, nextFrame->rotation, alpha);
            scale = glm::mix(prevFrame->scale, nextFrame->scale, alpha);
        }
        // Update the global transform of all children
        for (auto& child : children) {
            child->updateGlobalTransform();
        }

        return true;
    }
}