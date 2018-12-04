#define GLFW_DLL 
#include <GLFW/glfw3.h>

#include <vector>
#include <list>
#include <iostream>

class Window
{
    public:
        struct Monitor
        {
            public:
            private:
                friend Window;
                GLFWmonitor* monitor;
                std::vector<const GLFWvidmode*> video_modes;
                const GLFWvidmode* current_video_mode;
        };
        class WindowProperties
        {
            private:
                friend Window;
                uint32_t width = UINT32_MAX;
                uint32_t height = UINT32_MAX;
                int32_t	position_x = INT32_MAX;
                int32_t	position_y = INT32_MAX;
                const Window::Monitor* owner = nullptr;
                bool fullscreen = false;
                std::string window_title;
            public:
                const uint32_t getWidth(void) const;
                const uint32_t getHeight(void) const;
                void setWindowSize(uint32_t width , uint32_t height);
                void setMonitor(const Window::Monitor* owner = nullptr);
        };
        GLFWwindow* createWindow(void);
        
        Window();
        ~Window();

        WindowProperties& getWindowProperties(void);
        static const std::vector<Monitor>& getMonitors(void);
        static const Monitor* getPrimaryMonitor(void);

    private:
        GLFWwindow *window;
        static std::vector<Monitor> monitors;
        static Monitor* primary_monitor;
        WindowProperties properties;
};