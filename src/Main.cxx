#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <iostream>
#include <assert.h>
#include <vector>
#include "utils.hxx"
#include "VulkanFunctions.hxx"

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

    uint8_t queue_family_present = UINT8_MAX;
    for(uint32_t queue_family_index = 0; queue_family_index < queue_family_count; queue_family_index++)
    {
        glfw_res = glfwGetPhysicalDevicePresentationSupport(instance , physical_devices[0] , queue_family_index );
        if(GLFW_TRUE == glfw_res)
        {
            queue_family_present = queue_family_index;
        } 
    }
    assert(UINT8_MAX != queue_family_present);

    float queue_priorities[1] = {0.0};
    VkDeviceQueueCreateInfo queue_info =
    {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        queue_family_present,
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
    window_properties.width = 1280;
    window_properties.height = 720;
    window_properties.title = "Vulkan Test";

    GLFWwindow* window = glfwCreateWindow(window_properties.width, 
                                          window_properties.height,
                                          window_properties.title, 
                                          NULL, 
                                          NULL);
    assert(nullptr != window);

    VkSurfaceKHR surface;
    vk_res = glfwCreateWindowSurface(instance, window, NULL, &surface);	
    assert(VK_SUCCESS == vk_res);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_devices[0] , surface , &surface_capabilities );
    assert(0 != surface_capabilities.minImageCount);

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    if( (surface_capabilities.maxImageCount > 0) &&
        (image_count > surface_capabilities.maxImageCount) ) 
    {
      image_count = surface_capabilities.maxImageCount;
    }

    uint32_t surface_format_count{};
    vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[0] , surface , &surface_format_count , VK_NULL_HANDLE);
    assert(VK_SUCCESS == vk_res);

    vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[0] , surface , &surface_format_count , surface_formats.data());
    assert(VK_SUCCESS == vk_res);

    surface_formats[0] = GetSwapChainFormat(surface_formats);

    VkImageUsageFlags swapchain_image_flags = GetSwapChainUsageFlags(surface_capabilities);
    
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_info = 
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        VK_NULL_HANDLE,
        0
    };

    vk_res = vkCreateSemaphore(device , &semaphore_info , VK_NULL_HANDLE , &semaphore);
    assert(VK_SUCCESS == vk_res);

    while (!glfwWindowShouldClose(window))
    {
        if( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}