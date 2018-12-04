#ifndef VULKAN_H
#define VULKAN_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

class Vulkan
{
    public:
        Vulkan(VkApplicationInfo&);
        ~Vulkan();
        VkApplicationInfo application_info;
        
        struct PhysicalDevice
        {
            private:
                friend Vulkan;
                VkPhysicalDeviceProperties properties;
                VkPhysicalDevice device;
                std::vector<VkQueueFamilyProperties> queue_family_properties;
                uint32_t queue_family_index;
            public:
                const VkPhysicalDevice& getDevice(void) const;
                const VkPhysicalDeviceProperties&  getDeviceProperties(void) const;
                const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties(void) const;
                const uint32_t& getQueueFamilyIndex(void) const;
        };
        struct LogicalDevice
        {
            private:
                friend Vulkan;
                VkDevice logical_device;
                VkSurfaceKHR surface;
                VkRenderPass render_pass;
                std::vector<VkFramebuffer> frame_buffers;
                VkBuffer vertex_buffer;
                VkPipeline pipeline;
                VkCommandBuffer cmd_buffer;
            public:
                const VkDevice& getDevice(void) const;
        };
        struct SwapchainKHR
        {
            private:
                friend Vulkan;
                VkSwapchainKHR swapchain;
                std::vector<VkImage> images;
                std::vector<VkImageView> imageviews;
                VkSurfaceKHR surface;
                std::vector<VkSurfaceFormatKHR> formats;
                VkSurfaceFormatKHR prefered_surface_format;
                VkSurfaceCapabilitiesKHR surface_capabilities;
            public:
                const VkSurfaceFormatKHR& getPreferedSurfaceFormat(void) const;
                const std::vector<VkSurfaceFormatKHR>& getSurfaceFormats(void) const;
        };

        void createLogicalDevice(uint32_t physical_device_index);
        void createSwapchain(GLFWwindow *window , uint32_t width , uint32_t height);
        void createRenderPass(uint32_t width , uint32_t height);
        void createGraphicsPipeline(std::string source_dir_path , uint32_t width , uint32_t height );
        void createCommandBuffer(void);

        void prepareRendering(GLFWwindow *window , uint32_t width , uint32_t height);

        const std::vector<PhysicalDevice>& getPhysicalDevices(void) const;
        const PhysicalDevice& getCurrentPhysicalDevice(void) const;
        const SwapchainKHR& getSwapchainKHR(void) const;
        void getVulkanFunctions();
        void getVulkanFunctions(VkInstance instance);
        void getVulkanFunctions(VkDevice device);

        private:
            VkInstance instance;
            std::vector<PhysicalDevice> physical_devices;
            PhysicalDevice *current_physical_device;
            LogicalDevice logical_device;
            SwapchainKHR swapchain;
};

#endif /* VULKAN_H */