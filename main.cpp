#include "raylib-cpp/include/raylib-cpp.hpp"
#include <print>
#include <cmath>

#define SQUARE(x) ((x) * (x))


constexpr double G = 500;

class Ball {
public:
    std::vector<raylib::Vector2> positions;
    raylib::Vector2 velocity;
    raylib::Vector2 position;
    int radius = 4;
    long mass;

    Ball(raylib::Vector2 velocity, raylib::Vector2 position, long mass)
        : velocity(velocity), position(position), mass(mass) {
        std::println("initial velocity set: {}, {}", velocity.x, velocity.y);
    }

    void apply_velocity() {
        float dt = GetFrameTime();
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
    }

    void draw() {
        position.DrawCircle(radius);
    }

    void trail() {
        for (int i = 0; i < positions.size(); i++) {
            DrawCircle(positions.at(i).x, positions.at(i).y, 0.9, BLACK);
        }
        positions.push_back(position);
    }

    void apply_gravity(Ball other) {
        std::println("Applying gravity");
        float angle = atan2(other.position.y - position.y, other.position.x - position.x);
        float distance = sqrt(SQUARE(position.x - other.position.x) + SQUARE(position.y - other.position.y));
        double total_acceleration = (G * (other.mass))/(SQUARE(distance));
        std::println("Total acceleration: {}", total_acceleration);
        velocity.x += total_acceleration * cos(angle) * GetFrameTime();
        velocity.y += total_acceleration * sin(angle) * GetFrameTime();
    }
};

int main() {
    raylib::Window window(800, 450, "Physics Sim");
    window.SetTargetFPS(120);
    window.DrawFPS(50, 50);
    Ball ball(raylib::Vector2(0, 0), raylib::Vector2(50, 100), 5);

    Ball ball2(raylib::Vector2(0, 0), raylib::Vector2(100, 200),5);

    std::vector<Ball> balls;

    balls.push_back(ball);
    balls.push_back(ball2);

    while (!window.ShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int i = 0; i < balls.size(); i++) {
            for (int j = 0; j < balls.size(); j++)  {
                if (j == i) {
                    continue;
                }
                balls[i].apply_gravity(balls[j]);

            }
            balls[i].apply_velocity();
            balls[i].draw();
            balls[i].trail();
        }
        EndDrawing();
    }

    return 0;
}