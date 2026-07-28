cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME redball4)
set(SERVER_NAME ${PROJECT_NAME}_server)
set(CLIENT_NAME ${PROJECT_NAME}_client)
project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(EnTT CONFIG REQUIRED)
find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Window System Audio)
find_package(GameNetworkingSockets CONFIG REQUIRED)

file(GLOB_RECURSE COMMON_SOURCES
    "src/common/*.cpp" "src/common/*.hpp"
    "meatnet/src/*.cpp" "include/meatnet/*.hpp"
)

file(GLOB_RECURSE CLIENT_SOURCES
    "src/client/*.cpp" "src/client/*.hpp"
)

file(GLOB_RECURSE SERVER_SOURCES
    "src/server/*.cpp" "src/server/*.hpp"
)


# Client
add_executable(${CLIENT_NAME}
    ${COMMON_SOURCES}
    ${CLIENT_SOURCES}
)

target_link_libraries(${CLIENT_NAME} PRIVATE 
    EnTT::EnTT
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    GameNetworkingSockets::GameNetworkingSockets
)


add_custom_command(TARGET ${CLIENT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${CLIENT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/res" "$<TARGET_FILE_DIR:${CLIENT_NAME}>/res"
    COMMENT "Resources copied successfully to build directory."
)
##

# Common
target_include_directories(${CLIENT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
#

# Server
add_executable(${SERVER_NAME}
    ${COMMON_SOURCES}
    ${SERVER_SOURCES}
)

target_link_libraries(${SERVER_NAME} PRIVATE 
    EnTT::EnTT
    SFML::System
    SFML::Graphics
    SFML::Audio
    GameNetworkingSockets::GameNetworkingSockets
)

add_custom_command(TARGET ${SERVER_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${SERVER_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/res" "$<TARGET_FILE_DIR:${SERVER_NAME}>/res"
    COMMENT "Resources copied successfully to build directory."
)
##