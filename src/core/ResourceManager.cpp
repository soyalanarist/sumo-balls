#include "ResourceManager.h"
#include "Settings.h"
#include <iostream>

std::shared_ptr<ResourceManager> ResourceManager::create(
    const std::string& windowTitle,
    unsigned int windowWidth,
    unsigned int windowHeight,
    bool fullscreen,
    const std::string& fontPath
) {
    // Custom deleter guarantees the window closes even if construction fails later.
    auto manager = std::shared_ptr<ResourceManager>(new ResourceManager(), [](ResourceManager* m){
        if (m) {
            if (m->window) {
                m->window->close();
            }
            delete m;
        }
    });

    // Create window with anti-aliasing
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;  // Request 16x AA

    // Build resources with strong exception safety: if any step throws, cleanup via deleter
    manager->window = std::make_shared<sf::RenderWindow>();

    try {
        if (fullscreen) {
            manager->window->create(sf::VideoMode::getDesktopMode(), windowTitle, sf::Style::Fullscreen, settings);
        } else {
            manager->window->create(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default, settings);
            manager->window->setPosition(sf::Vector2i(0, 0));
        }

        if (!manager->window->isOpen()) {
            throw std::runtime_error("Failed to create game window");
        }

        // Set frame rate limit
        manager->window->setFramerateLimit(60);

        // Load default font
        manager->defaultFont = std::make_shared<sf::Font>();
        if (!manager->defaultFont->loadFromFile(fontPath)) {
            throw std::runtime_error("Failed to load font: " + fontPath);
        }
    } catch (...) {
        // Ensure partial resources are released before propagating
        manager->defaultFont.reset();
        manager->window.reset();
        throw;
    }

    return manager;
}
