#pragma once

namespace GameConfig {
    // Arena settings
    constexpr float INITIAL_ARENA_RADIUS = 300.f;
    constexpr float ARENA_CENTER_X = 600.f;
    constexpr float ARENA_CENTER_Y = 450.f;
    constexpr float ARENA_SHRINK_RATE = 5.0f;  // pixels per second

    // Spawn settings
    constexpr float SPAWN_RADIUS = 200.f;
    constexpr int NUM_PLAYERS = 6;
    constexpr int NUM_AI_PLAYERS = 5;

    // Player settings
    constexpr float PLAYER_RADIUS = 18.f;
    constexpr float PLAYER_BASE_SPEED = 100.f;
    constexpr float PLAYER_ACCELERATION = 25.f;
    constexpr float PLAYER_MAX_SPEED = 500.f;
    constexpr float PLAYER_FRICTION = 0.002f;

    // Physics
    constexpr float COLLISION_RESTITUTION = 1.8f;

    // Particle system
    constexpr int PARTICLE_COUNT_PER_EXPLOSION = 18;
    constexpr float PARTICLE_LIFETIME = 0.6f;
    constexpr float PARTICLE_RADIUS = 4.f;
    constexpr float PARTICLE_VELOCITY_MULTIPLIER = 1.5f;
    constexpr float PARTICLE_GRAVITY = 50.f;
    constexpr float PARTICLE_DRAG = 0.98f;

    // Countdown
    constexpr float COUNTDOWN_DURATION = 3.0f;
    constexpr float COUNTDOWN_FADE_IN_TIME = 0.2f;
    constexpr float COUNTDOWN_FADE_OUT_START = 0.7f;
    constexpr float COUNTDOWN_FADE_OUT_TIME = 0.3f;

    // UI
    constexpr int COUNTDOWN_FONT_SIZE = 200;
    constexpr int WINDOW_WIDTH = 1200;
    constexpr int WINDOW_HEIGHT = 900;

    // AI settings
    constexpr float AI_DIFFICULTY_EASY = 0.5f;
    constexpr float AI_DIFFICULTY_MEDIUM_LOW = 0.55f;
    constexpr float AI_DIFFICULTY_MEDIUM = 0.6f;
    constexpr float AI_DIFFICULTY_MEDIUM_HIGH = 0.65f;
    constexpr float AI_DIFFICULTY_HIGH = 0.7f;

    // AI burst timing
    constexpr float BURST_MIN_DELAY = 6.0f;
    constexpr float BURST_MAX_DELAY = 14.0f;
    constexpr float BURST_DURATION = 5.0f;

    // Arena rendering
    constexpr int ARENA_CIRCLE_POINT_COUNT = 1000;
    constexpr int PLAYER_CIRCLE_POINT_COUNT = 1000;
}
