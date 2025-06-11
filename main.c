#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <unistd.h>

#define W_WIDTH 1300
#define W_HEIGHT 900
#define LED_SIZE 11
#define PADDING 23
#define BOX_SIZE_FACTOR 20.0f

#define ACCELERATION 22.8f
#define TIMESTEP 0.01f
#define PARTICLE_RADIUS 5.0f
#define PARTICLE_COUNT 220

#define LEFT_MARGIN (W_WIDTH - LED_SIZE * 16 - PADDING * 15) / 2
#define TOP_MARGIN (W_HEIGHT - LED_SIZE * 16 - PADDING * 15) / 2

// COLLISION STATES
#define COLLISION_NONE 0
#define COLLISION_WALL 1
#define COLLISION_PARTICLE 2
#define COLLISION_WALL_PARTICLE 3

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    vec2 position;
    vec2 velocity;
    u_int8_t collision_state;
} particle;

void initLeds(bool leds[16][16]) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            leds[i][j] = false;
        }
    }
}

vec2 rotate(vec2 p, float angle) {
    vec2 p_translated = { p.x - 7.5f, p.y - 7.5f };

    float rotatedX = p_translated.x * cos(angle) - p_translated.y * sin(angle);
    float rotatedY = p_translated.x * sin(angle) + p_translated.y * cos(angle);

    vec2 result = { rotatedX + 7.5f, rotatedY + 7.5f };

    return result;
}

void initParticles(particle* particles, int n) {
    for (int i = 0; i < n; i++) {
        particles[i].position.x = ((float)rand() / RAND_MAX) * (15 * BOX_SIZE_FACTOR);
        particles[i].position.y = ((float)rand() / RAND_MAX) * (15 * BOX_SIZE_FACTOR);
        particles[i].velocity.x = 0.0f;
        particles[i].velocity.y = 0.0f;
    }
}

void update_particles(particle* particles, int n, float angle, bool leds[16][16]) {
    initLeds(leds);

    for (int i = 0; i < n; i++) particles[i].collision_state = COLLISION_NONE;

    #define WALL_BOUNCE 0.35f
    #define PARTICLE_BOUNCE 0.985f

    for (int i = 0; i < n; i++) {
        particle* p = &particles[i];

        p->position.x += p->velocity.x * TIMESTEP;
        p->position.y += p->velocity.y * TIMESTEP;

        if (p->position.x < 0) {
            p->position.x = 0;
            p->velocity.x = -p->velocity.x * WALL_BOUNCE;

            p->collision_state = COLLISION_WALL;
        } else if (p->position.x > 15 * BOX_SIZE_FACTOR) {
            p->position.x = 15 * BOX_SIZE_FACTOR;
            p->velocity.x = -p->velocity.x * WALL_BOUNCE;
            
            p->collision_state = COLLISION_WALL;
        }

        if (p->position.y < 0) {
            p->position.y = 0;
            p->velocity.y = -p->velocity.y * WALL_BOUNCE;
            
            p->collision_state = COLLISION_WALL;
        } else if (p->position.y > 15 * BOX_SIZE_FACTOR) {
            p->position.y = 15 * BOX_SIZE_FACTOR;
            p->velocity.y = -p->velocity.y * WALL_BOUNCE;
            
            p->collision_state = COLLISION_WALL;
        }

        // particles collision with "slip" effect
        for (int j = i + 1; j < n; j++) {
            float dx = particles[j].position.x - p->position.x;
            float dy = particles[j].position.y - p->position.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < 2 * PARTICLE_RADIUS) {
                p->collision_state = p->collision_state == COLLISION_WALL ?
                    COLLISION_WALL_PARTICLE : COLLISION_PARTICLE;
                particles[j].collision_state = particles[j].collision_state == COLLISION_WALL ?
                    COLLISION_WALL_PARTICLE : COLLISION_PARTICLE;

                // Calculate normal and tangent vectors
                float nx = dx / distance;
                float ny = dy / distance;
                float tx = -ny;
                float ty = nx;

                // Project velocities onto the normal and tangent vectors
                float vi_n = p->velocity.x * nx + p->velocity.y * ny;
                float vi_t = p->velocity.x * tx + p->velocity.y * ty;
                float vj_n = particles[j].velocity.x * nx + particles[j].velocity.y * ny;
                float vj_t = particles[j].velocity.x * tx + particles[j].velocity.y * ty;

                // Swap the normal components to simulate "slipping"
                float temp = vi_n;
                vi_n = vj_n * PARTICLE_BOUNCE; //- rand() / RAND_MAX * 2;
                vj_n = temp * PARTICLE_BOUNCE; //- rand() / RAND_MAX * 2;
                // vi_t = vj_t;

                // Reconstruct the velocities
                p->velocity.x = vi_n * nx + vi_t * tx;
                p->velocity.y = vi_n * ny + vi_t * ty;
                particles[j].velocity.x = vj_n * nx + vj_t * tx;
                particles[j].velocity.y = vj_n * ny + vj_t * ty;

                // Separate overlapping particles
                float overlap = 2 * PARTICLE_RADIUS - distance;
                p->position.x -= nx * overlap / 2;
                p->position.y -= ny * overlap / 2;
                particles[j].position.x += nx * overlap / 2;
                particles[j].position.y += ny * overlap / 2;
            }
        }

        float x = p->position.x / BOX_SIZE_FACTOR;
        float y = p->position.y / BOX_SIZE_FACTOR;

        // project the particle position to the LED grid
        leds[(int)round(x)][(int)round(y)] = true;
        // if (x >= 0 && x < 16 && y >= 0 && y < 16) leds[x][y] = true;

        // Accelerate velocity components based on the angle
        p->velocity.x += ACCELERATION * sin(angle) * TIMESTEP;
        p->velocity.y += ACCELERATION * cos(angle) * TIMESTEP;
    }
}

void render_particles(const particle* particles, int n, float z_rotation) {
    for (int i = 0; i < n; i++) {
        vec2 p = { particles[i].position.x / BOX_SIZE_FACTOR,
                    particles[i].position.y / BOX_SIZE_FACTOR };
        vec2 rotated = rotate(p, z_rotation);

        int screen_x = LEFT_MARGIN + rotated.x * (LED_SIZE + PADDING);
        int screen_y = TOP_MARGIN + rotated.y * (LED_SIZE + PADDING);

        DrawCircle(
            screen_x,
            screen_y,
            PARTICLE_RADIUS + 2.0f,
            particles[i].collision_state == COLLISION_NONE ? (Color) {255, 255, 255, 255} : 
                (particles[i].collision_state == COLLISION_WALL ? BLUE :
                (particles[i].collision_state == COLLISION_PARTICLE ? GREEN : ORANGE))
        );

        vec2 vel_vec = {particles[i].velocity.x, particles[i].velocity.y};
        vec2 rotated_vel = rotate(vel_vec, z_rotation);

        DrawLine(
            screen_x,
            screen_y,
            screen_x + rotated_vel.x / 2,
            screen_y + rotated_vel.y / 2,
            (Color) {255, 255, 255, 255}
        );
    }
}

int main() {
    bool leds[16][16];
    float z_rotation = 0.0f;

    particle* particles = malloc(sizeof(particle) * PARTICLE_COUNT);
    int particle_count = PARTICLE_COUNT;
    initParticles(particles, particle_count);

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

        if (IsKeyPressed(KEY_SPACE)) {
            particle_count -= 5;
        }

        update_particles(particles, particle_count, z_rotation, leds);

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                // Calculate rotated positions
                vec2 p = {i, j};
                vec2 rotated = rotate(p, z_rotation);

                // Translate back and apply margins
                int screen_x = LEFT_MARGIN + rotated.x * (LED_SIZE + PADDING);
                int screen_y = TOP_MARGIN + rotated.y * (LED_SIZE + PADDING);

                DrawCircle(
                    screen_x,
                    screen_y,
                    LED_SIZE,
                    leds[i][j] ? RED : (Color) {20, 15, 10, 255}
                );
            }
        }

        char text[100];
        sprintf(text, "Rotation: %.3f", z_rotation);
        DrawText(text, 10, 10, 20, WHITE);

        // render_particles(particles, particle_count, z_rotation);

        EndDrawing();
        usleep(70);
    }

    CloseWindow();
    free(particles);

    return 0;
}
