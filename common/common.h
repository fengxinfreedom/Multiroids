#ifndef __COMMON_H__
#define __COMMON_H__

#include <SDL.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PLAYER_SPR_SIZE 16
#define PLAYER_SPR_POINTS 5
#define PLAYER_ACCELERATION 0.0675f
#define PLAYER_MOVE_SPEED 3
#define PLAYER_TURN_SPEED 0.05f

#define MAX_PLAYERS 8
#define MAX_BULLETS 256
#define FIRE_INTERVAL 0.5

#define BULLET_SIZE 3
#define BULLET_SPEED 5

enum PACKET_TYPE
{
    PACKET_INPUT = 0,
    PACKET_WORLD,
    PACKET_BULLET
};

extern const double physics_step;
extern const double network_step;

typedef struct point_t
{
    float x;
    float y;
} point_t;

typedef struct input_state_t
{
    uint8_t type;
    uint32_t sequence;
    uint8_t id : 4;
    uint8_t thrust : 1;
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t shoot : 1;
} input_state_t;

typedef struct bullet_state_t
{
    uint8_t type;
    uint32_t sequence;
    uint8_t owner : 7;
    uint8_t alive : 1;
    point_t position;
    point_t velocity;
} bullet_state_t;

typedef struct player_state_t
{
    uint32_t sequence;
    point_t position;
    point_t velocity;
    float angle;
} player_state_t;

typedef struct world_state_t
{
    uint8_t type;
    uint32_t sequence;
    player_state_t players[MAX_PLAYERS];
    uint8_t alive;
} world_state_t;

void translate_world(world_state_t* world);
void render_world(SDL_Renderer* renderer, world_state_t* world);
void translate_bullets(bullet_state_t* bullets, int count);
void render_bullets(SDL_Renderer* renderer, bullet_state_t* bullets, int count);
void clean_bullets(bullet_state_t* bullets, int* count);
void check_bullet_collisions(world_state_t* world, bullet_state_t* bullets, int count);

inline float to_rad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

inline float to_deg(float rad)
{
    return rad * 180.0f / (float)M_PI;
}

inline int is_player_alive(world_state_t* world, int player)
{
    return world->alive & 1 << player;
}

inline void set_player_alive(world_state_t* world, int player)
{
    world->alive = world->alive | 1 << player;
}

inline void set_player_free(world_state_t* world, int player)
{
    world->alive = world->alive & ~(1 << player);
}

inline int get_free_player(world_state_t* world)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (is_player_alive(world, i))
            continue;

        set_player_alive(world, i);

        return i;
    }

    return -1;
}

extern const point_t player_sprite[PLAYER_SPR_POINTS];
extern const SDL_Color player_colors[MAX_PLAYERS];

#endif