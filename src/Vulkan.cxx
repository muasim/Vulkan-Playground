#include "Vulkan.hxx"

#include <assert.h>
#include <chrono>
#include <array>
#include <functional>

#include "VulkanFunctions.hxx"
#include "utils.hxx"

void Vulkan::getVulkanFunctions()
{
    #define VK_GLOBAL_FUNCTION( fun ) fun = (PFN_##fun)glfwGetInstanceProcAddress( NULL , #fun ); assert( fun );
    #include "VulkanFunctions.inl"
}
void Vulkan::getVulkanFunctions(VkInstance instance)
{
    #define VK_INSTANCE_FUNCTION( fun ) fun = (PFN_##fun)glfwGetInstanceProcAddress( instance , #fun ); assert( fun );
    #include "VulkanFunctions.inl"
}
void Vulkan::getVulkanFunctions(VkDevice device)
{
    #define VK_DEVICE_FUNCTION( fun ) fun = (PFN_##fun)vkGetDeviceProcAddr( device , #fun ); assert( fun );
    #include "VulkanFunctions.inl"
}
const std::vector<Vulkan::PhysicalDevice>& Vulkan::getPhysicalDevices(void) const { return this->physical_devices; }
const Vulkan::PhysicalDevice& Vulkan::getCurrentPhysicalDevice(void) const { return *this->current_physical_device; }
const Vulkan::SwapchainKHR& Vulkan::getSwapchainKHR(void) const { return this->swapchain; }

const VkPhysicalDevice& Vulkan::PhysicalDevice::getDevice(void) const  { return this->device; }
const VkPhysicalDeviceMemoryProperties& Vulkan::PhysicalDevice::getDeviceMemoryProperties(void) 
{
    // If it is not initializied;
    if(0 == this->memory_properties.memoryHeapCount)
    {
        vkGetPhysicalDeviceMemoryProperties(this->device , &this->memory_properties);
    }
    return this->memory_properties;
}
const VkPhysicalDeviceProperties& Vulkan::PhysicalDevice::getDeviceProperties(void) const { return this->properties; }
const std::vector<VkQueueFamilyProperties>& Vulkan::PhysicalDevice::getQueueFamilyProperties(void) const { return this->queue_family_properties; }
const uint32_t& Vulkan::PhysicalDevice::getQueueFamilyIndex(void) const { return this->queue_family_index; }

const VkDevice& Vulkan::LogicalDevice::getDevice(void) const { return this->logical_device; }

const VkSurfaceFormatKHR& Vulkan::SwapchainKHR::getPreferedSurfaceFormat(void) const { return this->prefered_surface_format; }
const std::vector<VkSurfaceFormatKHR>& Vulkan::SwapchainKHR::getSurfaceFormats(void) const { return this->formats; }

Vulkan::Vulkan(VkApplicationInfo& application_info)
{
    uint32_t glfw_res;

    glfw_res = glfwInit();
    assert(GLFW_TRUE == glfw_res);

    glfw_res = glfwVulkanSupported();
    assert(GLFW_TRUE == glfw_res);
    
    std::cout << &vkCreateInstance << std::endl;
    this->getVulkanFunctions();	
    std::cout << &vkCreateInstance << std::endl;

	std::vector<char*> instance_layers;
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");

	uint32_t instance_extension_count;
    const char ** instance_extensions_c = glfwGetRequiredInstanceExtensions(&instance_extension_count);
    
	std::vector<char*> instance_extensions;
	for (uint32_t i = 0; i < instance_extension_count; i++)
	{
		instance_extensions.push_back(const_cast<char*>(instance_extensions_c[i]));
	}
	instance_extensions.push_back("VK_EXT_debug_report");
    	
	this->application_info = application_info;
	VkInstanceCreateInfo instance_info = 
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        &application_info,
        1,
        &instance_layers[0],
        instance_extensions.size(),
        &instance_extensions[0]
    };

    VkResult vk_res = vkCreateInstance( &instance_info, VK_NULL_HANDLE, &this->instance);
    assert(VK_SUCCESS == vk_res);
    getVulkanFunctions(this->instance);

    uint32_t physical_device_count;
    vk_res = vkEnumeratePhysicalDevices(this->instance , &physical_device_count , VK_NULL_HANDLE  );
    assert(VK_SUCCESS == vk_res);

    std::vector<VkPhysicalDevice> devices(physical_device_count);
    vk_res = vkEnumeratePhysicalDevices(this->instance , &physical_device_count , devices.data() );
    assert(VK_SUCCESS == vk_res);
    
    this->physical_devices.resize(physical_device_count);
    for(uint32_t i = 0; i < physical_device_count; i++)
    {
        this->physical_devices[i].device = devices[i];
        vkGetPhysicalDeviceProperties(this->physical_devices[i].getDevice() , &this->physical_devices[i].properties );
    }
}

void Vulkan::createLogicalDevice(uint32_t physical_device_index)
{
    uint32_t queue_family_count;
    PhysicalDevice * const physical_device = &this->physical_devices[physical_device_index];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device->getDevice(),
                                                                           &queue_family_count,
                                                                           VK_NULL_HANDLE);
    assert(0 != queue_family_count);

    physical_device->queue_family_properties.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device->getDevice(),
                                                                           &queue_family_count,
                                                                           physical_device->queue_family_properties.data());
    assert(0 != physical_device->getQueueFamilyProperties()[0].queueCount);



    physical_device->queue_family_index = UINT32_MAX;
    for(uint32_t queue_family_index = 0; queue_family_index < queue_family_count; queue_family_index++)
    {
        if(physical_device->queue_family_properties[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            if(GLFW_TRUE == glfwGetPhysicalDevicePresentationSupport(this->instance , physical_device->getDevice() , queue_family_index ))
            {
                physical_device->queue_family_index = queue_family_index;
                break;
            }
                physical_device->queue_family_index = queue_family_index;
        } 
    }
    assert(UINT32_MAX != physical_device->queue_family_index);

    std::vector<float> queue_priorities(physical_device->getQueueFamilyProperties()[physical_device->queue_family_index].queueCount);
    for(int32_t queue_index = queue_priorities.size() - 1; queue_index >= 0 ; queue_index--)
    {
        queue_priorities[queue_index] = queue_index  / queue_priorities.size(); 
    }
    std::vector<const char*> extensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
    
    VkDeviceQueueCreateInfo queue_info = 
    {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        physical_device->getQueueFamilyIndex(),
        physical_device->getQueueFamilyProperties()[physical_device->getQueueFamilyIndex()].queueCount,
        queue_priorities.data()
    };
    VkDeviceCreateInfo device_info =
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &queue_info,
        0,
        VK_NULL_HANDLE,
        extensions.size(),
        extensions.data(),
        VK_NULL_HANDLE
    };
    VkResult vk_res = vkCreateDevice(physical_device->getDevice() , &device_info , VK_NULL_HANDLE , &this->logical_device.logical_device);
    assert(VK_SUCCESS == vk_res);

    this->current_physical_device = &this->physical_devices[physical_device_index];
}
void Vulkan::createSwapchain(GLFWwindow * window , uint32_t width , uint32_t height)
{
    this->getVulkanFunctions(this->logical_device.getDevice());
    this->swapchain.hwindow = window;
    this->swapchain.swapchain_image_size.width = width;
    this->swapchain.swapchain_image_size.height = height;
    VkResult vk_res = glfwCreateWindowSurface(this->instance , window , VK_NULL_HANDLE , &this->logical_device.surface);
    assert(VK_SUCCESS == vk_res);

    vk_res = glfwCreateWindowSurface(this->instance , window , VK_NULL_HANDLE , &this->swapchain.surface);
    assert(VK_SUCCESS == vk_res);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->getCurrentPhysicalDevice().getDevice(), this->getSwapchainKHR().surface , &this->swapchain.surface_capabilities);


    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(this->getCurrentPhysicalDevice().getDevice() , 
                                                                     this->getSwapchainKHR().surface,
                                                                     &surface_format_count,
                                                                     VK_NULL_HANDLE );

    this->swapchain.formats.resize(surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(this->getCurrentPhysicalDevice().getDevice() , 
                                                                     this->getSwapchainKHR().surface,
                                                                     &surface_format_count,
                                                                     this->swapchain.formats.data());

    this->swapchain.prefered_surface_format = getSwapChainFormat(this->swapchain.getSurfaceFormats());

    VkSwapchainCreateInfoKHR swapchain_info = 
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        VK_NULL_HANDLE,
        0,
        this->getSwapchainKHR().surface,
        2,
        this->getSwapchainKHR().getPreferedSurfaceFormat().format,
        this->getSwapchainKHR().getPreferedSurfaceFormat().colorSpace,
        {
            width,
            height
        },
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        1,
        &this->current_physical_device->queue_family_index,
        this->getSwapchainKHR().surface_capabilities.currentTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR 
            ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
            : this->getSwapchainKHR().surface_capabilities.currentTransform,
        getSupportedSwapchainFlagBit( this->getSwapchainKHR().surface_capabilities.supportedCompositeAlpha),
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_TRUE,
        VK_NULL_HANDLE
    };
    vkCreateSwapchainKHR(this->logical_device.getDevice() , &swapchain_info , VK_NULL_HANDLE , &this->swapchain.swapchain);

	this->swapchain.images.resize(swapchain_info.minImageCount);
    vkGetSwapchainImagesKHR(this->logical_device.getDevice() , this->swapchain.swapchain , &swapchain_info.minImageCount , this->swapchain.images.data());

    VkImageSubresourceRange subresource_range = 
    {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1
    };

    VkImageViewCreateInfo swapchain_imageview_info = 
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE,
        VK_IMAGE_VIEW_TYPE_2D,
        swapchain_info.imageFormat,
        {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        },
        subresource_range
    };
	this->swapchain.image_views.resize(swapchain_info.minImageCount);
    for(uint32_t i = 0; i < swapchain_info.minImageCount; i++)
    {
		swapchain_imageview_info.image = this->swapchain.images[i];
		vk_res = vkCreateImageView(this->logical_device.getDevice() , &swapchain_imageview_info , VK_NULL_HANDLE , &this->swapchain.image_views[i]);
		assert(VK_SUCCESS == vk_res);
    }
}

void Vulkan::createRenderPass()
{
    VkAttachmentDescription description =
    {
        0,
        this->swapchain.getPreferedSurfaceFormat().format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference attachment_reference =
    {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass_description =
    {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        VK_NULL_HANDLE,
        1,
        &attachment_reference,
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
        &description,
        1,
        &subpass_description,
        0,
        VK_NULL_HANDLE
    };
    VkResult vk_res = vkCreateRenderPass(this->logical_device.getDevice() , &render_pass_info , VK_NULL_HANDLE , &this->logical_device.render_pass);
    assert(VK_SUCCESS == vk_res);

    VkFramebufferCreateInfo frame_buffer_info =
    {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        this->logical_device.render_pass,
        1,
        VK_NULL_HANDLE,
        this->getSwapchainKHR().swapchain_image_size.width,
        this->getSwapchainKHR().swapchain_image_size.height,
        1
    };
    
    this->logical_device.frame_buffers.resize(this->getSwapchainKHR().images.size());
    for(uint32_t i = 0; i < this->getSwapchainKHR().images.size(); i++)
    {
        frame_buffer_info.pAttachments = &this->getSwapchainKHR().image_views[i];
        vk_res = vkCreateFramebuffer(this->logical_device.getDevice() , &frame_buffer_info , VK_NULL_HANDLE , &this->logical_device.frame_buffers[i] );
        assert(VK_SUCCESS == vk_res);
    }
}

void Vulkan::createGraphicsPipeline(std::string source_dir_path , uint32_t width , uint32_t height)
{
    std::vector<char> shader_binary = GetBinaryFileContents(std::string(source_dir_path).append("\\resources\\shaders\\shader.vert.spv"));
    VkShaderModuleCreateInfo shader_module_info = 
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        shader_binary.size(),
        reinterpret_cast<const uint32_t*>(shader_binary.data())
    };

    VkShaderModule shader_vertex_module;
    VkResult vk_res = vkCreateShaderModule(this->logical_device.getDevice() , &shader_module_info , VK_NULL_HANDLE , &shader_vertex_module);
    assert(VK_SUCCESS == vk_res);

    shader_binary = GetBinaryFileContents(std::string(source_dir_path).append("\\resources\\shaders\\shader.frag.spv"));
    shader_module_info.codeSize = shader_binary.size();
    shader_module_info.pCode = reinterpret_cast<const uint32_t*>(shader_binary.data());

    VkShaderModule shader_frag_module;
    vk_res = vkCreateShaderModule(this->logical_device.getDevice() , &shader_module_info , VK_NULL_HANDLE , &shader_frag_module);
    assert(VK_SUCCESS == vk_res);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
    shader_stages[0].sType = shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].pNext = shader_stages[1].pNext = VK_NULL_HANDLE;
    shader_stages[0].flags = shader_stages[1].flags = 0;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = shader_vertex_module;
    std::string shader_vertex_name("main"); 
    shader_stages[0].pName =  shader_vertex_name.c_str();
    shader_stages[0].pSpecializationInfo = VK_NULL_HANDLE;

    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = shader_frag_module;
    std::string shader_frag_name("main"); 
    shader_stages[1].pName =  shader_frag_name.c_str();
    shader_stages[1].pSpecializationInfo = VK_NULL_HANDLE;
    
    float vertex_data[] =
    {
        -0.7f, -0.7f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f,

        -0.7f, 0.7f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f,

        0.7f, -0.7f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,

        0.7f, 0.7f, 0.0f, 1.0f,
        0.3f, 0.3f, 0.3f, 0.0f
    };

    this->logical_device.vertex_buffer = 
        this->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                      sizeof(vertex_data));

    this->staging_buffer = 
        this->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                      4000);


    void *staging_buffer_memory_pointer;
    vk_res = vkMapMemory(this->logical_device.getDevice() , this->staging_buffer.memory , 0 , sizeof(vertex_data) , 0 , &staging_buffer_memory_pointer);
    assert(VK_SUCCESS == vk_res);

    memcpy(staging_buffer_memory_pointer , vertex_data , sizeof(vertex_data));

    VkMappedMemoryRange flush_range = 
    {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        VK_NULL_HANDLE,
        this->staging_buffer.memory,
        0,
        sizeof(vertex_data),
    };
    vk_res = vkFlushMappedMemoryRanges(this->logical_device.getDevice() , 1 , &flush_range);
    assert(VK_SUCCESS == vk_res);
    vkUnmapMemory(this->logical_device.getDevice() , this->staging_buffer.memory);

    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions = 
    {
        {
            0,
            sizeof(float) * 8,
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };
    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions = 
    {
        {
            0,
            vertex_binding_descriptions[0].binding,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            0
        },
        {
            1,
            vertex_binding_descriptions[0].binding,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            sizeof(float) * 4
        }
    };
    VkPipelineVertexInputStateCreateInfo shader_vertex_input_create_info =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        static_cast<uint32_t>(vertex_binding_descriptions.size()),
        &vertex_binding_descriptions[0],
        static_cast<uint32_t>(vertex_attribute_descriptions.size()),
        &vertex_attribute_descriptions[0]
    };
    VkPipelineInputAssemblyStateCreateInfo shader_vertex_input_assembly_info =
    {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        VK_FALSE
    };

    VkViewport viewport = 
    {
        0,
        0,
        width,
        height,
        0.0f,
        0.0f
    };

    VkRect2D scissor = 
    {
        VkOffset2D {0 , 0},
        {
            width,
            height
        }
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
    vk_res = vkCreatePipelineLayout(this->logical_device.getDevice() , &pipeline_layout_info , VK_NULL_HANDLE , &pipeline_layout);
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
        this->logical_device.render_pass,
        0,
        VK_NULL_HANDLE,
        0
    };
    vk_res = vkCreateGraphicsPipelines(this->logical_device.getDevice() , VK_NULL_HANDLE , 1 , &pipeline_info , VK_NULL_HANDLE , &this->logical_device.pipeline);
    assert(VK_SUCCESS == vk_res);
}

void Vulkan::createCommandBuffer()
{
    VkCommandPoolCreateInfo cmd_pool_info = 
    {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        VK_NULL_HANDLE,
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ,
        this->getCurrentPhysicalDevice().queue_family_index
    };

    VkCommandPool cmd_pool;
    VkResult vk_res = vkCreateCommandPool(this->logical_device.logical_device , &cmd_pool_info , VK_NULL_HANDLE , &cmd_pool);
    assert(VK_SUCCESS == vk_res);

    VkCommandBufferAllocateInfo cmd_buffer_info =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        VK_NULL_HANDLE,
        cmd_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        this->swapchain.surface_capabilities.minImageCount + 1 // +1 for staging_buffer
    };

	this->logical_device.cmd_buffers.resize(cmd_buffer_info.commandBufferCount);
    vk_res = vkAllocateCommandBuffers(this->logical_device.logical_device , &cmd_buffer_info , this->logical_device.cmd_buffers.data());
    assert(VK_SUCCESS == vk_res);

    // Prepare command buffer to copy data from staging buffer to a vertex buffer
    VkCommandBufferBeginInfo command_buffer_begin_info = 
    {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      nullptr,
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      nullptr
    };

    VkCommandBuffer command_buffer = this->logical_device.cmd_buffers.back();

    vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info);

    VkBufferCopy buffer_copy_info = {
      0,
      0,
      this->logical_device.vertex_buffer.size
    };
    vkCmdCopyBuffer( command_buffer, this->staging_buffer.buffer, this->logical_device.vertex_buffer.buffer, 1, &buffer_copy_info );

    VkBufferMemoryBarrier buffer_memory_barrier = {
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      nullptr,
      VK_ACCESS_MEMORY_WRITE_BIT,
      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      this->logical_device.vertex_buffer.buffer, 
      0,
      VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier( command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &buffer_memory_barrier, 0, nullptr );

    vkEndCommandBuffer( command_buffer );

    // Submit command buffer and copy data from staging buffer to a vertex buffer
    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      nullptr,
      0,
      nullptr,
      nullptr,
      1,
      &command_buffer,
      0,
      nullptr
    };

    VkQueue queue;
    vkGetDeviceQueue(this->logical_device.logical_device , this->getCurrentPhysicalDevice().queue_family_index , 0 , &queue );

    vk_res = vkQueueSubmit( queue , 1, &submit_info, VK_NULL_HANDLE );
    assert(VK_SUCCESS == vk_res);

    vkDeviceWaitIdle( this->logical_device.getDevice() );
}

int queued_count = 0;
void Vulkan::prepareRendering(GLFWwindow *window , uint32_t width , uint32_t height)
{
	VkResult vk_res;
    VkSemaphoreCreateInfo semaphore_info = 
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        VK_NULL_HANDLE,
        0
    };
    std::vector<VkSemaphore> semaphores_rendering_finished(this->swapchain.surface_capabilities.minImageCount);
	for (size_t i = 0; i < this->swapchain.surface_capabilities.minImageCount; i++)
	{
		vk_res = vkCreateSemaphore(this->logical_device.logical_device , &semaphore_info , VK_NULL_HANDLE , &semaphores_rendering_finished[i]);
		assert(VK_SUCCESS == vk_res);
	}

	VkSemaphore semaphore_image_acquired;
	vk_res = vkCreateSemaphore(this->logical_device.logical_device, &semaphore_info, VK_NULL_HANDLE, &semaphore_image_acquired);
	assert(VK_SUCCESS == vk_res);


    VkQueue queue;
    vkGetDeviceQueue(this->logical_device.logical_device , this->getCurrentPhysicalDevice().queue_family_index , 0 , &queue );

    uint32_t swapchain_available_image_index;

    VkCommandBufferBeginInfo cmd_buffer_begin_info =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        VK_NULL_HANDLE,
		0 ,
        VK_NULL_HANDLE
    };

    VkClearValue clear_color = 
    {
        { 1.0f, 0.8f, 0.4f, 0.0f }
    };

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      VK_NULL_HANDLE,
      1,
      &semaphore_image_acquired,
      &wait_dst_stage_mask,
      1,
      &this->logical_device.cmd_buffers[0],
      1,
      &semaphores_rendering_finished[0]
    };

    VkResult vk_present_res;
    VkPresentInfoKHR present_info = 
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        VK_NULL_HANDLE,
        1,
        &semaphores_rendering_finished[0],
        1,
        &this->swapchain.swapchain,
        &swapchain_available_image_index,
        &vk_present_res
    };

    VkRenderPassBeginInfo render_pass_begin_info = 
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        VK_NULL_HANDLE,
        this->logical_device.render_pass,
        this->logical_device.frame_buffers[0],
        {
          {
            0,
            0
          },
          {
              width,
              height
          }
        },
        1,
        &clear_color
    };
    long long now = (std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
    while (!glfwWindowShouldClose(window))
    {
        if( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }
        glfwPollEvents();

        vk_res = vkAcquireNextImageKHR(this->logical_device.logical_device , this->swapchain.swapchain , UINT64_MAX , semaphore_image_acquired , VK_NULL_HANDLE , &swapchain_available_image_index);
        assert(VK_SUCCESS == vk_res);

		VkCommandBuffer *cmd_buffer = &this->logical_device.cmd_buffers[swapchain_available_image_index];
		if (queued_count != this->swapchain.surface_capabilities.minImageCount)
		{			
			vkBeginCommandBuffer(*cmd_buffer , &cmd_buffer_begin_info);

			render_pass_begin_info.framebuffer = this->logical_device.frame_buffers[swapchain_available_image_index];

			vkCmdBeginRenderPass(*cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

			vkCmdBindPipeline(*cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->logical_device.pipeline );
        
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(*cmd_buffer, 0, 1, &this->logical_device.vertex_buffer.buffer, &offset ); 
        
			vkCmdDraw(*cmd_buffer, 4 , 1, 0, 0 );

			vkCmdEndRenderPass(*cmd_buffer);

			vk_res = vkEndCommandBuffer(*cmd_buffer);
			assert(VK_SUCCESS == vk_res);
			queued_count += 1;
		}
		submit_info.pCommandBuffers = cmd_buffer;
		submit_info.pSignalSemaphores = &semaphores_rendering_finished[swapchain_available_image_index];
		vkQueueWaitIdle(queue);
        vk_res = vkQueueSubmit(queue , 1 , &submit_info ,  VK_NULL_HANDLE );
        assert(VK_SUCCESS == vk_res);
        
		present_info.pImageIndices = &swapchain_available_image_index;
		present_info.pWaitSemaphores = &semaphores_rendering_finished[swapchain_available_image_index];
        vk_res = vkQueuePresentKHR(queue , &present_info);
		assert(VK_SUCCESS == vk_res);

        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch());
        now = ms.count();
    }
}

Vulkan::Buffer Vulkan::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memory_property , uint64_t data_size)
{
    Vulkan::Buffer buffer;
    buffer.size = data_size;
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        data_size,
        usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };
    VkResult vk_res = vkCreateBuffer( this->logical_device.getDevice(), &buffer_create_info, nullptr, &buffer.buffer );
    assert(VK_SUCCESS == vk_res);

    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements( this->logical_device.getDevice() , buffer.buffer, &buffer_memory_requirements );

	vk_res = VK_RESULT_MAX_ENUM; //Trash value
    VkPhysicalDeviceMemoryProperties memory_properties = this->current_physical_device->getDeviceMemoryProperties(); 
    for( uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i ) 
    {
        if( (buffer_memory_requirements.memoryTypeBits & (1 << i)) &&
            ((memory_properties.memoryTypes[i].propertyFlags & memory_property) == memory_property) ) 
            {
                VkMemoryAllocateInfo memory_allocate_info = {
                    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    VK_NULL_HANDLE,
                    buffer_memory_requirements.size,
                    i
                };

            vk_res = vkAllocateMemory( this->logical_device.getDevice() , &memory_allocate_info, VK_NULL_HANDLE , &buffer.memory ); 
        }
    }
	assert(VK_SUCCESS == vk_res);

    vk_res = vkBindBufferMemory( this->logical_device.getDevice() , buffer.buffer , buffer.memory, 0 );
    assert(VK_SUCCESS == vk_res);

    return buffer;
}

Vulkan::~Vulkan()
{
    glfwTerminate();
    //vkDestroyDevice(this->logical_device.getDevice() , VK_NULL_HANDLE);
    //vkDestroyInstance(this->instance , VK_NULL_HANDLE);
}