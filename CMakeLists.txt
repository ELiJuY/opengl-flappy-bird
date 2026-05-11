cmake_minimum_required(VERSION 3.7)
project(FlappyOpenGL)

set(CMAKE_CXX_STANDARD 17)

# -----------------------------
# Añadir framework
# -----------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error -Wno-unused-but-set-variable")
add_subdirectory(external/gpo-framework)

# -----------------------------
# Includes
# -----------------------------
include_directories(
    include/
    external/gpo-framework/include
    external/gpo-framework/libs/glad/include
    external/gpo-framework/libs/glm
    external/gpo-framework/libs/imgui
    external/gpo-framework/libs/assimp/include
    external/gpo-framework/include/stb
)

# -----------------------------
# Ejecutable
# -----------------------------
add_executable(flappy
    src/main.cpp
        external/gpo-framework/src/GPO_aux.cpp
        src/core/config.cpp
        include/core/config.h
        src/graphics/sky.cpp
        include/graphics/sky.h
        src/graphics/shaders.cpp
        include/graphics/shaders.h
        src/graphics/renderer.cpp
        include/graphics/renderer.h
        src/graphics/meshes.cpp
        include/graphics/meshes.h
        src/graphics/camera.cpp
        include/graphics/camera.h
        src/game/player.cpp
        include/game/player.h
        src/world/world.cpp
        include/world/world.h
        src/world/walls.cpp
        include/world/walls.h
        src/game/gameState.cpp
        include/game/gameState.h
        src/core/timing.cpp
        include/core/timing.h
        src/core/mathUtils.cpp
        include/core/mathUtils.h
        include/core/gpo_wrapper.h
        src/core/frame.cpp
        include/core/frame.h
        src/game/gameLoop.cpp
        include/game/gameLoop.h
        src/game/collisionSystem.cpp
        include/game/collisionSystem.h
        src/graphics/lighting.cpp
        include/graphics/lighting.h
        src/graphics/worldRenderer.cpp
        include/graphics/worldRenderer.h
        src/world/sun.cpp
        include/world/sun.h
        src/graphics/sunShaders.cpp
        include/graphics/sunShaders.h
        include/core/debug.h
        src/graphics/shadowMapping.cpp
        include/graphics/shadowMapping.h
        src/weather/weather.cpp
        include/weather/weather.h
        src/weather/fog.cpp
        include/weather/fog.h
        src/graphics/postprocess.cpp
        include/graphics/postprocess.h
        src/graphics/screenShader.cpp
        include/graphics/screenShader.h
)

target_compile_definitions(flappy PRIVATE GLM_ENABLE_EXPERIMENTAL)

if(ENABLE_GRAPHICS_DEBUG)
    add_compile_definitions(ENABLE_GRAPHICS_DEBUG)
endif()
# -----------------------------
# Link librerías del framework
# -----------------------------
target_link_libraries(flappy
    PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
    glm
    glad_interface
    assimp
)

# -----------------------------
# Copiar assets
# -----------------------------
file(COPY external/gpo-framework/data DESTINATION ${CMAKE_BINARY_DIR}/bin)
