#include <math.h>
#include <raylib.h>
#include <stdio.h>

#define RWIDTH 50
#define RHEIGHT 50
#define WIDTH 800
#define HEIGHT 600
#define bg GetColor(0x141415ff)
#define red GetColor(0xd8647eff)
#define green GetColor(0x7fa563ff)

int main() {
    InitWindow(WIDTH, HEIGHT, "bouncy");

    Vector2 pos = {20.0, 20.0};
    Vector2 gravity = {0.0, 1000.0};
    Vector2 velocity = {100 * GetRandomValue(1, 10),
                        100 * GetRandomValue(0, 10)};
    printf("Initial velocity: %f %f\n", velocity.x, velocity.y);
    float green_hue = ColorToHSV(green).y;
    float red_hue = ColorToHSV(red).x;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float t = (GetScreenHeight() - pos.y - RHEIGHT) / GetScreenHeight();
        Color color = ColorFromHSV(green_hue + t * (red_hue - green_hue), 1, 1);

        velocity.x += gravity.x * dt;
        velocity.y += gravity.y * dt;
        Vector2 prev = pos;
        pos.x += velocity.x * dt;
        pos.y += velocity.y * dt;

        if (pos.x <= 0 || (pos.x + RWIDTH) >= GetScreenWidth()) {
            velocity.x *= -0.8f;
            pos.x = prev.x;
        }
        if (pos.y <= 0 || (pos.y + RHEIGHT) >= GetScreenHeight()) {
            velocity.y *= -0.8f;
            pos.y = prev.y;
        }

        // zero out y component when it's close enough to the ground
        if ((pos.y + RHEIGHT) >= GetScreenHeight() - 1 &&
            fabsf(velocity.y) < 1) {
            velocity.y = 0;
            pos.y = GetScreenHeight() - RHEIGHT;
            velocity.x *=
                1.0f - 2 * dt; // friction when the square is just sliding
        }

        BeginDrawing();
        ClearBackground(bg);

        DrawRectangleV(pos, (Vector2){RWIDTH, RHEIGHT}, color);

        DrawFPS(10, 10);
        DrawText(TextFormat("Velocity: %0.2f %0.2f", velocity.x, velocity.y),
                 GetScreenWidth() / 2.0f, 10, 20, GRAY);
        EndDrawing();
    }
    return 0;
}
