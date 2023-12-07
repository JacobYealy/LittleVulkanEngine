#include "lve_game_object.hpp"

namespace lve {

    glm::mat4 TransformComponent::mat4() {
        // Rotation matrices
        glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        // Scale matrix
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

        // Translation matrix
        glm::mat4 transMat = glm::translate(glm::mat4(1.0f), translation);

        // Combine transformations
        glm::mat4 modelMatrix = transMat * rotZ * rotY * rotX * scaleMat;

        // Include parent's transformation if present
        if (parent != nullptr) {
            glm::mat4 parentMatrix = parent->mat4();
            return parentMatrix * modelMatrix;
        }

        return modelMatrix;
    }



    glm::mat4 TransformComponent::normalMatrix() {
        glm::mat4 localMatrix = mat4();
        glm::mat3 normalMat3 = glm::mat3(glm::transpose(glm::inverse(localMatrix)));

        return glm::mat4(normalMat3);
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
     * Update handles animation sequence
     * @param deltaTime: The time since the last frame.
     */
    bool TransformComponent::update(float deltaTime) {
        currentTime += deltaTime;
        // reset animation when done
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
        // If frames are good then interpolate
        if (prevFrame && nextFrame) {
            float alpha = (currentTime - prevFrame->timeStamp) / (nextFrame->timeStamp - prevFrame->timeStamp);
            translation = glm::mix(prevFrame->translation, nextFrame->translation, alpha);
            rotation = glm::mix(prevFrame->rotation, nextFrame->rotation, alpha);
            scale = glm::mix(prevFrame->scale, nextFrame->scale, alpha);
        }
        return true;
    }
}
