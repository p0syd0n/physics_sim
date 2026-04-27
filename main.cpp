#include "raylib-cpp/include/raylib-cpp.hpp"
#include <print>
#include <cmath>

#define SQUARE(x) ((x) * (x))


constexpr double G = 9000;
constexpr int MAX_RADIUS = 20;
constexpr int MIN_RADIUS = 5;

class Ball {
public:
    std::vector<raylib::Vector2> positions;
    raylib::Vector2 velocity;
    raylib::Vector2 position;

    raylib::Vector2 next_velocity;
    raylib::Vector2 next_position;

    long mass;
    float radius;

    raylib::Color color;

    Ball(raylib::Vector2 velocity, raylib::Vector2 position, long mass, raylib::Color color)
        : velocity(velocity), position(position), mass(mass), color(color), radius(calculate_normalized_radius(mass)) {

        std::println("Ball created");
    }

    void apply_velocity() {
        float dt = GetFrameTime();
        next_position.x = position.x + (velocity.x * dt);
        next_position.y = position.y + (velocity.y * dt);
        // position.x += velocity.x * dt;
        // position.y += velocity.y * dt;
    }

    void draw() {
        position.DrawCircle(radius, color);
    }

    void trail() {
        for (int i = 0; i < positions.size(); i++) {
            DrawCircle(positions.at(i).x, positions.at(i).y, 0.9, BLACK);
        }
        positions.push_back(position);
    }

    void apply_gravity(Ball other) {
        //std::println("Applying gravity");
        float angle = atan2(other.position.y - position.y, other.position.x - position.x);
        float distance = sqrt(SQUARE(position.x - other.position.x) + SQUARE(position.y - other.position.y));
        double total_acceleration = (G * (other.mass))/(SQUARE(distance));
        //std::println("Total acceleration: {}", total_acceleration);
        // velocity.x += total_acceleration * cos(angle) * GetFrameTime();
        // velocity.y += total_acceleration * sin(angle) * GetFrameTime();
        
        next_velocity.x = velocity.x + total_acceleration * cos(angle) * GetFrameTime();
        next_velocity.y = velocity.y + total_acceleration * sin(angle) * GetFrameTime();
    }

    void apply_updates() {
        position.x = next_position.x;
        position.y = next_position.y;

        velocity.x = next_velocity.x;
        velocity.y = next_velocity.y;
    }

    static float calculate_normalized_radius(float mass) {
        return (float)(mass-MIN_RADIUS)/(MAX_RADIUS-MIN_RADIUS);
    }
};

void ResolveCollision(Ball& a, Ball& b) {
    // 1. Get the collision vector (normal)
    raylib::Vector2 normal = a.position - b.position;
    float distanceSq = normal.LengthSqr();
    
    if (distanceSq == 0.0f) return;

    raylib::Vector2 relativeVelocity = a.velocity - b.velocity;

    // dotProduct = (vA - vB) . (xA - xB)
    float dotProduct = relativeVelocity.DotProduct(normal);

    if (dotProduct > 0) return;

    float commonFactor = (2.0f * dotProduct) / ((a.mass + b.mass) * distanceSq);

    a.next_velocity = a.velocity - (normal * (commonFactor * b.mass));
    b.next_velocity = b.velocity + (normal * (commonFactor * a.mass));
}

int main() {
    raylib::Window window(800, 450, "Physics Sim");
    window.SetTargetFPS(120);
    window.DrawFPS(50, 50);
    Ball ball(raylib::Vector2(0, -50), raylib::Vector2(700, 800), 500, RED);

    Ball ball2(raylib::Vector2(20, 0), raylib::Vector2(550, 550), 400, BLUE);
    Ball ball3(raylib::Vector2(50, 0), raylib::Vector2(300, 270), 100, GREEN);

    std::vector<Ball> balls;

    balls.push_back(ball);
    balls.push_back(ball2);
     balls.push_back(ball3);

    while (!window.ShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int i = 0; i < balls.size(); i++) {
            for (int j = 0; j < balls.size(); j++)  {
                if (j == i) {
                    continue;
                }
                balls[i].apply_gravity(balls[j]);
                if (CheckCollisionCircles(balls[i].position, balls[i].radius, balls[j].position, balls[j].radius)) {
                    std::println("COllision detected");
                    ResolveCollision(balls[i], balls[j]);

                }
            }

            balls[i].apply_velocity();
            balls[i].draw();
            balls[i].trail();
        }

        for (int i = 0; i < balls.size(); i++) {
            balls[i].apply_updates();
        }

        EndDrawing();
    }

    return 0;
}