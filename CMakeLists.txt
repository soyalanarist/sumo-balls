cmake_minimum_required(VERSION 3.16)
project(sumo_balls VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(sumo_balls
    src/main.cpp
    src/core/Game.cpp
    src/core/Screen.cpp
    src/core/ScreenStack.cpp
    src/core/Settings.cpp
    src/ui/UIElement.cpp
    src/ui/Button.cpp
    src/ui/ToggleSwitch.cpp

    src/screens/GameScreen.cpp
    src/screens/menus/MainMenu.cpp
    src/screens/menus/OptionsMenu.cpp
    src/screens/menus/PauseMenu.cpp
    src/screens/menus/GameOverMenu.cpp

    src/game/entities/Player.cpp
    src/game/entities/PlayerEntity.cpp
    src/game/entities/Arena.cpp

    src/game/controllers/HumanController.cpp
    src/game/controllers/AIController.cpp

    src/systems/InputSystem.cpp
)

target_include_directories(sumo_balls PRIVATE include)

target_link_libraries(sumo_balls
    sfml-graphics
    sfml-window
    sfml-system
)

if (MSVC)
    target_compile_options(sumo_balls PRIVATE /W4)
else()
    target_compile_options(sumo_balls PRIVATE -Wall -Wextra -Wpedantic)
endif()
