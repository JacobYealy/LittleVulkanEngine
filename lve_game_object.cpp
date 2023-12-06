//
// Created by cdgira on 7/31/2023.
//
#include "lve_game_object.hpp"

namespace lve {

    glm::mat4 TransformComponent::mat4(const glm::mat4& parentTransform) {
        // Calculate the rotation matrices for each axis
        glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        // Combine the rotation matrices
        glm::mat4 rotMatrix = rotZ * rotY * rotX;

        // Create the translation and scale matrices
        glm::mat4 transMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

        // Combine all transformations
        glm::mat4 localTransform = transMatrix * rotMatrix * scaleMatrix;

        // Combine with the parent transform
        return parentTransform * localTransform;
    }


    glm::mat4 TransformComponent::normalMatrix(){
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.y);
        const float s2 = glm::sin(rotation.y);
        const float c1 = glm::cos(rotation.x);
        const float s1 = glm::sin(rotation.x);
        const glm::vec3 invScale = 1.0f / scale;
        return glm::mat4{
                {
                        invScale.x * (c1 * c3 + s1 * s2 * s3),
                        invScale.x * (c2 * s3),
                             invScale.x * (c1 * s2 * s3 - c3 * s1),
                                   0.0f},
                {
                        invScale.y * (c3 * s1 * s2 - c1 * s3),
                        invScale.y * (c2 * c3),
                             invScale.y * (c1 * c3 * s2 + s1 * s3),
                                   0.0f},
                {
                        invScale.z * (c2 * s1),
                        invScale.z * (-s2),
                             invScale.z * (c1 * c2),
                                   0.0f},
                {0.0f, 0.0f, 0.0f, 1.0f}};
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
            return false; // Return immediately if not playing
        }
        currentTime += deltaTime;
        // If animation is over, reset
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
        return true;

    }



    void LveGameObject::setParent(LveGameObject* newParent) {
        parent = newParent;
    }

    void LveGameObject::addChild(std::unique_ptr<LveGameObject> child) {
        child->setParent(this);
        children.push_back(std::move(child));
    }

    LveGameObject* LveGameObject::getParent() const {
        return parent;
    }

    const std::vector<std::unique_ptr<LveGameObject>>& LveGameObject::getChildren() const {
        return children;
    }

}
