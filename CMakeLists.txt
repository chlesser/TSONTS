cmake_minimum_required(VERSION 3.20)
project(Test VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable + glad implementation
add_executable(Test
    src/main.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui.cpp
    src/glad.c
)

# Include folder for glad/GLFW headers
target_include_directories(Test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/imgui
)

# Link folder for glfw3.lib
target_link_directories(Test PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glfw/lib
)

# Link actual GLFW + system OpenGL
target_link_libraries(Test PRIVATE
    glfw3          # the glfw3.lib file
    opengl32       # Windows OpenGL loader
)