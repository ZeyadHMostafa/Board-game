#include "data_manager.h"
#include "function_utilities.h"
#include "board_moves.h"
#include "board_analysis.h"
#include "display_utilities.h"

// Deprecated format
bool write_to_file(bitboard board[2], float rating, uint8_t player_index, std::string filename) {
	std::ofstream ofile(filename, std::fstream::out | std::fstream::binary | std::fstream::app);
	ofile.write((const char*)&board[player_index], sizeof(bitboard));
	ofile.write((const char*)&board[1 - player_index], sizeof(bitboard));
	ofile.write((const char*)&rating, sizeof(float));
	ofile.close();
	return true;
}

// Deprecated format
bool read_from_file(bitboard board[2], float& rating, uint8_t player_index, std::string filename) {
	std::ifstream ifile(filename, std::fstream::in | std::fstream::binary);
	ifile.read((char*)&board[player_index], sizeof(bitboard));
	ifile.read((char*)&board[1 - player_index], sizeof(bitboard));
	ifile.read((char*)&rating, sizeof(float));
	ifile.close();
	return true;
}

void gather_data(bitboard initial_board[2], std::string filename) {
	for (int board_index = 0; board_index < 1000; board_index++) {
		bitboard current_board[2] = { initial_board[0],initial_board[1] };

		uint8_t current_player_index = 0;
		// vary starting position slightly
		for (int i = 0; i < 10; i++) {
			uint8_t control_board[64] = {};
			ratedmovevector moves = find_board_moves(current_board, current_player_index, control_board);
			uint16_t move = 0;
			if (moves.size() == 0) {
				break;
			}
			move = pick_move(current_board, moves, current_player_index, 0);

			make_move(current_board, move, current_player_index);
			current_player_index = 1 - current_player_index;
		}
		// show starting board
		show_board(current_board);
		// keep track to prevent repetition
		bitboard last_4_boards[4][2] = { {0,0}, {1,1}, {2,2}, {4,4} };
		bitboard last_board_index = 0;
		// play a 100 moves
		for (int i = 0; i < 100; i++) {
			// check if the position is repeated after 4 moves (2 full turns)
			// uses buffer
			if (boards_equal(last_4_boards[last_board_index], current_board)) {
				break;
			}
			std::memcpy(last_4_boards[last_board_index], current_board, sizeof(bitboard) * 2);
			last_board_index = (last_board_index + 1) % 4;

			uint8_t control_board[64] = {};
			ratedmovevector moves = find_board_moves(current_board, current_player_index, control_board);
			uint16_t move = 0;
			if (moves.size() == 0) {
				break;
			}

			move = pick_move(current_board, moves, current_player_index, 2, true, filename);

			make_move(current_board, move, current_player_index);
			current_player_index = 1 - current_player_index;
			std::cout << "-";
		}
		std::cout << "\n";
		show_board(current_board);
	}
}

// fixes deprecated format
void fix_data(bitboard board[2], float rating, std::ofstream& ofile) {
	uint8_t control_values[9] = {};
	uint8_t player_move_values[8] = {};
	uint8_t opponent_move_values[8] = {};
	float total_rating = 0.0;
	//p: player, o: opponent
	uint8_t control_board1[64] = {}, control_board2[64] = {};
	auto player_moves = find_board_moves(board, 0, control_board1);
	auto opponent_moves = find_board_moves(board, 1, control_board2);
	int8_t total_control_board[64];
	for (int i = 0; i < 64; i++) {
		total_control_board[i] = (control_board1[i] - control_board2[i]);
	}

	get_control_values(board, total_control_board, control_values, 0);
	get_move_values(board[1], total_control_board, player_moves, player_move_values);
	get_move_values(board[0], total_control_board, opponent_moves, opponent_move_values, 1);
	uint8_t input[25];
	std::memcpy(input, control_values, 9);
	std::memcpy(input + 9, player_move_values, 8);
	std::memcpy(input + 9 + 8, opponent_move_values, 8);

	ofile.write((const char*)&input, sizeof(uint8_t) * 25);
	ofile.write((const char*)&rating, sizeof(float));
}

// fixes depricated format for entire file
void fix_file_data(std::string filename) {
	std::ifstream ifile(filename, std::fstream::in | std::fstream::binary);
	std::ofstream ofile(filename + "_fixed", std::fstream::out | std::fstream::binary | std::fstream::app);
	while (!ifile.eof()) {
		bitboard board[2];
		float rating;
		ifile.read((char*)&board[0], sizeof(bitboard));
		ifile.read((char*)&board[1], sizeof(bitboard));
		ifile.read((char*)&rating, sizeof(float));
		fix_data(board, rating, ofile);
	}

	ifile.close();
	ofile.close();
}