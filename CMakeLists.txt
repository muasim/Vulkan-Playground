cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)

project(IntelVulkan C CXX)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(DEPENDENCY_PATH ${CMAKE_SOURCE_DIR}/deps)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
link_directories(${LIBRARY_OUTPUT_PATH})
set(TOOLS_PATH ${CMAKE_SOURCE_DIR}/tools)
set(RESOURCES_PATH ${CMAKE_SOURCE_DIR}/resources CACHE FILEPATH "Filepath to the Resources folder." FORCE)
set(SHADERS_PATH ${RESOURCES_PATH}/shaders)

add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_sources(${PROJECT_NAME} PRIVATE 
src/Main.cxx
src/utils.cxx
src/VulkanFunctions.cxx
src/Vulkan.cxx
src/Window.cxx)

add_library(whereami STATIC ${DEPENDENCY_PATH}/whereami/src/whereami++.cpp)
#MinGW workaround to set minimum required windows version
if(WIN32)
target_compile_definitions( whereami PUBLIC _WIN32_WINNT=_WIN32_WINNT_WINXP)
endif()
target_link_libraries(${PROJECT_NAME} whereami)
add_dependencies(${PROJECT_NAME} whereami)
include_directories(${DEPENDENCY_PATH}/whereami/src)

set(BUILD_SHARED_LIBS ON CACHE BOOLEAN "Build Shared Files" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOLEAN "GLFW Example Option" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOLEAN "GLFW Tests Option" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOLEAN "GLFW Docs Option" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE BOOLEAN "GLFW Vulkan Static Option" FORCE)
set(GLFW_USE_HYBRID_HPG ON CACHE BOOLEAN "GLFW Force Hybrid Option" FORCE)
set(GLFW_INSTALL OFF CACHE BOOLEAN "GLFW Install Option" FORCE)

add_subdirectory(${DEPENDENCY_PATH}/GLFW)
target_link_libraries(${PROJECT_NAME} glfw)
set(BUILD_SHARED_LIBS OFF CACHE BOOLEAN "Build Shared Files" FORCE)
include_directories(${DEPENDENCY_PATH}/glfw/include)

include(FindVulkan)
if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
else()
    message("Vulkan not found!")
endif()

# file(GLOB all_SRCS "${CMAKE_SOURCE_DIR}/src/*.cxx")
# add the executable

# set(CMD_RUN_SCRIPT -P)
# set(SCRIPT_NAME ${CMAKE_SOURCE_DIR}/InitResources.cmake)
# set(SCRIPT_VARS cmake -DTOOLS_PATH=${TOOLS_PATH} -DRESOURCES_PATH=${RESOURCES_PATH} -DSHADERS_PATH=${SHADERS_PATH} -DBINARY_DIR=${CMAKE_BINARY_DIR} -DBUILD_CONFIG=$<CONFIG>)
# message(${CMAKE_BINARY_DIR})
# add_custom_target(InitResources ALL
#                   COMMAND ${SCRIPT_VARS} ${CMD_RUN_SCRIPT} ${SCRIPT_NAME}
#  COMMENT "Initializing Resources..."
# )
# add_dependencies(${PROJECT_NAME} InitResources)


