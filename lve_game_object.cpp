#include "lve_game_object.hpp"

namespace lve {

    glm::mat4 TransformComponent::mat4(){
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.y);
        const float s2 = glm::sin(rotation.y);
        const float c1 = glm::cos(rotation.x);
        const float s1 = glm::sin(rotation.x);
        return glm::mat4{
                {
                        scale.x * (c1 * c3 + s1 * s2 * s3),
                                scale.x * (c2 * s3),
                                               scale.x * (c1 * s2 * s3 - c3 * s1),
                                                              0.0f},
                {
                        scale.y * (c3 * s1 * s2 - c1 * s3),
                                scale.y * (c2 * c3),
                                               scale.y * (c1 * c3 * s2 + s1 * s3),
                                                              0.0f},
                {
                        scale.z * (c2 * s1),
                                scale.z * (-s2),
                                               scale.z * (c1 * c2),
                                                              0.0f},
                {translation.x, translation.y, translation.z, 1.0f}};
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
}