#pragma once
#include <cmath>

struct Vec2 {
    float x{0.f};
    float y{0.f};

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vec2& operator/=(float s) { x /= s; y /= s; return *this; }
};

namespace VectorMath {
    inline Vec2 normalize(const Vec2& v) {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len == 0.f) return {0.f, 0.f};
        return {v.x / len, v.y / len};
    }

    inline float magnitude(const Vec2& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    inline float magnitudeSquared(const Vec2& v) {
        return v.x * v.x + v.y * v.y;
    }

    inline float distance(const Vec2& a, const Vec2& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    inline float distanceSquared(const Vec2& a, const Vec2& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return dx * dx + dy * dy;
    }
}
