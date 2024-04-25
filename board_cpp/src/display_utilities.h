#pragma once

#include "main.h"

void print_move(uint16_t move);

void show_board(bitboard board[2]);

uint16_t translate_to_move(std::string line);

void show_board_moves(bitboard board[2], uint8_t is_second_player);