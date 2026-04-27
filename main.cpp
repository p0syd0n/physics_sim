#include "raylib-cpp/include/raylib-cpp.hpp"
#include <print>
#include <cmath>

#define SQUARE(x) ((x) * (x))


constexpr double G = 10000;
constexpr int MAX_RADIUS = 20;
constexpr int MIN_RADIUS = 5;

constexpr int WIDTH = 800;
constexpr int HEIGHT = 450;

class Ball {
public:
    std::vector<raylib::Vector2> positions;
    raylib::Vector2 velocity;
    raylib::Vector2 position;
    raylib::Vector2 acceleration = {0, 0};

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

        float scale = 0.5f;  // tune this
        raylib::Vector2 tip = {
            position.x + acceleration.x * scale,
            position.y + acceleration.y * scale
        };
        DrawLineEx(position, tip, 2.0f, YELLOW);
        DrawCircleV(tip, 3.0f, YELLOW);  // arrowhead dot
    }

    void trail() {
        for (int i = 0; i < positions.size(); i++) {
            DrawCircle(positions.at(i).x, positions.at(i).y, 0.9, color);
        }
        positions.push_back(position);
    }

    void apply_gravity(Ball other) {
        float angle = atan2(other.position.y - position.y, other.position.x - position.x);
        float distance = std::max(sqrt(SQUARE(position.x - other.position.x) + SQUARE(position.y - other.position.y)), radius + other.radius);
        double total_acceleration = (G * other.mass) / SQUARE(distance);

        next_velocity.x = velocity.x + total_acceleration * cos(angle) * GetFrameTime();
        next_velocity.y = velocity.y + total_acceleration * sin(angle) * GetFrameTime();

        acceleration.x += total_acceleration * cos(angle);
        acceleration.y += total_acceleration * sin(angle);
    }
    void apply_updates() {
        acceleration = raylib::Vector2(0, 0);
        int w = GetScreenWidth();
        int h = GetScreenHeight();

        position.x = next_position.x;
        position.y = next_position.y;
        velocity.x = next_velocity.x;
        velocity.y = next_velocity.y;

        if (position.x - radius > w)  position.x = -radius;
        if (position.x + radius < 0)  position.x = w + radius;
        if (position.y - radius > h)  position.y = -radius;
        if (position.y + radius < 0)  position.y = h + radius;
    }

    void separate(Ball& other) {
        raylib::Vector2 delta = position - other.position;
        float distance = delta.Length();
        float minDistance = radius + other.radius;

        if (distance >= minDistance) return;
        if (distance == 0.0f) {
            // Exactly on top of each other - push in arbitrary direction
            delta = raylib::Vector2(1.0f, 0.0f);
            distance = 1.0f;
        }

        // How much overlap there is
        float overlap = minDistance - distance;

        // Normalized direction to push along
        raylib::Vector2 pushDir = delta / distance;

        // Push each ball out by half the overlap, weighted by mass
        float totalMass = mass + other.mass;
        float myShare = (float)other.mass / totalMass;
        float otherShare = (float)mass / totalMass;

        position.x += pushDir.x * overlap * myShare;
        position.y += pushDir.y * overlap * myShare;

        other.position.x -= pushDir.x * overlap * otherShare;
        other.position.y -= pushDir.y * overlap * otherShare;
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

    raylib::Window window(WIDTH, HEIGHT, "Physics Sim");
    window.SetTargetFPS(120);
    window.DrawFPS(50, 50);
    // Ball ball(raylib::Vector2(0, -50), raylib::Vector2(700, 800), 500, RED);

    // Ball ball2(raylib::Vector2(20, 0), raylib::Vector2(550, 550), 400, BLUE);
    // Ball ball3(raylib::Vector2(50, 40), raylib::Vector2(300, 270), 300, GREEN);

    std::vector<Ball> balls;

    // balls.push_back(ball);
    // balls.push_back(ball2);
    //  balls.push_back(ball3);


    auto randomBall = []() {
        float vx = GetRandomValue(-100, 100);
        float vy = GetRandomValue(-100, 100);
        float x  = GetRandomValue(MAX_RADIUS, WIDTH  - MAX_RADIUS);
        float y  = GetRandomValue(MAX_RADIUS, HEIGHT - MAX_RADIUS);
        long  m  = GetRandomValue(100, 500);
        Color c  = { (unsigned char)GetRandomValue(50, 255),
                    (unsigned char)GetRandomValue(50, 255),
                    (unsigned char)GetRandomValue(50, 255), 255 };
        return Ball(raylib::Vector2(vx, vy), raylib::Vector2(x, y), m, c);
    };

    constexpr int NUM_BALLS = 3;
    for (int i = 0; i < NUM_BALLS; i++)
        balls.push_back(randomBall());

    while (!window.ShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int i = 0; i < balls.size(); i++) {
            for (int j = i + 1; j < balls.size(); j++) {  // note j = i+1 to avoid double separation
                balls[i].separate(balls[j]);
            }
        }

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