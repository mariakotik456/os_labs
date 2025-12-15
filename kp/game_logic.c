#include "game_common.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

void init_game(Game *game, const char *name, const char *secret_word, int max_players)
{
    memset(game, 0, sizeof(Game));

    strncpy(game->game_name, name, MAX_GAME_NAME - 1);
    strncpy(game->secret_word, secret_word, MAX_WORD_LENGTH - 1);

    game->max_players = (max_players > MAX_PLAYERS_PER_GAME) ? MAX_PLAYERS_PER_GAME : max_players;
    game->current_players = 0;
    game->is_active = true;
    game->is_game_over = false;
    game->winner_id = -1;
    game->total_moves = 0;
    game->words_tried = 0;

    game->created_at = time(NULL);
    game->last_move_at = time(NULL);

    for (int i = 0; i < MAX_PLAYERS_PER_GAME; i++)
    {
        game->players[i].player_id = i;
        game->players[i].is_connected = false;
        game->players[i].is_active = false;
        game->players[i].score = 0;
        game->players[i].attempts_count = 0;
        game->players[i].last_activity = 0;
    }
}

bool add_player_to_game(Game *game, const char *player_name, int *player_id)
{
    if (!game->is_active || game->is_game_over)
    {
        return false;
    }

    if (game->current_players >= game->max_players)
    {
        return false;
    }

    for (int i = 0; i < game->current_players; i++)
    {
        if (strcmp(game->players[i].name, player_name) == 0)
        {
            if (game->players[i].is_connected)
            {
                return false;
            }
            else
            {
                game->players[i].is_connected = true;
                game->players[i].is_active = true;
                game->players[i].last_activity = time(NULL);
                *player_id = i;
                return true;
            }
        }
    }

    int new_id = game->current_players;
    Player *new_player = &game->players[new_id];

    strncpy(new_player->name, player_name, MAX_PLAYER_NAME - 1);
    new_player->player_id = new_id;
    new_player->score = 0;
    new_player->attempts_count = 0;
    new_player->is_connected = true;
    new_player->is_active = true;
    new_player->last_activity = time(NULL);

    game->current_players++;
    *player_id = new_id;

    return true;
}

bool remove_player_from_game(Game *game, int player_id)
{
    if (player_id < 0 || player_id >= game->current_players)
    {
        return false;
    }

    if (!game->players[player_id].is_connected)
    {
        return false;
    }

    game->players[player_id].is_connected = false;
    game->players[player_id].is_active = false;

    return true;
}

void calculate_bulls_cows(const char *secret, const char *guess, int *bulls, int *cows)
{
    *bulls = 0;
    *cows = 0;

    int secret_len = strlen(secret);
    int guess_len = strlen(guess);

    if (secret_len != guess_len)
    {
        return;
    }

    int secret_count[26] = {0};
    int guess_count[26] = {0};

    for (int i = 0; i < secret_len; i++)
    {
        if (secret[i] == guess[i])
        {
            (*bulls)++;
        }
        else
        {
            if (secret[i] >= 'a' && secret[i] <= 'z')
            {
                secret_count[secret[i] - 'a']++;
            }
            if (guess[i] >= 'a' && guess[i] <= 'z')
            {
                guess_count[guess[i] - 'a']++;
            }
        }
    }

    for (int i = 0; i < 26; i++)
    {
        *cows += (secret_count[i] < guess_count[i]) ? secret_count[i] : guess_count[i];
    }
}

bool is_game_joinable(const Game *game)
{
    return (game->is_active &&
            !game->is_game_over &&
            game->current_players < game->max_players);
}

void broadcast_game_update(Game *game, SharedMemoryData *shm_data)
{
    time_t now = time(NULL);

    snprintf(shm_data->server_status, sizeof(shm_data->server_status),
             "Game '%s': %d/%d players, last update: %ld",
             game->game_name, game->current_players,
             game->max_players, now);
}