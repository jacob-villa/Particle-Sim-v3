cmake_minimum_required(VERSION 3.10)

project(STDISCM_PROJECT_1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

find_package(OpenGL REQUIRED)

# Specify GLEW paths
set(GLEW_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/glew-2.1.0/include)
set(GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/glew-2.1.0/lib/Release/x64/glew32.lib)

# Specify GLFW paths
set(GLFW_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/glfw-3.3.9.bin.WIN64/include)
set(GLFW_LIBRARIES ${PROJECT_SOURCE_DIR}/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3.lib)

add_executable(${PROJECT_NAME} Particle-Sim.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/include 
    ${GLEW_INCLUDE_DIRS} 
    ${GLFW_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/glm
    ${PROJECT_SOURCE_DIR}/imgui
    ${PROJECT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenGL::GL 
    ${GLEW_LIBRARIES} 
    ${GLFW_LIBRARIES}
)

# ImGui
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/imgui/backends)
target_sources(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
    ${PROJECT_SOURCE_DIR}/imgui/imgui_widgets.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/vendor/include)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)

