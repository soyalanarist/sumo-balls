cmake_minimum_required(VERSION 3.16)
project(sumo_balls VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

# Compiler flags for different build types
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # Debug flags: enable sanitizers and debug symbols
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wpedantic")
    # Release flags: optimize aggressively
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
elseif(MSVC)
    # MSVC flags
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /W4")
endif()

include(FetchContent)

# Graphics and windowing
find_package(SDL2 CONFIG REQUIRED)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)

# Fetch ImGui backends (SDL2 + SDLRenderer2)
FetchContent_Declare(
    imgui_backends
    URL https://raw.githubusercontent.com/ocornut/imgui/v1.89.9/backends/imgui_impl_sdl2.h
    DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(imgui imgui_backends)

# Fetch ENet for authoritative networking
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG master
)
FetchContent_MakeAvailable(enet)

add_executable(sumo_balls
    src/main.cpp
    src/core/Game.cpp
    src/core/Screen.cpp
    src/core/ScreenStack.cpp
    src/core/Settings.cpp
    src/core/SettingsManager.cpp
    src/core/GraphicsContext.cpp
    src/core/ImGuiManager.cpp

    src/screens/GameScreen.cpp
    src/screens/GameEndedScreen.cpp
    src/screens/WelcomeScreen.cpp
    src/screens/FriendsScreen.cpp
    src/screens/LobbyScreen.cpp
    src/screens/menus/MainMenu.cpp
    src/screens/menus/OptionsMenu.cpp
    src/screens/menus/PauseMenu.cpp
    src/screens/menus/GameOverMenu.cpp

    src/game/controllers/HumanController.cpp

    src/network/NetCommon.cpp
    src/network/NetServer.cpp
    src/network/NetClient.cpp

    src/systems/InputSystem.cpp
    
    # ImGui implementation files
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    
    # ImGui backends for SDL2
    include/imgui/backends/imgui_impl_sdl2.cpp
    include/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(sumo_balls PRIVATE include)
target_include_directories(sumo_balls PRIVATE include/imgui/backends)
target_include_directories(sumo_balls PRIVATE ${enet_SOURCE_DIR}/include)
target_include_directories(sumo_balls PRIVATE ${imgui_SOURCE_DIR})
target_include_directories(sumo_balls PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(sumo_balls
    SDL2::SDL2
    enet
)

add_executable(sumo_balls_server
    src/server_main.cpp
    src/simulation/Simulation.cpp
    src/network/NetCommon.cpp
    src/network/NetServer.cpp
)

target_include_directories(sumo_balls_server PRIVATE include)
target_include_directories(sumo_balls_server PRIVATE ${enet_SOURCE_DIR}/include)

target_link_libraries(sumo_balls_server
    enet
)

if (MSVC)
    target_compile_options(sumo_balls_server PRIVATE /W4)
else()
    target_compile_options(sumo_balls_server PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (MSVC)
    target_compile_options(sumo_balls PRIVATE /W4)
else()
    target_compile_options(sumo_balls PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# TESTS
# ============================================================================

enable_testing()

add_executable(sumo_balls_test
    tests/TestRunner.cpp
    tests/test_physics.cpp
)

target_include_directories(sumo_balls_test PRIVATE include)
target_include_directories(sumo_balls_test PRIVATE src)

# Link SDL2 for tests that might need it
target_link_libraries(sumo_balls_test
    SDL2::SDL2
)

if (MSVC)
    target_compile_options(sumo_balls_test PRIVATE /W4)
else()
    target_compile_options(sumo_balls_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME sumo_balls_tests COMMAND sumo_balls_test)

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build: sanitizers enabled (-fsanitize=address,undefined)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release build: optimizations enabled (-O3 -march=native)")
endif()
