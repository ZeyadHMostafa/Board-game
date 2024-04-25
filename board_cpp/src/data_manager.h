#pragma once

#include "main.h"

bool write_to_file(bitboard board[2], float rating, uint8_t player_index, std::string filename = "data");

bool read_from_file(bitboard board[2], float& rating, uint8_t player_index, std::string filename = "data");

void gather_data(bitboard initial_board[2], std::string filename);

void fix_data(bitboard board[2], float rating, std::ofstream& ofile);

void fix_file_data(std::string filename = "data4");