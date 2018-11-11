#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <iostream>
#include <assert.h>
#include <vector>
#include <chrono>
#include "utils.hxx"
#include "VulkanFunctions.hxx"

#define USE_SWAPCHAIN_EXTENSIONS

using namespace std;

typedef uint32_t GLFWresult;  
void getVulkanFunctions()
{
    #define VK_GLOBAL_FUNCTION( fun ) fun = (PFN_##fun)glfwGetInstanceProcAddress( NULL , #fun ); assert( fun );
    #include "VulkanFunctions.inl"
}
void getVulkanFunctions(VkInstance instance)
{
    #define VK_INSTANCE_FUNCTION( fun ) fun = (PFN_##fun)glfwGetInstanceProcAddress( instance , #fun ); assert( fun );
    #include "VulkanFunctions.inl"
}
void getVulkanFunctions(VkDevice device)
{
    #define VK_DEVICE_FUNCTION( fun ) fun = (PFN_##fun)vkGetDeviceProcAddr( device , #fun ); assert( fun );
    #include "VulkanFunctions.inl"
}
int main()
{
    GLFWresult glfw_res;
    glfw_res = glfwInit();
    assert(GLFW_TRUE == glfw_res);

    glfw_res = glfwVulkanSupported();
    assert(GLFW_TRUE == glfw_res);

    getVulkanFunctions();
    
    VkApplicationInfo app_info =
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        VK_NULL_HANDLE,
        "Vulkan Test",
        VK_MAKE_VERSION(1 , 0 , 0),
        "Tatli",
        VK_MAKE_VERSION(1 , 0 , 0),
        VK_MAKE_VERSION(1 , 0 , 0)
    };
    
    uint32_t extension_count;
    const char** instance_extensions = glfwGetRequiredInstanceExtensions(&extension_count);
    assert(nullptr !=instance_extensions);

    VkInstanceCreateInfo instance_info =
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        &app_info,
        0,
        VK_NULL_HANDLE,
        extension_count,
        instance_extensions
    };
    VkInstance instance;
    VkResult vk_res;
    vk_res = vkCreateInstance(&instance_info , VK_NULL_HANDLE , &instance);
    assert(VK_SUCCESS == vk_res);

    getVulkanFunctions(instance);
    
    uint32_t pyhsical_device_count;
    vk_res = vkEnumeratePhysicalDevices(instance , &pyhsical_device_count , VK_NULL_HANDLE);
    assert(VK_SUCCESS == vk_res);

    vector<VkPhysicalDevice> physical_devices( pyhsical_device_count );

    vk_res = vkEnumeratePhysicalDevices(instance , &pyhsical_device_count , physical_devices.data());
    assert(VK_SUCCESS == vk_res);

    VkPhysicalDeviceProperties physical_device_prop = {};
    vkGetPhysicalDeviceProperties(physical_devices[0] , &physical_device_prop );
    assert(0 != physical_device_prop.apiVersion);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0] , &queue_family_count , VK_NULL_HANDLE );
    assert(0 != queue_family_count);

    uint32_t queue_family_present_count = queue_family_count;
    uint32_t *queue_family_index_present_priority = new uint32_t[queue_family_count];

    for(uint32_t queue_family_index = 0; queue_family_index < queue_family_count; queue_family_index++)
    {
        glfw_res = glfwGetPhysicalDevicePresentationSupport(instance , physical_devices[0] , queue_family_index );
        
        if(GLFW_TRUE == glfw_res) queue_family_index_present_priority[queue_family_index] = queue_family_index;
        else queue_family_index_present_priority[--queue_family_present_count] = queue_family_index; 
    }
    assert(queue_family_count != queue_family_present_count);
    float queue_priorities[1] = {0.0};
    VkDeviceQueueCreateInfo queue_info =
    {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        queue_family_index_present_priority[0],
        1,
        queue_priorities
    };
    
    const char *device_ext[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo device_info = 
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &queue_info,
        0,
        VK_NULL_HANDLE,
        NS_ARRAY_LENGTH(device_ext),
        device_ext,
        VK_NULL_HANDLE
    };

    VkDevice device;
    vk_res = vkCreateDevice(physical_devices[0] , &device_info , VK_NULL_HANDLE , &device );
    assert(VK_SUCCESS == vk_res);

    getVulkanFunctions(device);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    WindowProperties &window_properties = WindowProperties::get_instance();
    window_properties.setWidth(1280);
    window_properties.setHeight(720);
    window_properties.setTitle("Vulkan Test");

    GLFWwindow* window = glfwCreateWindow(window_properties.getWidth(), 
                                          window_properties.getHeight(),
                                          window_properties.getTitle(), 
                                          NULL, 
                                          NULL);
    assert(nullptr != window);

    VkSurfaceKHR surface;
    vk_res = glfwCreateWindowSurface(instance, window, NULL, &surface);	
    assert(VK_SUCCESS == vk_res);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_devices[0] , surface , &surface_capabilities );
    assert(0 != surface_capabilities.minImageCount);

    uint32_t swapchain_image_count = surface_capabilities.minImageCount;
    if( (surface_capabilities.maxImageCount > 0) &&
        (swapchain_image_count > surface_capabilities.maxImageCount) ) 
    {
      swapchain_image_count = surface_capabilities.maxImageCount;
    }

    uint32_t surface_format_count{};
    vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[0] , surface , &surface_format_count , VK_NULL_HANDLE);
    assert(VK_SUCCESS == vk_res);

    vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[0] , surface , &surface_format_count , surface_formats.data());
    assert(VK_SUCCESS == vk_res);

    surface_formats[0] = getSwapChainFormat(surface_formats);

    VkImageUsageFlags swapchain_image_flags = getSwapChainUsageFlags(surface_capabilities);

    VkSwapchainCreateInfoKHR swapchain_info =
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        VK_NULL_HANDLE,
        0,
        surface,
        swapchain_image_count,
        surface_formats[0].format,
        surface_formats[0].colorSpace,
        window_properties.getDimensionExtent(),
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        queue_family_present_count,
        queue_family_index_present_priority,
        surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR 
            ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR 
            : surface_capabilities.currentTransform,
        getSupportedSwapchainFlagBit(surface_capabilities.supportedCompositeAlpha),
        VK_PRESENT_MODE_FIFO_KHR,
        VK_TRUE,
        VK_NULL_HANDLE
    };
    VkSwapchainKHR swapchain;
    vk_res = vkCreateSwapchainKHR(device , &swapchain_info , VK_NULL_HANDLE , &swapchain);
    assert(VK_SUCCESS == vk_res);

    VkImage *swapchain_images = new VkImage[swapchain_image_count];
    vk_res = vkGetSwapchainImagesKHR(device , swapchain , &swapchain_image_count , swapchain_images );
    assert(VK_SUCCESS == vk_res);

    assert(VK_SUCCESS == vk_res);

    VkCommandPoolCreateInfo cmd_pool_info = 
    {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        queue_family_index_present_priority[0]
    };

    VkCommandPool cmd_pool;
    vk_res = vkCreateCommandPool(device , &cmd_pool_info , VK_NULL_HANDLE , &cmd_pool);
    assert(VK_SUCCESS == vk_res);

    VkCommandBufferAllocateInfo cmd_buffer_info =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        VK_NULL_HANDLE,
        cmd_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    VkCommandBuffer cmd_buffer;
    vk_res = vkAllocateCommandBuffers(device , &cmd_buffer_info , &cmd_buffer);
    assert(VK_SUCCESS == vk_res);

    VkSemaphoreCreateInfo semaphore_info = 
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        VK_NULL_HANDLE,
        0
    };

    VkSemaphore semaphore;
    vk_res = vkCreateSemaphore(device , &semaphore_info , VK_NULL_HANDLE , &semaphore);
    assert(VK_SUCCESS == vk_res);

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo cmd_submit_info = 
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE,
        &wait_dst_stage_mask,
        1,
        &cmd_buffer,
        0,
        VK_NULL_HANDLE
    };

    VkQueue queue;
    vkGetDeviceQueue(device , queue_family_index_present_priority[0] , 0 , &queue );

    uint32_t swapchain_available_image_index;

    VkImageSubresourceRange image_subresource_range =
    {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1
    };

    VkImageMemoryBarrier barrier_from_present_to_clear =
    {
        VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        VK_NULL_HANDLE,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        queue_family_index_present_priority[0],
        queue_family_index_present_priority[0],
        swapchain_images[0],
        image_subresource_range
    };

    VkImageMemoryBarrier barrier_from_clear_to_present =
    {
        VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        VK_NULL_HANDLE,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        queue_family_index_present_priority[0],
        queue_family_index_present_priority[0],
        swapchain_images[0],
        image_subresource_range
    };

    VkCommandBufferBeginInfo cmd_buffer_begin_info =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE
    };

    VkClearColorValue clear_color = 
    {
        { 1.0f, 0.8f, 0.4f, 0.0f }
    };

    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,                // VkStructureType              sType
      VK_NULL_HANDLE,                                      // const void                  *pNext
      0,                                            // uint32_t                     waitSemaphoreCount
      VK_NULL_HANDLE,                               // const VkSemaphore           *pWaitSemaphores
      &wait_dst_stage_mask,                         // const VkPipelineStageFlags  *pWaitDstStageMask;
      1,                                            // uint32_t                     commandBufferCount
      &cmd_buffer,                                  // const VkCommandBuffer       *pCommandBuffers
      1,                                            // uint32_t                     signalSemaphoreCount
      &semaphore            // const VkSemaphore           *pSignalSemaphores
    };

    VkResult vk_present_res;
    VkPresentInfoKHR present_info = 
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        VK_NULL_HANDLE,
        1,
        &semaphore,
        1,
        &swapchain,
        &swapchain_available_image_index,
        &vk_present_res
    };

    long long now = (std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
    while (!glfwWindowShouldClose(window))
    {
        if( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }
        glfwPollEvents();

        vkBeginCommandBuffer(cmd_buffer , &cmd_buffer_begin_info);

        vkCmdPipelineBarrier(cmd_buffer , VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear );

        vkCmdClearColorImage(cmd_buffer , barrier_from_present_to_clear.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &image_subresource_range );

        vkCmdPipelineBarrier(cmd_buffer , VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present );

        vk_res = vkEndCommandBuffer(cmd_buffer);
        assert(VK_SUCCESS == vk_res);

        vk_res = vkAcquireNextImageKHR(device , swapchain , UINT64_MAX , VK_NULL_HANDLE , VK_NULL_HANDLE , &swapchain_available_image_index);
        assert(VK_SUCCESS == vk_res);
        
        
        vkQueueSubmit(queue , 1 , &submit_info , VK_NULL_HANDLE );


        vkQueuePresentKHR(queue , &present_info);

        barrier_from_present_to_clear.image = barrier_from_clear_to_present.image = barrier_from_clear_to_present.image == swapchain_images[0] ? swapchain_images[1] : swapchain_images[0];  
        clear_color.float32[0] = clear_color.float32[0] < 0.0f ? 1.0f : clear_color.float32[0] - 0.01f;

        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
        // cout << ms.count() - now << endl;
        now = ms.count();
    }

    glfwTerminate();

    return 0;
}