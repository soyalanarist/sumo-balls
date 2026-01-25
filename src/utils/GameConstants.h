#pragma once
#include <cstdint>
#include <string>

/// Game-wide constants and configuration values
/// Centralizes magic numbers for maintainability
namespace GameConstants {

// === Window & Display ===
constexpr float WINDOW_WIDTH = 1200.f;
constexpr float WINDOW_HEIGHT = 900.f;
constexpr int FRAMERATE_LIMIT = 60;

// === Arena & Physics ===
constexpr float ARENA_CENTER_X = 600.f;
constexpr float ARENA_CENTER_Y = 450.f;
constexpr float ARENA_RADIUS = 400.f;
constexpr float SPAWN_RADIUS = 200.f;        // Distance from center for player spawn
constexpr float SPAWN_RADIUS_LARGE = 300.f;  // Alternative spawn radius

// === Player Physics ===
constexpr float PLAYER_RADIUS = 30.f;
constexpr float PLAYER_MASS = 1.f;
constexpr float PLAYER_ACCELERATION = 800.f;
constexpr float PLAYER_MAX_SPEED = 400.f;
constexpr float PLAYER_FRICTION = 0.9f;

// === Collision & Impulse ===
constexpr float RESTITUTION = 0.6f;           // Bounciness of collisions
constexpr float PUSH_IMPULSE_SCALE = 1.2f;   // Multiplier for push force

// === Particles ===
constexpr float PARTICLE_LIFETIME = 0.6f;     // Seconds
constexpr int PARTICLES_PER_EXPLOSION = 20;

// === Network ===
constexpr uint16_t DEFAULT_SERVER_PORT = 7777;
constexpr int NETWORK_TIMEOUT_MS = 5000;
constexpr int SNAPSHOT_RATE_MS = 30;          // Server sends snapshots every 30ms

// === Interpolation & Prediction ===
constexpr float INTERP_BLEND_FACTOR = 0.35f;  // Client-side interpolation blend
constexpr float SNAPSHOT_INTERP_DELAY = 100.f; // ms to delay for smooth interpolation

// === Game Timing ===
constexpr float COUNTDOWN_DURATION = 3.f;     // Seconds
constexpr float GAME_DURATION = 120.f;        // Seconds (2 minutes)
constexpr float FIXED_TIMESTEP = 1.f / 60.f;  // Physics update rate (60 FPS)

// === Math Constants ===
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;

// === File Paths ===
inline const std::string ASSETS_DIR = "assets/";
inline const std::string CONFIG_FILE = "config.json";
inline const std::string FONT_FILE = "assets/arial.ttf";

// === UI Layout ===
constexpr float BUTTON_WIDTH = 200.f;
constexpr float BUTTON_HEIGHT = 50.f;
constexpr float MENU_SPACING = 20.f;

// === Debug ===
constexpr bool ENABLE_DEBUG_OVERLAY = false;
constexpr bool ENABLE_PHYSICS_VALIDATION = true;

} // namespace GameConstants
