#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace VectorMath {
    /**
     * Normalize a 2D vector to unit length.
     * Returns zero vector if input is zero-length.
     */
    inline sf::Vector2f normalize(const sf::Vector2f& v) {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len == 0.f) return {0.f, 0.f};
        return {v.x / len, v.y / len};
    }

    /**
     * Calculate magnitude (length) of a 2D vector.
     */
    inline float magnitude(const sf::Vector2f& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    /**
     * Calculate squared magnitude without sqrt (faster for comparisons).
     */
    inline float magnitudeSquared(const sf::Vector2f& v) {
        return v.x * v.x + v.y * v.y;
    }

    /**
     * Calculate distance between two points.
     */
    inline float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * Calculate squared distance (faster for comparisons).
     */
    inline float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return dx * dx + dy * dy;
    }
}
