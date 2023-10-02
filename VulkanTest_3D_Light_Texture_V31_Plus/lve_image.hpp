//
// Created by cdgira on 8/4/2023.
//

#ifndef VULKANTEST_LVE_IMAGE_HPP
#define VULKANTEST_LVE_IMAGE_HPP

#include "lve_buffer.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"

#include <memory>

namespace lve {

        class LveImage {
        public:

            LveImage(LveDevice &device, uint32_t width, uint32_t height, LveBuffer &imgBuffer);
            ~LveImage();

            LveImage(const LveImage&) = delete;
            LveImage &operator=(const LveImage&) = delete;

            static std::unique_ptr<LveImage> createImageFromFile(LveDevice &lveDevice, const std::string &filepath);
            VkDescriptorImageInfo descriptorImageInfo();

        private:
            void createImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
            void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

            void createImageView(VkFormat format);
            void createTextureSampler();
            void generateMipmaps();

            LveDevice &lveDevice;
            uint32_t width, height, mipLevels; // Using for MipMaps.
            uint32_t arrayLayers = 1;
            VkImage image;
            VkDeviceMemory imageMemory;
            VkImageView imageView;
            VkSampler textureSampler;
        };
}

#endif //VULKANTEST_LVE_IMAGE_HPP
