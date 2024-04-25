#pragma once

#include "main.h"

void bytes_to_board(uint8_t bytes[16], bitboard board[2]);

int count_pieces(uint8_t board[16], uint8_t is_second_player);

int count_pieces(bitboard half_board);

uint8_t get_piece_at_pos(uint8_t board[16], uint8_t x, uint8_t y, uint8_t is_second_player);

uint8_t get_piece_from_byte(uint8_t board[16], uint8_t pos, uint8_t is_second_player);

uint8_t get_circular_byte(bitboard half_board, uint8_t x, uint8_t y);

uint8_t get_sides_from_circular_bytes(uint8_t b1, uint8_t b2);

uint8_t get_corners_from_circular_bytes(uint8_t b1, uint8_t b2);

bool boards_equal(bitboard board1[2], bitboard board2[2]);