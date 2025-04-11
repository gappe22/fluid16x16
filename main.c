#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <raylib.h>

#define W_WIDTH 1300
#define W_HEIGHT 900
#define LED_SIZE 11
#define PADDING 23
#define BOX_SIZE_FACTOR 20.0f

#define ACCELERATION 20.8f
#define TIMESTEP 0.01f
#define PARTICLE_RADIUS 5.0f

#define LEFT_MARGIN (W_WIDTH - LED_SIZE * 16 - PADDING * 15) / 2
#define TOP_MARGIN (W_HEIGHT - LED_SIZE * 16 - PADDING * 15) / 2

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    vec2 position;
    vec2 velocity;
} particle;

void initLeds(bool leds[16][16]) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            leds[i][j] = false;
        }
    }
}

vec2 rotate(vec2 p, float angle) {
    float rotatedX = p.x * cos(angle) - p.y * sin(angle);
    float rotatedY = p.x * sin(angle) + p.y * cos(angle);

    vec2 result = { rotatedX, rotatedY };

    return result;
}

void update_particles(particle* particles, int n, float angle, bool leds[16][16]) {
    initLeds(leds);

    for (int i = 0; i < n; i++) {
        if (particles[i].position.x < 0) {
            particles[i].position.x = 0;
            particles[i].velocity.x = 0;
        } else if (particles[i].position.x > 15 * BOX_SIZE_FACTOR) {
            particles[i].position.x = 15 * BOX_SIZE_FACTOR;
            particles[i].velocity.x = 0;
        }

        if (particles[i].position.y < 0) {
            particles[i].position.y = 0;
            particles[i].velocity.y = 0;
        } else if (particles[i].position.y > 15 * BOX_SIZE_FACTOR) {
            particles[i].position.y = 15 * BOX_SIZE_FACTOR;
            particles[i].velocity.y = 0;
        }

        // if (particles[i].position.x > 0 && particles[i].position.x < 16 * BOX_SIZE_FACTOR &&
        //     particles[i].position.y > 0 && particles[i].position.y < 16 * BOX_SIZE_FACTOR) {

            int x = (int) (particles[i].position.x / BOX_SIZE_FACTOR);
            int y = (int) (particles[i].position.y / BOX_SIZE_FACTOR);

            if (x >= 0 && x < 16 && y >= 0 && y < 16) leds[x][y] = true;

            particles[i].position.x += particles[i].velocity.x * TIMESTEP;
            particles[i].position.y += particles[i].velocity.y * TIMESTEP;

            // Accelerate velocity components based on the angle
            particles[i].velocity.x += ACCELERATION * sin(angle) * TIMESTEP;
            particles[i].velocity.y += ACCELERATION * cos(angle) * TIMESTEP;
        // }
    }
}

int main() {
    bool leds[16][16];
    float z_rotation = 0.0f;

    particle* particles = malloc(sizeof(particle) * 16 * 16);
    int particle_count = 4;

    InitWindow(W_WIDTH, W_HEIGHT, "fluid16x16");

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) {
            z_rotation += 0.009f;
        } else if (IsKeyDown(KEY_LEFT)) {
            z_rotation -= 0.009f;
        }

        if (IsKeyPressed(KEY_UP)) {
            z_rotation = 0.0f;
        } else if (IsKeyPressed(KEY_DOWN)) {
            z_rotation = PI;
        }

        update_particles(particles, particle_count, z_rotation, leds);

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                // Calculate rotated positions
                vec2 p = {i - 7.5f, j - 7.5f};
                vec2 rotated = rotate(p, z_rotation);

                // Translate back and apply margins
                int screen_x = LEFT_MARGIN + (rotated.x + 7.5f) * (LED_SIZE + PADDING);
                int screen_y = TOP_MARGIN + (rotated.y + 7.5f) * (LED_SIZE + PADDING);

                // if (leds[i][j]) {
                //     DrawCircle(
                //         screen_x,
                //         screen_y,
                //         LED_SIZE + 5,
                //         (Color) {255, 0, 0, 100}
                //     );
                // }

                DrawCircle(
                    screen_x,
                    screen_y,
                    LED_SIZE,
                    leds[i][j] ? RED : (Color) {20, 15, 10, 255}
                );
            }
        }

        for (int i = 0; i < particle_count; i++) {
            vec2 p = { particles[i].position.x / BOX_SIZE_FACTOR - 7.5f,
                        particles[i].position.y / BOX_SIZE_FACTOR - 7.5f };
            vec2 rotated = rotate(p, z_rotation);

            int screen_x = LEFT_MARGIN + (rotated.x + 7.5f) * (LED_SIZE + PADDING);
            int screen_y = TOP_MARGIN + (rotated.y + 7.5f) * (LED_SIZE + PADDING);

            DrawCircle(
                screen_x,
                screen_y,
                PARTICLE_RADIUS,
                BLUE
            );
        }

        EndDrawing();
    }

    CloseWindow();
    free(particles);

    return 0;
}