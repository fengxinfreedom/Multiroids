#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include "sprite.h"

#define PORT 8888

ENetHost* server = NULL;
ENetAddress address;
ENetEvent net_event;

char buffer[256];
int running = 1;

// TODO use bitfields!
typedef struct player_state_t
{
    int index;
    int turn_left;
    int turn_right;
    int thrust;
} player_state_t;

player_state_t update_players[MAX_PLAYERS];
int update_count = 0;

void init()
{
    if (enet_initialize() != 0)
    {
        printf("Failed to initialize ENet.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_set_host(&address, "127.0.0.1");
    address.port = PORT;

    // Create the server (host address, number of clients,
    // number of channels, incoming bandwith, outgoing bandwith).
    server = enet_host_create(&address, 4, 2, 0, 0);

    if (server == NULL)
    {
        printf("Failed to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_get_host_ip(&address, buffer, sizeof(buffer));
    printf("Launching new server at %s:%u\n", buffer, address.port);
}

void receive_packets()
{
    while (enet_host_service(server, &net_event, 0) > 0)
    {
        printf("Listening to incoming connections.\n");
        switch (net_event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            enet_address_get_host_ip(&net_event.peer->address, buffer, sizeof(buffer));
            printf("A new client connected from %s:%u\n", buffer, net_event.peer->address.port);
            net_event.peer->data = "Player 1";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            printf("A packet was received!\nSize: %u\nData: %s\nFrom: %s\nChannel: %u\n\n", (uint32_t)net_event.packet->dataLength, (char*)net_event.packet->data, (char*)net_event.peer->data, net_event.channelID);
            enet_packet_destroy(net_event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("%s disconnected.\n", (char*)net_event.peer->data);
            net_event.peer->data = NULL;
            break;
        }
    }
}

void update()
{
    for (int i = 0; i < update_count; i++)
    {
        sprite_t* player = &players[update_players[i].index];

        if (update_players[i].turn_right)
        {
            player->angle += 0.1f;
        }

        if (update_players[i].turn_left)
        {
            player->angle -= 0.1f;
        }

        if (update_players[i].thrust)
        {
            player->velocity.x += cosf(player->angle) * 0.125f;
            player->velocity.y += sinf(player->angle) * 0.125f;

            float speed = sqrtf(powf(player->velocity.x, 2.0f) + powf(player->velocity.y, 2.0f));

            if (speed > MAX_SPEED)
            {
                player->velocity.x *= MAX_SPEED / speed;
                player->velocity.y *= MAX_SPEED / speed;
            }
        }
    }

    translate_sprites(players, MAX_PLAYERS);
}

void send_packets()
{

}

void deinit()
{
    enet_host_destroy(server);
    enet_deinitialize();
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

    init();

    double accumulator = 0.0;
    double current_time = 0.0;
    double delta_time = 1 / 60.0;

    while (running)
    {
        double new_time = SDL_GetTicks() / 1000.0;
        double frame_time = new_time - current_time;

        current_time = new_time;
        accumulator += frame_time;

        receive_packets();

        while (accumulator >= delta_time)
        {
            update();
            accumulator -= delta_time;
        }

        send_packets();
    }

    deinit();
	return 0;
}