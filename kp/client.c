#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include "client.h"
#include "game_common.h"

static SharedMemoryData *shm_data = NULL;
static int shm_fd = -1;
static int client_game_id = -1;
static int client_player_id = -1;
static char current_game_name[MAX_GAME_NAME] = "";

int init_client_connection()
{
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("Не удалось открыть shared memory");
        return -1;
    }

    shm_data = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_data == MAP_FAILED)
    {
        perror("Не удалось маппировать shared memory");
        close(shm_fd);
        return -1;
    }

    printf("Подключение к серверу установлено\n");
    return 0;
}

bool create_game_client(const char *game_name, const char *secret_word, int max_players)
{
    if (!shm_data)
    {
        printf("Нет подключения к серверу\n");
        return false;
    }

    pthread_mutex_lock(&shm_data->shm_mutex);

    for (int i = 0; i < shm_data->active_games; i++)
    {
        if (shm_data->games[i].is_active &&
            strcmp(shm_data->games[i].game_name, game_name) == 0)
        {
            pthread_mutex_unlock(&shm_data->shm_mutex);
            printf("Игра с именем '%s' уже существует\n", game_name);
            return false;
        }
    }

    int free_slot = -1;
    for (int i = 0; i < MAX_GAMES; i++)
    {
        if (!shm_data->games[i].is_active)
        {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1)
    {
        pthread_mutex_unlock(&shm_data->shm_mutex);
        printf("Достигнуто максимальное количество игр\n");
        return false;
    }

    Game *new_game = &shm_data->games[free_slot];

    pthread_mutex_lock(&new_game->game_mutex);
    pthread_mutex_unlock(&shm_data->shm_mutex);

    init_game(new_game, game_name, secret_word, max_players);

    char player_name[50];
    printf("Введите ваше имя: ");
    fgets(player_name, sizeof(player_name), stdin);
    player_name[strcspn(player_name, "\n")] = 0;

    int player_id;
    if (!add_player_to_game(new_game, player_name, &player_id))
    {
        pthread_mutex_unlock(&new_game->game_mutex);
        printf("Не удалось добавить игрока в игру\n");
        return false;
    }

    shm_data->active_games++;
    client_game_id = free_slot;
    client_player_id = player_id;
    strcpy(current_game_name, game_name);

    printf("Игра '%s' создана успешно. Ваш ID: %d\n", game_name, player_id);

    pthread_mutex_unlock(&new_game->game_mutex);
    return true;
}

bool join_game_client(const char *game_name, const char *player_name)
{
    if (!shm_data)
    {
        printf("Нет подключения к серверу\n");
        return false;
    }

    pthread_mutex_lock(&shm_data->shm_mutex);

    Game *target_game = NULL;
    int found_game_id = -1;

    for (int i = 0; i < MAX_GAMES; i++)
    {
        if (shm_data->games[i].is_active &&
            strcmp(shm_data->games[i].game_name, game_name) == 0)
        {
            target_game = &shm_data->games[i];
            found_game_id = i;
            break;
        }
    }

    if (!target_game)
    {
        pthread_mutex_unlock(&shm_data->shm_mutex);
        printf("Игра '%s' не найдена\n", game_name);
        return false;
    }

    pthread_mutex_lock(&target_game->game_mutex);
    pthread_mutex_unlock(&shm_data->shm_mutex);

    if (!is_game_joinable(target_game))
    {
        pthread_mutex_unlock(&target_game->game_mutex);
        printf("Невозможно присоединиться к игре '%s'\n", game_name);
        return false;
    }

    int player_id;
    if (!add_player_to_game(target_game, player_name, &player_id))
    {
        pthread_mutex_unlock(&target_game->game_mutex);
        printf("Не удалось присоединиться к игре\n");
        return false;
    }

    client_game_id = found_game_id;
    client_player_id = player_id;
    strcpy(current_game_name, game_name);

    printf("Вы присоединились к игре '%s' как игрок %d\n", game_name, player_id);

    printf("Текущие игроки в игре:\n");
    for (int i = 0; i < target_game->current_players; i++)
    {
        if (target_game->players[i].is_connected)
        {
            printf("- %s (ID: %d)\n", target_game->players[i].name, i);
        }
    }

    pthread_mutex_unlock(&target_game->game_mutex);
    return true;
}

bool make_guess_client(const char *game_name, int player_id, const char *guess)
{
    if (!shm_data)
    {
        printf("Нет подключения к серверу\n");
        return false;
    }

    pthread_mutex_lock(&shm_data->shm_mutex);

    Game *game = NULL;

    for (int i = 0; i < MAX_GAMES; i++)
    {
        if (shm_data->games[i].is_active &&
            strcmp(shm_data->games[i].game_name, game_name) == 0)
        {
            game = &shm_data->games[i];
            break;
        }
    }

    if (!game)
    {
        pthread_mutex_unlock(&shm_data->shm_mutex);
        printf("Игра '%s' не найдена\n", game_name);
        return false;
    }

    pthread_mutex_lock(&game->game_mutex);
    pthread_mutex_unlock(&shm_data->shm_mutex);

    if (player_id < 0 || player_id >= game->current_players ||
        !game->players[player_id].is_connected)
    {
        pthread_mutex_unlock(&game->game_mutex);
        printf("Игрок с ID %d не найден или не активен\n", player_id);
        return false;
    }

    int bulls, cows;
    calculate_bulls_cows(game->secret_word, guess, &bulls, &cows);

    printf("Результат: Быки: %d, Коровы: %d\n", bulls, cows);

    Player *player = &game->players[player_id];
    player->attempts_count++;
    player->last_activity = time(NULL);

    if ((size_t)bulls == strlen(game->secret_word))
    {
        game->is_game_over = true;
        game->winner_id = player_id;
        printf("\nПОЗДРАВЛЯЕМ! Вы угадали слово '%s'!\n", game->secret_word);
        printf("Игра завершена. Победитель: %s\n", player->name);
    }

    pthread_mutex_unlock(&game->game_mutex);
    return true;
}

bool leave_game_client(const char *game_name, int player_id)
{
    if (!shm_data)
    {
        printf("Нет подключения к серверу\n");
        return false;
    }

    pthread_mutex_lock(&shm_data->shm_mutex);

    Game *game = NULL;

    for (int i = 0; i < MAX_GAMES; i++)
    {
        if (shm_data->games[i].is_active &&
            strcmp(shm_data->games[i].game_name, game_name) == 0)
        {
            game = &shm_data->games[i];
            break;
        }
    }

    if (!game)
    {
        pthread_mutex_unlock(&shm_data->shm_mutex);
        printf("Игра '%s' не найдена\n", game_name);
        return false;
    }

    pthread_mutex_lock(&game->game_mutex);
    pthread_mutex_unlock(&shm_data->shm_mutex);

    if (!remove_player_from_game(game, player_id))
    {
        pthread_mutex_unlock(&game->game_mutex);
        printf("Не удалось выйти из игры\n");
        return false;
    }

    printf("Вы вышли из игры '%s'\n", game_name);

    int active_players = 0;
    for (int i = 0; i < game->current_players; i++)
    {
        if (game->players[i].is_connected)
        {
            active_players++;
        }
    }

    if (active_players == 0 && !game->is_game_over)
    {
        game->is_active = false;
        shm_data->active_games--;
        printf("Игра '%s' завершена (нет активных игроков)\n", game_name);
    }

    client_game_id = -1;
    client_player_id = -1;
    current_game_name[0] = '\0';

    pthread_mutex_unlock(&game->game_mutex);
    return true;
}

void list_games_client()
{
    if (!shm_data)
    {
        printf("Нет подключения к серверу\n");
        return;
    }

    pthread_mutex_lock(&shm_data->shm_mutex);

    printf("\n=== Список активных игр ===\n");

    int found_games = 0;
    for (int i = 0; i < MAX_GAMES; i++)
    {
        if (shm_data->games[i].is_active)
        {
            Game *game = &shm_data->games[i];

            pthread_mutex_lock(&game->game_mutex);

            int active_players = 0;
            for (int j = 0; j < game->current_players; j++)
            {
                if (game->players[j].is_connected)
                {
                    active_players++;
                }
            }

            printf("Игра: %s\n", game->game_name);
            printf("  Игроков: %d/%d\n", active_players, game->max_players);
            printf("  Статус: %s\n", game->is_game_over ? "Завершена" : "Активна");

            if (game->is_game_over && game->winner_id != -1)
            {
                printf("  Победитель: %s\n", game->players[game->winner_id].name);
            }

            pthread_mutex_unlock(&game->game_mutex);
            printf("------------------------\n");
            found_games++;
        }
    }

    if (found_games == 0)
    {
        printf("Нет активных игр\n");
    }

    pthread_mutex_unlock(&shm_data->shm_mutex);
}

void print_game_status(const char *game_name)
{
    if (!shm_data)
    {
        printf("Нет подключения к серверу\n");
        return;
    }

    pthread_mutex_lock(&shm_data->shm_mutex);

    Game *game = NULL;

    for (int i = 0; i < MAX_GAMES; i++)
    {
        if (shm_data->games[i].is_active &&
            strcmp(shm_data->games[i].game_name, game_name) == 0)
        {
            game = &shm_data->games[i];
            break;
        }
    }

    if (!game)
    {
        pthread_mutex_unlock(&shm_data->shm_mutex);
        printf("Игра '%s' не найдена\n", game_name);
        return;
    }

    pthread_mutex_lock(&game->game_mutex);
    pthread_mutex_unlock(&shm_data->shm_mutex);

    printf("\n=== Статус игры '%s' ===\n", game_name);
    printf("Загаданное слово: [скрыто]\n");
    printf("Максимум игроков: %d\n", game->max_players);
    printf("Текущих игроков: %d\n", game->current_players);
    printf("Статус: %s\n", game->is_game_over ? "Завершена" : "Активна");

    if (game->is_game_over)
    {
        printf("Победитель: %s\n", game->players[game->winner_id].name);
    }

    printf("\nИгроки:\n");
    for (int i = 0; i < game->current_players; i++)
    {
        Player *player = &game->players[i];
        printf("- %s (ID: %d) [%s]\n",
               player->name, i,
               player->is_connected ? "в сети" : "не в сети");

        if (player->attempts_count > 0)
        {
            printf("  Попыток: %d, Последняя: ", player->attempts_count);
            if (player->attempts_count > 0)
            {
                AttemptResult *last = &player->attempts[player->attempts_count - 1];
                printf("'%s' - Быки: %d, Коровы: %d\n",
                       last->guess, last->bulls, last->cows);
            }
        }
    }

    pthread_mutex_unlock(&game->game_mutex);
}

void client_main_menu()
{
    printf("\n=== Клиент 'Быки и Коровы' ===\n");

    if (init_client_connection() != 0)
    {
        printf("Не удалось подключиться к серверу. Убедитесь, что сервер запущен.\n");
        return;
    }

    while (1)
    {
        printf("\nГлавное меню:\n");
        printf("1. Создать новую игру\n");
        printf("2. Присоединиться к существующей игре\n");
        printf("3. Сделать предположение (если в игре)\n");
        printf("4. Показать список игр\n");
        printf("5. Показать статус текущей игры\n");
        printf("6. Выйти из игры\n");
        printf("7. Выход из программы\n");
        printf("Выберите действие: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
        {
            char game_name[MAX_GAME_NAME];
            char secret_word[MAX_WORD_LENGTH];
            int max_players;

            printf("Введите название игры: ");
            fgets(game_name, sizeof(game_name), stdin);
            game_name[strcspn(game_name, "\n")] = 0;

            printf("Введите загаданное слово: ");
            fgets(secret_word, sizeof(secret_word), stdin);
            secret_word[strcspn(secret_word, "\n")] = 0;

            printf("Введите количество игроков (макс %d): ", MAX_PLAYERS_PER_GAME);
            scanf("%d", &max_players);
            getchar();

            create_game_client(game_name, secret_word, max_players);
            break;
        }

        case 2:
        {
            char game_name[MAX_GAME_NAME];
            char player_name[MAX_PLAYER_NAME];

            list_games_client();

            printf("Введите название игры для присоединения: ");
            fgets(game_name, sizeof(game_name), stdin);
            game_name[strcspn(game_name, "\n")] = 0;

            printf("Введите ваше имя: ");
            fgets(player_name, sizeof(player_name), stdin);
            player_name[strcspn(player_name, "\n")] = 0;

            join_game_client(game_name, player_name);
            break;
        }

        case 3:
        {
            if (client_game_id == -1)
            {
                printf("Вы не в игре. Присоединитесь к игре сначала.\n");
                break;
            }

            char guess[MAX_WORD_LENGTH];
            printf("Введите ваше предположение: ");
            fgets(guess, sizeof(guess), stdin);
            guess[strcspn(guess, "\n")] = 0;

            make_guess_client(current_game_name, client_player_id, guess);
            break;
        }

        case 4:
            list_games_client();
            break;

        case 5:
            if (client_game_id == -1)
            {
                printf("Вы не в игре.\n");
            }
            else
            {
                print_game_status(current_game_name);
            }
            break;

        case 6:
            if (client_game_id == -1)
            {
                printf("Вы не в игре.\n");
            }
            else
            {
                leave_game_client(current_game_name, client_player_id);
            }
            break;

        case 7:
            printf("Выход...\n");
            if (shm_data)
            {
                munmap(shm_data, SHM_SIZE);
            }
            if (shm_fd != -1)
            {
                close(shm_fd);
            }
            return;

        default:
            printf("Неверный выбор. Попробуйте снова.\n");
        }
    }
}

int main()
{
    client_main_menu();
    return 0;
}