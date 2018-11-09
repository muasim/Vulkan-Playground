#define VK_MAKE_VERSION(major,minor,patch) (((major) << 22) | ((minor) << 12) | (patch))
#define VK_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define VK_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define VK_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

#define NS_ARRAY_LENGTH(arr)  (sizeof(arr) / sizeof((arr)[0]))

#ifndef VULKAN_H_
    #define VK_NO_PROTOTYPES
    #include <vulkan/vulkan.h>
#endif 

#ifndef _VECTOR_
    #include <vector>
#endif

struct WindowProperties
{
    static WindowProperties& get_instance()
    {
        static WindowProperties instance;
        return instance;
    }
    uint16_t height;
    uint16_t width;
    char *title; 
    // The copy constructor is deleted, to prevent client code from creating new
    // instances of this class by copying the instance returned by get_instance()
    WindowProperties(WindowProperties const&) = delete;

    // The move constructor is deleted, to prevent client code from moving from
    // the object returned by get_instance(), which could result in other clients
    // retrieving a reference to an object with unspecified state.
    WindowProperties(WindowProperties&&) = delete;

private:

    // Default-constructor is private, to prevent client code from creating new
    // instances of this class. The only instance shall be retrieved through the
    // get_instance() function.
    WindowProperties() { }

};

VkSurfaceFormatKHR GetSwapChainFormat( std::vector<VkSurfaceFormatKHR> &surface_formats );
VkImageUsageFlags GetSwapChainUsageFlags( VkSurfaceCapabilitiesKHR &surface_capabilities );