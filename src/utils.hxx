#define VK_MAKE_VERSION(major,minor,patch) (((major) << 22) | ((minor) << 12) | (patch))
#define VK_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define VK_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define VK_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

#define NS_ARRAY_LENGTH(arr)  (sizeof(arr) / sizeof((arr)[0]))

#ifndef VK_NO_PROTOTYPES
    #define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>

#include <vector>

struct WindowProperties
{
    static WindowProperties& get_instance();
    static uint32_t getHeight(); 
    static void setHeight(uint32_t height); 
    static uint32_t getWidth();
    static void setWidth(uint32_t width); 
    static char* getTitle();
    static void setTitle(char* const title);
    static VkExtent2D getDimensionExtent();

    // The copy constructor is deleted, to prevent client code from creating new
    // instances of this class by copying the instance returned by get_instance()
    WindowProperties(WindowProperties const&) = delete;

    // The move constructor is deleted, to prevent client code from moving from
    // the object returned by get_instance(), which could result in other clients
    // retrieving a reference to an object with unspecified state.
    WindowProperties(WindowProperties&&) = delete;

private:
    static VkExtent2D ext2d_dimension;
    static char *title; 

    // Default-constructor is private, to prevent client code from creating new
    // instances of this class. The only instance shall be retrieved through the
    // get_instance() function.
    WindowProperties();

};

VkSurfaceFormatKHR getSwapChainFormat( std::vector<VkSurfaceFormatKHR> &surface_formats );
VkImageUsageFlags getSwapChainUsageFlags( VkSurfaceCapabilitiesKHR &surface_capabilities );
VkCompositeAlphaFlagBitsKHR getSupportedSwapchainFlagBit(VkCompositeAlphaFlagsKHR &supported_flag_bits);