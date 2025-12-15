#ifndef CLIENT_H
#define CLIENT_H

#include "game_common.h"

int init_client_connection();
bool create_game_client(const char *game_name, const char *secret_word, int max_players);
bool join_game_client(const char *game_name, const char *player_name);
bool make_guess_client(const char *game_name, int player_id, const char *guess);
bool leave_game_client(const char *game_name, int player_id);
void list_games_client();
void print_game_status(const char *game_name);

#endif