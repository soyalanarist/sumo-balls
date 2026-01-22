#include "TestFramework.h"
#include "../src/ui/Button.h"
#include <SFML/Graphics/Font.hpp>

bool testButtonConstruction(std::string& msg) {
    sf::Font font;
    Button button(font, "Test Button", {200.f, 50.f}, {100.f, 100.f});
    
    sf::Vector2f pos = button.getPosition();
    if (std::abs(pos.x - 100.f) > 0.1f || std::abs(pos.y - 100.f) > 0.1f) {
        msg = "Button position should match constructor arguments";
        return false;
    }
    
    sf::Vector2f size = button.getSize();
    if (std::abs(size.x - 200.f) > 0.1f || std::abs(size.y - 50.f) > 0.1f) {
        msg = "Button size should match constructor arguments";
        return false;
    }
    
    if (button.wasClicked()) {
        msg = "Button should not be clicked initially";
        return false;
    }
    
    if (button.isHovered()) {
        msg = "Button should not be hovered initially";
        return false;
    }
    
    return true;
}

bool testButtonSetPosition(std::string& msg) {
    sf::Font font;
    Button button(font, "Test", {100.f, 50.f}, {0.f, 0.f});
    
    button.setPosition({300.f, 200.f});
    sf::Vector2f newPos = button.getPosition();
    
    if (std::abs(newPos.x - 300.f) > 0.1f || std::abs(newPos.y - 200.f) > 0.1f) {
        msg = "Button position should be updated correctly";
        return false;
    }
    
    return true;
}

bool testButtonSetSize(std::string& msg) {
    sf::Font font;
    Button button(font, "Test", {100.f, 50.f}, {0.f, 0.f});
    
    button.setSize({250.f, 75.f});
    sf::Vector2f newSize = button.getSize();
    
    if (std::abs(newSize.x - 250.f) > 0.1f || std::abs(newSize.y - 75.f) > 0.1f) {
        msg = "Button size should be updated correctly";
        return false;
    }
    
    return true;
}

bool testButtonReset(std::string& msg) {
    sf::Font font;
    Button button(font, "Test", {100.f, 50.f}, {100.f, 100.f});
    
    // Create a hidden window for update testing
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    window.setVisible(false);
    
    // Simulate interaction
    button.update(window);
    button.reset();
    
    if (button.wasClicked()) {
        msg = "Button click state should be cleared after reset";
        return false;
    }
    
    window.close();
    return true;
}

int main() {
    test::TestSuite::instance().registerTest("Button construction", testButtonConstruction);
    test::TestSuite::instance().registerTest("Button set position", testButtonSetPosition);
    test::TestSuite::instance().registerTest("Button set size", testButtonSetSize);
    test::TestSuite::instance().registerTest("Button reset", testButtonReset);
    
    int result = test::TestSuite::instance().runAll();
    return result;
}

