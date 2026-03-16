#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define WIN_HEIGHT 800
#define WIN_WIDTH 800
#define FPS 24

#define TRAIL_REF_LEN 5000 // trail positions for scale = 1
#define STEP_SIZE_MAX_EXP 5
#define STEP_SIZE_MIN_EXP 3

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// vague theme <3
static const Color BG = {0x26, 0x26, 0x26, 0xff};
static const Color red = {0xd8, 0x64, 0x7e, 0xff};
static const Color orange = {0xe0, 0x83, 0x98, 0xff};
static const Color yellow = {0xf3, 0xbe, 0x7c, 0xff};
static const Color green = {0x7f, 0xa5, 0x63, 0xff};
static const Color purple = {0xbb, 0x9d, 0xbd, 0xff};
static const Color pink = {0xc9, 0xb1, 0xca, 0xff};
static const Color cyan = {0xae, 0xae, 0xd1, 0xff};
static const Color colors[] = {red, orange, yellow, green, purple, pink, cyan};
static const u8 ncolors = 7;

// static const Color grey = {0x60, 0x60, 0x79, 0xff};

typedef struct {
    u8 step_size;
    Color color;
    Vector2 head;
    Vector2 *positions;
} Agent;

void move(float *x, float *y, float dx, float dy) {
    // wrap remainder to a positive value
    *x = ((i64)(*x + dx) % WIN_WIDTH + WIN_WIDTH) % WIN_WIDTH;
    *y = ((i64)(*y + dy) % WIN_HEIGHT + WIN_HEIGHT) % WIN_HEIGHT;
}

u64 get_trail_len(Agent *agent) { return TRAIL_REF_LEN / agent->step_size; }

Vector2 rand_dir() {
    u64 num = GetRandomValue(0, 3);
    Vector2 dir;
    switch (num) {
    case 0: // right
        dir.x = 1;
        dir.y = 0;
        break;
    case 1: // down
        dir.x = 0;
        dir.y = 1;
        break;
    case 2: // left
        dir.x = -1;
        dir.y = 0;
        break;
    case 3: // up
        dir.x = 0;
        dir.y = -1;
        break;
    default:
        dir.x = 0;
        dir.y = 1;
    }

    return dir;
}
static const Vector2 abscenter = {WIN_WIDTH / 2, WIN_HEIGHT / 2};

int main(int argc, char *argv[]) {
    u32 n_agents;
    if (argc < 2) {
        n_agents = 1;
    } else {
        n_agents = strtol(argv[1], NULL, 10);
    }
    Agent *agents = malloc(n_agents * sizeof(Agent));

    SetRandomSeed(time(NULL));

    // initialize agents
    for (u32 i = 0; i < n_agents; i++) {
        i32 rand_num = GetRandomValue(STEP_SIZE_MIN_EXP, STEP_SIZE_MAX_EXP);
        u8 scale = powl(2, rand_num);
        printf("scale %d: %d\n", i, scale);

        Vector2 *positions = malloc(TRAIL_REF_LEN / scale * sizeof(Vector2));
        agents[i] = (Agent){.step_size = scale,
                            .head = {abscenter.x, abscenter.y},
                            .color = colors[i % ncolors],
                            .positions = positions};
    }

    InitWindow(WIN_WIDTH, WIN_HEIGHT, "gowno");
    SetTargetFPS(FPS);

    u64 step = 0;
    while (!WindowShouldClose()) {
        // write positions
        for (u32 j = 0; j < n_agents; j++) {
            Agent *agent = &agents[j];
            u64 trail_len = get_trail_len(agent);

            agent->positions[step % trail_len] = agent->head;
        }
        step++;

        ClearBackground(BG);

        BeginDrawing();
        for (u32 j = 0; j < n_agents; j++) {
            Vector2 dir = rand_dir();
            Agent *agent = &agents[j];
            u64 trail_len = get_trail_len(agent);

            // draw trail
            // draw positions_size most recent positions
            u64 draw_start = step > trail_len ? step - trail_len : 1;
            for (u64 i = draw_start; i < step; i++) {
                u64 cur = i % trail_len;
                u64 prev = (i - 1) % trail_len;

                // don't connect wrapped positions
                if (cur == step % trail_len)
                    continue;

                Vector2 start = agent->positions[prev];
                Vector2 end = agent->positions[cur];

                // check for wrap around the screen
                float dx = end.x - start.x; // positive: left to right edge,
                                            // negative: right to left edge
                float dy = end.y - start.y; // positive: up to bottom edge,
                                            // negative: bottom to top edge
                if (fabsf(dx) > agent->step_size ||
                    fabsf(dy) > agent->step_size) {
                    // draw start -> edge, edge -> end
                    Vector2 edge_start = start;
                    Vector2 edge_end = end;

                    if (fabsf(dx) > agent->step_size) { // wrapped horizontally
                        edge_start.x = dx > 0 ? 0 : WIN_WIDTH;
                        edge_end.x = dx > 0 ? WIN_WIDTH : 0;
                    }
                    if (fabsf(dy) > agent->step_size) { // wrapped vertically
                        edge_start.y = dy > 0 ? 0 : WIN_HEIGHT;
                        edge_end.y = dy > 0 ? WIN_HEIGHT : 0;
                    }

                    DrawLineV(start, edge_start, agent->color);
                    DrawLineV(edge_end, end, agent->color);
                } else {
                    DrawLineV(start, end, agent->color);
                }
            }

            move(&agent->head.x, &agent->head.y, dir.x * agent->step_size,
                 dir.y * agent->step_size);
        }
        EndDrawing();
        DrawText(TextFormat("Step: %llu", step), 10, 10, 20, GRAY);
    }
    CloseWindow();

    for (u64 i = 0; i < n_agents; i++) {
        free(agents[i].positions);
    }
    free(agents);

    return 0;
}
