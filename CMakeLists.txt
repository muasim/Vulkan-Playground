cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 11)

project(IntelVulkan C CXX)
include_directories("${CMAKE_SOURCE_DIR}/include")
file(GLOB all_SRCS "${CMAKE_SOURCE_DIR}/src/*.cxx")
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
link_directories(${CMAKE_SOURCE_DIR}/lib)

# add the executable
add_executable(${PROJECT_NAME} ${all_SRCS})

include(FindVulkan)
if(Vulkan_FOUND)
include_directories(${Vulkan_INCLUDE_DIRS})
else()
message("Vulkan not found!")
endif()

find_library(GLFW3_LIB NAMES glfw3dll)
add_library(glfw3 SHARED IMPORTED)
set_target_properties(glfw3 PROPERTIES IMPORTED_LOCATION "${GLFW3_LIB}")
target_link_libraries(${PROJECT_NAME} ${GLFW3_LIB})
