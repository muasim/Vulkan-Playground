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
#include "whereami++.h"

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
    whereami::whereami_path_t source_dir_path = whereami::getExecutablePath();
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


    VkAttachmentDescription attachment =
    {
        0,
        surface_formats[0].format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    
    VkAttachmentReference reference = 
    {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass =
    {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        VK_NULL_HANDLE,
        1,
        &reference,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE
    };

    VkRenderPassCreateInfo render_pass_info =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &attachment,
        1,
        &subpass,
        0,
        VK_NULL_HANDLE
    };
    VkRenderPass render_pass;
    vk_res = vkCreateRenderPass(device , &render_pass_info , VK_NULL_HANDLE , &render_pass);
    assert(VK_SUCCESS == vk_res);

    VkImageSubresourceRange image_subresource_range =
    {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1
    };

    VkImageViewCreateInfo swapchain_image_view_info =
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE,
        VK_IMAGE_VIEW_TYPE_2D,
        surface_formats[0].format,
        {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        },
        image_subresource_range
    };
    VkFramebufferCreateInfo frame_buffer_info = 
    {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        render_pass,
        1,
        VK_NULL_HANDLE,
        window_properties.getWidth(),
        window_properties.getHeight(),
        1
    };
    
    VkImageView *swapchain_image_views = new VkImageView[swapchain_image_count];
    VkFramebuffer *frame_buffers = new VkFramebuffer[swapchain_image_count];
    for(size_t i = 0; i < swapchain_image_count; i++)
    {   
        swapchain_image_view_info.image = swapchain_images[i];
        vk_res = vkCreateImageView(device , &swapchain_image_view_info , VK_NULL_HANDLE , swapchain_image_views + i);
        assert(VK_SUCCESS == vk_res);

        frame_buffer_info.pAttachments = swapchain_image_views + i;
        vk_res = vkCreateFramebuffer(device , &frame_buffer_info , VK_NULL_HANDLE , frame_buffers + i);
        assert(VK_SUCCESS == vk_res);
    }
    
    std::vector<char> shader_binary = GetBinaryFileContents(string(source_dir_path.dirname()).append("\\resources\\shaders\\shader.vert.spv"));
    VkShaderModuleCreateInfo shader_module_info = 
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        shader_binary.size(),
        reinterpret_cast<const uint32_t*>(shader_binary.data())
    };

    VkShaderModule shader_vertex_module;
    vk_res = vkCreateShaderModule(device , &shader_module_info , VK_NULL_HANDLE , &shader_vertex_module);
    assert(VK_SUCCESS == vk_res);

    shader_binary = GetBinaryFileContents(string(source_dir_path.dirname()).append("\\resources\\shaders\\shader.frag.spv"));
    shader_module_info.codeSize = shader_binary.size();
    shader_module_info.pCode = reinterpret_cast<const uint32_t*>(shader_binary.data());

    VkShaderModule shader_frag_module;
    vk_res = vkCreateShaderModule(device , &shader_module_info , VK_NULL_HANDLE , &shader_frag_module);
    assert(VK_SUCCESS == vk_res);
    
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
    shader_stages[0].sType = shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].pNext = shader_stages[1].pNext = VK_NULL_HANDLE;
    shader_stages[0].flags = shader_stages[1].flags = 0;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = shader_vertex_module;
    string shader_vertex_name("main"); 
    shader_stages[0].pName =  shader_vertex_name.c_str();
    shader_stages[0].pSpecializationInfo = VK_NULL_HANDLE;

    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = shader_frag_module;
    string shader_frag_name("main"); 
    shader_stages[1].pName =  shader_frag_name.c_str();
    shader_stages[1].pSpecializationInfo = VK_NULL_HANDLE;
    
    VkPipelineVertexInputStateCreateInfo shader_vertex_input_create_info =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        0,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE
    };
    VkPipelineInputAssemblyStateCreateInfo shader_vertex_input_assembly_info =
    {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    VkViewport viewport = 
    {
        0,
        0,
        window_properties.getWidth(),
        window_properties.getHeight(),
        0.0f,
        0.0f
    };

    VkRect2D scissor = 
    {
        VkOffset2D {0 , 0},
        window_properties.getDimensionExtent()
    };
    VkPipelineViewportStateCreateInfo viewport_stage_info =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &viewport,
        1,
        &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterization_stage_info =
    {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisample_stage_info = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1.0f,
        VK_NULL_HANDLE,
        VK_FALSE,
        VK_FALSE
    };
    VkPipelineColorBlendAttachmentState blend_state =
    {
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo blend_stage_info =
    {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &blend_state,
        { 0.0f, 0.0f, 0.0f, 0.0f }
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = 
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        0,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE
    };

    VkPipelineLayout pipeline_layout;
    vk_res = vkCreatePipelineLayout(device , &pipeline_layout_info , VK_NULL_HANDLE , &pipeline_layout);
    assert(VK_SUCCESS == vk_res);
    VkGraphicsPipelineCreateInfo pipeline_info = 
    {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        shader_stages.size(),
        shader_stages.data(),
        &shader_vertex_input_create_info,
        &shader_vertex_input_assembly_info,
        VK_NULL_HANDLE,
        &viewport_stage_info,
        &rasterization_stage_info,
        &multisample_stage_info,
        VK_NULL_HANDLE,
        &blend_stage_info,
        VK_NULL_HANDLE,
        pipeline_layout,
        render_pass,
        0,
        VK_NULL_HANDLE,
        0
    };

    VkPipeline pipelines;
    vk_res = vkCreateGraphicsPipelines(device , VK_NULL_HANDLE , 1 , &pipeline_info , VK_NULL_HANDLE , &pipelines);
    assert(VK_SUCCESS == vk_res);
     cout << "SUP";


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

    VkSemaphore semaphore_image_available;
    VkSemaphore semaphore_rendering_finished;
    vk_res = vkCreateSemaphore(device , &semaphore_info , VK_NULL_HANDLE , &semaphore_image_available);
    assert(VK_SUCCESS == vk_res);
    vk_res = vkCreateSemaphore(device , &semaphore_info , VK_NULL_HANDLE , &semaphore_rendering_finished);
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

    VkImageMemoryBarrier barrier_from_present_to_draw =
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

    VkImageMemoryBarrier barrier_from_draw_to_present =
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

    VkClearValue clear_color = 
    {
        { 1.0f, 0.8f, 0.4f, 0.0f }
    };

    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,                // VkStructureType              sType
      VK_NULL_HANDLE,                                      // const void                  *pNext
      0,                                            // uint32_t                     waitSemaphoreCount
      &semaphore_image_available,                               // const VkSemaphore           *pWaitSemaphores
      &wait_dst_stage_mask,                         // const VkPipelineStageFlags  *pWaitDstStageMask;
      1,                                            // uint32_t                     commandBufferCount
      &cmd_buffer,                                  // const VkCommandBuffer       *pCommandBuffers
      1,                                            // uint32_t                     signalSemaphoreCount
      &semaphore_rendering_finished            // const VkSemaphore           *pSignalSemaphores
    };

    VkResult vk_present_res;
    VkPresentInfoKHR present_info = 
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        VK_NULL_HANDLE,
        1,
        &semaphore_rendering_finished,
        1,
        &swapchain,
        &swapchain_available_image_index,
        &vk_present_res
    };

    VkRenderPassBeginInfo render_pass_begin_info = 
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,     // VkStructureType                sType
        nullptr,                                      // const void                    *pNext
        render_pass,                            // VkRenderPass                   renderPass
        frame_buffers[0],                       // VkFramebuffer                  framebuffer
        {                                             // VkRect2D                       renderArea
          {                                           // VkOffset2D                     offset
            0,                                          // int32_t                        x
            0                                           // int32_t                        y
          },
          window_properties.getDimensionExtent()
        },
        1,                                            // uint32_t                       clearValueCount
        &clear_color                                // const VkClearValue            *pClearValues
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

        vkCmdPipelineBarrier(cmd_buffer , VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_draw );

        vkCmdBeginRenderPass( cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

        vkCmdBindPipeline( cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines );

        vkCmdDraw( cmd_buffer , 3, 1, 0, 0 );

        vkCmdEndRenderPass( cmd_buffer );

        vkCmdPipelineBarrier(cmd_buffer , VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_draw_to_present );

        vk_res = vkEndCommandBuffer(cmd_buffer);
        assert(VK_SUCCESS == vk_res);

        vk_res = vkAcquireNextImageKHR(device , swapchain , UINT64_MAX , semaphore_image_available , VK_NULL_HANDLE , &swapchain_available_image_index);
        assert(VK_SUCCESS == vk_res);
        
        
        vkQueueSubmit(queue , 1 , &submit_info , VK_NULL_HANDLE );


        vkQueuePresentKHR(queue , &present_info);

        barrier_from_present_to_draw.image = barrier_from_draw_to_present.image = barrier_from_draw_to_present.image == swapchain_images[0] ? swapchain_images[1] : swapchain_images[0];  
        render_pass_begin_info.framebuffer = render_pass_begin_info.framebuffer == frame_buffers[0] ? frame_buffers[1] : frame_buffers[0]; 
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
        // cout << ms.count() - now << endl;
        now = ms.count();
    }

    glfwTerminate();

    return 0;
}   