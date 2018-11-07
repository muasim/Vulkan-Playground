#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VK_GLOBAL_FUNCTION( fun) PFN_##fun fun;
#define VK_INSTANCE_FUNCTION( fun) PFN_##fun fun;
#define VK_DEVICE_FUNCTION( fun ) PFN_##fun fun;

#include "VulkanFunctions.inl"
