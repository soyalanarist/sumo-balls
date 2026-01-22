cmake_minimum_required(VERSION 3.16)
project(sumo_balls VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

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
    src/ui/UIElement.cpp
    src/ui/Button.cpp
    src/ui/ToggleSwitch.cpp

    src/screens/GameScreen.cpp
    src/screens/GameEndedScreen.cpp
    src/screens/AuthScreen.cpp
    src/screens/FriendsScreen.cpp
    src/screens/LobbyScreen.cpp
    src/screens/menus/MainMenu.cpp
    src/screens/menus/OptionsMenu.cpp
    src/screens/menus/PauseMenu.cpp
    src/screens/menus/GameOverMenu.cpp

    src/game/entities/Player.cpp
    src/game/entities/PlayerEntity.cpp
    src/game/entities/Arena.cpp

    src/game/controllers/HumanController.cpp
    src/game/controllers/AIController.cpp

    src/network/NetCommon.cpp
    src/network/NetServer.cpp
    src/network/NetClient.cpp
    src/network/GameNetworkManager.cpp

    src/game/GamePhysics.cpp

    src/ui/GameOverlay.cpp

    src/systems/InputSystem.cpp
)

target_include_directories(sumo_balls PRIVATE include)
target_include_directories(sumo_balls PRIVATE ${enet_SOURCE_DIR}/include)

target_link_libraries(sumo_balls
    sfml-graphics
    sfml-window
    sfml-system
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
    sfml-system
    enet
)

if (MSVC)
    target_compile_options(sumo_balls_server PRIVATE /W4 /O2)
else()
    target_compile_options(sumo_balls_server PRIVATE -Wall -Wextra -Wpedantic -O3 -march=native)
endif()

# Unit tests
add_executable(test_physics
    tests/test_physics.cpp
)

target_include_directories(test_physics PRIVATE include)
target_link_libraries(test_physics
    sfml-system
)

add_executable(test_ui
    tests/test_ui.cpp
    src/ui/UIElement.cpp
    src/ui/Button.cpp
)

target_include_directories(test_ui PRIVATE include)
target_link_libraries(test_ui
    sfml-graphics
    sfml-window
    sfml-system
)

if (MSVC)
    target_compile_options(sumo_balls PRIVATE /W4 /O2)
else()
    target_compile_options(sumo_balls PRIVATE -Wall -Wextra -Wpedantic -O3 -march=native)
endif()
