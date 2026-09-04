cmake_minimum_required(VERSION 4.4.3)

project(Raytracing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/glad.c 
    src/shaders.cpp 
)

set_target_properties( ${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_include_directories( ${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_link_libraries( ${PROJECT_NAME} PRIVATE 
    glfw3 
    opengl32 
    gdi32 
    assimp
    z
)

#imgui

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b-docking
)

FetchContent_MakeAvailable(imgui)

target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)



target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backend)

option(STATIC_BUILD "Statically build the executable" OFF)

if (STATIC_BUILD)
    target_link_options ( ${PROJECT_NAME} PRIVATE 
        -static
        -static-libgcc 
        -static-libstdc++
    )
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    message(STATUS "Building statically-linked executable")
else()
    message(STATUS "Building dynamically-linked executable")
endif()