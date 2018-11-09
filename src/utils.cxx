#include "utils.hxx"
#include <iostream>

VkSurfaceFormatKHR GetSwapChainFormat( std::vector<VkSurfaceFormatKHR> &surface_formats ) 
{
    // If the list contains only one entry with undefined format
    // it means that there are no preferred surface formats and any can be chosen
    if( (surface_formats.size() == 1) &&
        (surface_formats[0].format == VK_FORMAT_UNDEFINED) ) {
        return{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
    }

    // Check if list contains most widely used R8 G8 B8 A8 format
    // with nonlinear color space
    for( VkSurfaceFormatKHR &surface_format : surface_formats ) {
        if( surface_format.format == VK_FORMAT_R8G8B8A8_UNORM ) {
        return surface_format;
        }
    }

    // Return the first format from the list
return surface_formats[0];
}

VkImageUsageFlags GetSwapChainUsageFlags( VkSurfaceCapabilitiesKHR &surface_capabilities ) {
    // Color attachment flag must always be supported
    // We can define other usage flags but we always need to check if they are supported
    if( surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) {
      return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    std::cout << "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT image usage is not supported by the swap chain!" << std::endl
      << "Supported swap chain's image usages include:" << std::endl
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT              ? "    VK_IMAGE_USAGE_TRANSFER_SRC\n" : "")
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT              ? "    VK_IMAGE_USAGE_TRANSFER_DST\n" : "")
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT                   ? "    VK_IMAGE_USAGE_SAMPLED\n" : "")
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT                   ? "    VK_IMAGE_USAGE_STORAGE\n" : "")
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT          ? "    VK_IMAGE_USAGE_COLOR_ATTACHMENT\n" : "")
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  ? "    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT\n" : "")
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT      ? "    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT\n" : "")
      << (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT          ? "    VK_IMAGE_USAGE_INPUT_ATTACHMENT" : "")
      << std::endl;
    return static_cast<VkImageUsageFlags>(-1);
  }