#pragma once

#include "main.h"

struct less_than_key
{
	inline bool operator() (const RatedMove& struct1, const RatedMove& struct2)
	{
		return (struct1.rating < struct2.rating);
	}
	inline bool operator() (const RatedMoveIndex& struct1, const RatedMoveIndex& struct2)
	{
		return (struct1.rating < struct2.rating);
	}
};

void get_move_values(bitboard opponent_half_board, int8_t control_board[64], ratedmovevector moves,
	uint8_t move_values[8], uint8_t reversed = 0);

void get_control_values(bitboard board[2], int8_t control_board[64], uint8_t values[9], uint8_t player_index);

float pseudo_rate_board(bitboard board[2], uint8_t player_index,
	ratedmovevector& player_moves, ratedmovevector& opponent_moves,
	uint8_t control_board1[64], uint8_t control_board2[64]);

void pseudo_rate_board_moves(bitboard board[2], ratedmovevector& moves, uint8_t player_index);

float rate_board_moves(bitboard board[2], ratedmovevector& moves, uint8_t player_index, uint8_t depth,
	float cutoff_percentage, float cutoff_fade = 0.75);


uint16_t pick_move(bitboard current_board[2], ratedmovevector moves, uint8_t current_player_index,
	uint8_t steps = 0, bool save_result = false, std::string filename = "data");