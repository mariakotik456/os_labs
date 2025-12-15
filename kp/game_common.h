#ifndef GAME_COMMON_H
#define GAME_COMMON_H

#include <stdbool.h>
#include <pthread.h>

#define MAX_GAMES 10
#define MAX_PLAYERS_PER_GAME 4
#define MAX_PLAYER_NAME 50
#define MAX_GAME_NAME 50
#define MAX_WORD_LENGTH 20
#define MAX_ATTEMPTS 100
#define SHM_NAME "/bulls_cows_shm"
#define SHM_SIZE (2 * 1024 * 1024)

typedef struct
{
    char guess[MAX_WORD_LENGTH];
    int bulls;
    int cows;
    int attempt_number;
    bool is_correct;
} AttemptResult;

typedef struct
{
    char name[MAX_PLAYER_NAME];
    int player_id;
    int score;
    int attempts_count;
    AttemptResult attempts[MAX_ATTEMPTS];
    bool is_connected;
    bool is_active;
    time_t last_activity;
} Player;

typedef struct
{
    char game_name[MAX_GAME_NAME];
    char secret_word[MAX_WORD_LENGTH];
    int max_players;
    int current_players;
    bool is_active;
    bool is_game_over;
    int winner_id;
    time_t created_at;
    time_t last_move_at;

    Player players[MAX_PLAYERS_PER_GAME];

    pthread_mutex_t game_mutex;
    pthread_cond_t game_cond;

    int total_moves;
    int words_tried;
} Game;

typedef enum
{
    MSG_CREATE_GAME,
    MSG_JOIN_GAME,
    MSG_MAKE_GUESS,
    MSG_LEAVE_GAME,
    MSG_LIST_GAMES,
    MSG_GAME_UPDATE,
    MSG_GAME_OVER,
    MSG_ERROR
} MessageType;

typedef struct
{
    MessageType type;
    char game_name[MAX_GAME_NAME];
    char player_name[MAX_PLAYER_NAME];
    char data[MAX_WORD_LENGTH * 2];
    int player_id;
    int max_players;
    int result_code;
    time_t timestamp;
} GameMessage;

typedef struct
{
    Game games[MAX_GAMES];
    int active_games;
    pthread_mutex_t shm_mutex;
    char server_status[100];
} SharedMemoryData;

void init_game(Game *game, const char *name, const char *secret_word, int max_players);
bool add_player_to_game(Game *game, const char *player_name, int *player_id);
bool remove_player_from_game(Game *game, int player_id);
void calculate_bulls_cows(const char *secret, const char *guess, int *bulls, int *cows);
bool is_game_joinable(const Game *game);
void broadcast_game_update(Game *game, SharedMemoryData *shm_data);

#endif