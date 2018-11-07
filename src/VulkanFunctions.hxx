
#define VK_GLOBAL_FUNCTION( fun) extern PFN_##fun fun;

#define VK_INSTANCE_FUNCTION( fun) extern PFN_##fun fun;

#define VK_DEVICE_FUNCTION( fun ) extern PFN_##fun fun;


#include "VulkanFunctions.inl"
