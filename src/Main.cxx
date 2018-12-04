// #define VK_NO_PROTOTYPES
// #include <vulkan/vulkan.h>

#define GLFW_DLL 
#include <GLFW/glfw3.h>

#include <whereami++.h>
// #include <glm/glm.hpp> 

#include <iostream>
// #include <assert.h>
// #include <vector>
// #include <chrono>
// #include <cstring>
// #include "utils.hxx"
// #include "VulkanFunctions.hxx"
#include "Vulkan.hxx"
#include "Window.hxx"
using namespace std;


int main()
{
    whereami::whereami_path_t source_dir_path = whereami::getExecutablePath();
    VkApplicationInfo application_info =
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        VK_NULL_HANDLE,
        "Vulkan Playground",
        VK_MAKE_VERSION(1 , 0 , 0),
        "Tatli",
        VK_MAKE_VERSION(1 , 0 , 0),
        VK_MAKE_VERSION(1 , 0 , 0),
    };
    Vulkan vulkan(application_info);

    vulkan.createLogicalDevice(0);
    
    Window window;
    
	window.getWindowProperties().setMonitor(window.getPrimaryMonitor());
	window.getWindowProperties().setWindowSize(1280, 720);
    GLFWwindow *hwindow = window.createWindow();
	
    
    vulkan.createSwapchain(hwindow , window.getWindowProperties().getWidth() , window.getWindowProperties().getHeight());
    vulkan.createRenderPass( window.getWindowProperties().getWidth() , window.getWindowProperties().getHeight());
    vulkan.createGraphicsPipeline(source_dir_path.dirname() ,  window.getWindowProperties().getWidth() , window.getWindowProperties().getHeight() );
    vulkan.createCommandBuffer();
    vulkan.prepareRendering(hwindow ,  window.getWindowProperties().getWidth() , window.getWindowProperties().getHeight() );

    

	system("pause");
    return 0;
}   