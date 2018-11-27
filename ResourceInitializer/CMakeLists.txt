project(ResInit C CXX)

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)
set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
message(${CMAKE_BINARY_DIR}) 
add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
               ${PROJECT_SOURCE_DIR}/main.cxx)

get_target_property(VAR ${PROJECT_NAME} SUFFIX)


add_custom_target(dummy_target ALL
    DEPENDS
        custom_output
        ${PROJECT_NAME}
    )

add_custom_command(
    OUTPUT custom_output
    COMMAND ${PROJECT_NAME} ${RESOURCES_PATH} ${CMAKE_BINARY_DIR}
    DEPENDS
        always_rebuild
    )
add_custom_command(
    OUTPUT always_rebuild
    COMMAND cmake -E echo
    )
 
    
# add_custom_command(TARGET ${PROJECT_NAME}
#                    POST_BUILD 
#                    COMMAND ${PROJECT_NAME} ${RESOURCES_PATH} ${CMAKE_BINARY_DIR})