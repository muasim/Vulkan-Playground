#include "Window.hxx"

#include <iostream>
#include <assert.h>

Window::Monitor* Window::primary_monitor;
std::vector<Window::Monitor> Window::monitors;

const Window::Monitor* Window::getPrimaryMonitor(void) { return Window::primary_monitor; }
const std::vector<Window::Monitor>& Window::getMonitors(void) { return Window::monitors; }
Window::WindowProperties& Window::getWindowProperties(void)  { return this->properties; }

const uint32_t Window::WindowProperties::getWidth(void) const { return this->width; }
const uint32_t Window::WindowProperties::getHeight(void) const { return this->height; }

void Window::WindowProperties::setWindowSize(const uint32_t width ,const uint32_t height)
{
    if(this->fullscreen)
    {
        for(auto& video_mode : this->owner->video_modes)
        {
            if(video_mode->width == width && video_mode->height == height)
            {
                this->width = width;
                this->height = height;
            }
        }
    }
    else if(this->owner->video_modes.back()->width > width && this->owner->video_modes.back()->height > height)
    {
        this->width = width;
        this->height = height;
    }
    // if(INT32_MAX != position_x) this->position_x = this->owner.current_video_mode->width - width / 2;
    // else this->position_x = position_x;
    // if(INT32_MAX != position_y)  this->position_y = this->owner.current_video_mode->height - height / 2;
    // else this->position_y = position_y;
}

void Window::WindowProperties::setMonitor(const Window::Monitor* owner)
{
    if(nullptr != owner)
    {
        this->owner = owner;
    }
    else this->owner = Window::primary_monitor;
}
Window::Window()
{
    int monitor_count;
    GLFWmonitor** monitors_c = glfwGetMonitors(&monitor_count);
    this->monitors.resize(monitor_count);
    
    GLFWmonitor* primary = glfwGetPrimaryMonitor();

    for(uint32_t i = 0 ; i < monitor_count; i++)
    {
        if(monitors_c[i] == primary) Window::primary_monitor = &monitors.at(i);
        this->monitors[i].monitor = monitors_c[i];
        
        int video_mode_count;
        const GLFWvidmode* video_modes_c = glfwGetVideoModes(monitors_c[i] , &video_mode_count);
        this->monitors[i].video_modes.resize(video_mode_count);
        for(uint32_t j = 0; j < video_mode_count; j++)
        {
            this->monitors[i].video_modes[j] = &video_modes_c[j];
        }
        this->monitors[i].current_video_mode = glfwGetVideoMode(monitors_c[i]);
    }    
}

GLFWwindow* Window::createWindow()
{
    GLFWmonitor *monitor = NULL;
    if(nullptr == this->properties.owner)this->properties.owner = Window::primary_monitor;
    if(this->properties.fullscreen)
    {
        if(UINT32_MAX == this->properties.width || UINT32_MAX == this->properties.height)
        {
            this->properties.width =  this->primary_monitor->current_video_mode->width;
            this->properties.height =  this->primary_monitor->current_video_mode->height;
        }
    }
    else
    {
            if(UINT32_MAX == this->properties.width)
            {
                this->properties.width = this->primary_monitor->video_modes[0]->width;
            }
            if(UINT32_MAX == this->properties.height)
            {
                this->properties.height = this->primary_monitor->video_modes[0]->height;
            }
            if(INT32_MAX == this->properties.position_x)
            {
                this->properties.position_x = (this->properties.owner->current_video_mode->width - this->properties.width) / 2;
            }
            if(INT32_MAX == this->properties.position_y)
            {
                this->properties.position_y = (this->properties.owner->current_video_mode->height - this->properties.height) / 2;
            }
    }
    
    std::cout << this->properties.width;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    this->window = glfwCreateWindow(this->properties.width,
                                                              this->properties.height,
                                                              "Vulkan Playground",
                                                              NULL,
                                                              NULL);
    assert(NULL != this->window);
    glfwSetWindowPos(this->window , this->properties.position_x , this->properties.position_y);

    return this->window;
}
Window::~Window()
{
    if(NULL != this->window) glfwDestroyWindow(this->window);
}