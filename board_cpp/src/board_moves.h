#pragma once

#include "main.h"

void get_axial_moves(bitboard board[2], uint8_t pos, uint8_t is_second_player, MoveContainer& moves, uint8_t control_board[64]);

void get_diagonal_moves(bitboard board[2], uint8_t pos, uint8_t is_second_player, MoveContainer& moves, uint8_t control_board[64]);

ratedmovevector find_board_moves(bitboard board[2], uint8_t is_second_player, uint8_t control_board[64]);

void make_move(bitboard board[2], uint16_t move, uint8_t player_index);

bool move_legal(ratedmovevector moves, uint16_t move);