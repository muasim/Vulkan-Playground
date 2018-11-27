cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)

project(IntelVulkan C CXX)
include_directories("${CMAKE_SOURCE_DIR}/include")
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
link_directories(${LIBRARY_OUTPUT_PATH})
set(TOOLS_PATH ${CMAKE_SOURCE_DIR}/tools)
set(RESOURCES_PATH ${CMAKE_SOURCE_DIR}/resources CACHE FILEPATH "Filepath to the Resources folder." FORCE)
set(SHADERS_PATH ${RESOURCES_PATH}/shaders)

add_executable(${PROJECT_NAME} ${all_SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_sources(${PROJECT_NAME} PRIVATE 
src/Main.cxx
src/utils.cxx
src/VulkanFunctions.cxx)


include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

add_library(whereami STATIC ${CMAKE_SOURCE_DIR}/whereami/whereami++.cpp)
if(WIN32)
    target_compile_definitions( whereami PUBLIC _WIN32_WINNT=_WIN32_WINNT_WINXP  )
endif()
target_link_libraries(${PROJECT_NAME} whereami)
add_dependencies(${PROJECT_NAME} whereami)

add_subdirectory(ResourceInitializer)

set(GLFW_SRC_DIR "C:/Users/Lenovo/Documents/SdksNLibs/OpenGL/glfw")
set(GLFW_ARGS -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:STRING=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
              -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
              -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:STRING=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
              -DBUILD_SHARED_LIBS:BOOL=ON 
              -DGLFW_BUILD_EXAMPLES:BOOL=OFF 
              -DGLFW_BUILD_TESTS:BOOL=OFF 
              -DGLFW_BUILD_DOCS:BOOL=OFF 
              -DGLFW_VULKAN_STATIC:BOOL=OFF 
              -DGLFW_USE_HYBRID_HPG:BOOL=ON 
              -DGLFW_INSTALL:BOOL=OFF)

ExternalProject_Add(GLFW 
                    DOWNLOAD_COMMAND URL ${GLFW_SRC_DIR}
                    PREFIX ${CMAKE_SOURCE_DIR}/glfw
                    INSTALL_COMMAND ""
                    BINARY_DIR ${CMAKE_SOURCE_DIR}/glfw/bin
                    CMAKE_GENERATOR ${CMAKE_GENERATOR}
                    CMAKE_CACHE_ARGS ${GLFW_ARGS}
                    )

add_dependencies(${PROJECT_NAME} GLFW)
find_library(GLFW_LIB_PATH "glfw3dll" HINTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
add_library(GLFW_LIB SHARED IMPORTED)
set_target_properties(GLFW_LIB PROPERTIES IMPORTED_LOCATION "${GLFW_LIB_PATH}")
target_link_libraries(${PROJECT_NAME} ${GLFW_LIB} )


# set(SCRIPT_NAME ${CMAKE_SOURCE_DIR}/InitResources.cmake)

# set_target_properties(glfw3 PROPERTIES IMPORTED_LOCATION "${GLFW3_LIB}")

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



include(FindVulkan)
if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
else()
    message("Vulkan not found!")
endif()