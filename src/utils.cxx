#include "utils.hxx"
#include <iostream>
#include <fstream>

VkExtent2D WindowProperties::ext2d_dimension;
char* WindowProperties::title;
WindowProperties& WindowProperties::get_instance()
{
    static WindowProperties instance;
    return instance;
}
uint32_t WindowProperties::getHeight()
{
    return WindowProperties::ext2d_dimension.height;
}
void WindowProperties::setHeight(uint32_t height)
{
    WindowProperties::ext2d_dimension.height = height;
} 
uint32_t WindowProperties::getWidth()
{
    return WindowProperties::ext2d_dimension.width;
}
void WindowProperties::setWidth(uint32_t width)
{
    WindowProperties::ext2d_dimension.width = width;
} 
char* WindowProperties::getTitle()
{
    return WindowProperties::title;
}
void WindowProperties::setTitle(char* const title)
{
    WindowProperties::title = title;
}
VkExtent2D WindowProperties::getDimensionExtent()
{
    return WindowProperties::ext2d_dimension;
}
WindowProperties::WindowProperties() { }



VkSurfaceFormatKHR getSwapChainFormat( std::vector<VkSurfaceFormatKHR> &surface_formats ) 
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

VkImageUsageFlags getSwapChainUsageFlags( VkSurfaceCapabilitiesKHR &surface_capabilities ) {
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

VkCompositeAlphaFlagBitsKHR getSupportedSwapchainFlagBit(VkCompositeAlphaFlagsKHR &supported_flag_bits)
{
    // Find a supported composite alpha mode - one of these is guaranteed to be set
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++) 
    {
        if (supported_flag_bits & compositeAlphaFlags[i]) return compositeAlphaFlags[i];  
    }
}
#include <direct.h>
std::vector<char> GetBinaryFileContents( std::string const &filename ) 
{
    std::ifstream file( filename, std::ios::binary | std::ios::in );
    if( file.fail() ) 
    {

        std::cout << "Could not open \"" << filename << "\" file!" << std::endl;
        char* buffer;
        if( (buffer = _getcwd( NULL, 0 )) == NULL ) perror( "_getcwd error" );
        else
        {
            printf( "%s \n", buffer);
            delete[] buffer;
        }
        return std::vector<char>();
    }

    std::streampos begin, end;
    begin = file.tellg();
    file.seekg( 0, std::ios::end );
    end = file.tellg();
    std::vector<char> result( static_cast<size_t>(end - begin) );
    file.seekg( 0, std::ios::beg );
    file.read( &result[0], end - begin );
    file.close();

    return result;
}