cmake_minimum_required(VERSION 3.0)

# Project name
project(GraphicsProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/GraphicsProject")
add_definitions(-DPROJECT_ROOT="${PROJECT_ROOT}")

# Specify directories
set(SRC_DIR "${PROJECT_ROOT}/src")
set(INCLUDE_DIR "${PROJECT_ROOT}/external/include")
set(HEADER_DIR "${PROJECT_ROOT}/src/headers")
set(LIB_DIR "${PROJECT_ROOT}/external/lib")
set(IMGUI_DIR "${PROJECT_ROOT}/external/imgui")

# Manually specify ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create a static library for ImGui
add_library(imgui STATIC ${IMGUI_SOURCES})

# Add ImGui include directories
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Add other source files
set(SOURCES
    ${PROJECT_ROOT}/external/glad.c
    ${PROJECT_ROOT}/external/include/stbi/stbi.cpp
    ${SRC_DIR}/Main.cpp
     
    ${SRC_DIR}/Camera.cpp 
    ${SRC_DIR}/Shader.cpp
    ${SRC_DIR}/Mesh.cpp
    ${SRC_DIR}/Model.cpp
    ${SRC_DIR}/Skybox.cpp
    ${SRC_DIR}/Terrain.cpp
    ${SRC_DIR}/Generator.cpp
)

# Include directories
include_directories(${INCLUDE_DIR} ${HEADER_DIR})

# Add the executable
add_executable(GraphicsProject ${SOURCES}    "src/Boid.cpp")

# Link Dear ImGui and other libraries
target_link_libraries(GraphicsProject PRIVATE imgui "${LIB_DIR}/glfw3.lib" "${LIB_DIR}/assimp-vc143-mtd.lib" opengl32)

# Post-build command to copy necessary DLLs
add_custom_command(TARGET GraphicsProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_ROOT}/external/DLL/assimp-vc143-mtd.dll" $<TARGET_FILE_DIR:GraphicsProject>)

# Output binary to the 'out' directory
set_target_properties(GraphicsProject PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out
)

# Copy shaders/resources if needed (example, adapt as necessary)
# file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_SOURCE_DIR}/out)
