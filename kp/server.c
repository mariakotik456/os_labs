#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include "game_common.h"

static SharedMemoryData *shm_data = NULL;
static int shm_fd = -1;
static volatile bool server_running = true;
static pthread_t cleanup_thread;

SharedMemoryData *init_shared_memory()
{
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1)
    {
        perror("ftruncate");
        close(shm_fd);
        return NULL;
    }

    shm_data = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE,
                    MAP_SHARED, shm_fd, 0);
    if (shm_data == MAP_FAILED)
    {
        perror("mmap");
        close(shm_fd);
        return NULL;
    }

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm_data->shm_mutex, &mutex_attr);

    for (int i = 0; i < MAX_GAMES; i++)
    {
        Game *game = &shm_data->games[i];
        pthread_mutexattr_t game_mutex_attr;
        pthread_mutexattr_init(&game_mutex_attr);
        pthread_mutexattr_setpshared(&game_mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&game->game_mutex, &game_mutex_attr);

        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);
        pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&game->game_cond, &cond_attr);
    }

    shm_data->active_games = 0;
    strcpy(shm_data->server_status, "Server running");

    printf("Shared memory initialized successfully\n");
    return shm_data;
}

void *cleanup_thread_func(void *arg)
{
    (void)arg;

    while (server_running)
    {
        sleep(30);

        pthread_mutex_lock(&shm_data->shm_mutex);

        time_t now = time(NULL);

        for (int i = 0; i < MAX_GAMES; i++)
        {
            Game *game = &shm_data->games[i];

            if (!game->is_active)
                continue;

            pthread_mutex_lock(&game->game_mutex);

            for (int j = 0; j < game->current_players; j++)
            {
                Player *player = &game->players[j];

                if (player->is_connected &&
                    (now - player->last_activity) > 300)
                {
                    printf("Player '%s' timed out from game '%s'\n",
                           player->name, game->game_name);

                    player->is_connected = false;
                    player->is_active = false;

                    int active_count = 0;
                    for (int k = 0; k < game->current_players; k++)
                    {
                        if (game->players[k].is_connected)
                        {
                            active_count++;
                        }
                    }

                    if (active_count == 0)
                    {
                        game->is_active = false;
                        shm_data->active_games--;
                    }
                }
            }

            if (game->is_game_over && (now - game->last_move_at) > 3600)
            {
                printf("Cleaning up finished game '%s'\n", game->game_name);
                game->is_active = false;
                shm_data->active_games--;
            }

            pthread_mutex_unlock(&game->game_mutex);
        }

        pthread_mutex_unlock(&shm_data->shm_mutex);
    }

    return NULL;
}

int main()
{
    printf("Starting Bulls and Cows Server...\n");

    if (!init_shared_memory())
    {
        fprintf(stderr, "Failed to initialize shared memory\n");
        return 1;
    }

    pthread_create(&cleanup_thread, NULL, cleanup_thread_func, NULL);

    printf("Server is running. Press Ctrl+C to stop.\n");
    printf("Active games: %d\n", shm_data->active_games);

    while (server_running)
    {
        sleep(1);

        static time_t last_stats = 0;
        time_t now = time(NULL);

        if (now - last_stats >= 10)
        {
            pthread_mutex_lock(&shm_data->shm_mutex);
            printf("\n=== Server Status ===\n");
            printf("Active games: %d\n", shm_data->active_games);

            for (int i = 0; i < MAX_GAMES; i++)
            {
                Game *game = &shm_data->games[i];
                if (game->is_active)
                {
                    printf("  Game '%s': %d/%d players, %s\n",
                           game->game_name, game->current_players,
                           game->max_players,
                           game->is_game_over ? "FINISHED" : "ACTIVE");
                }
            }
            printf("=====================\n");
            pthread_mutex_unlock(&shm_data->shm_mutex);

            last_stats = now;
        }
    }

    printf("Shutting down server...\n");

    server_running = false;
    pthread_join(cleanup_thread, NULL);

    if (shm_data)
    {
        munmap(shm_data, SHM_SIZE);
    }
    if (shm_fd != -1)
    {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }

    printf("Server stopped\n");
    return 0;
}