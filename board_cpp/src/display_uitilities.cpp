#include "function_utilities.h"
#include "board_moves.h"

/*
	used to display the board and other useful data
*/

// translate move to a player readable format
//(xyXY) 
// x,y: starting position
// X,Y: final position
// 00: bottom right corner
void print_move(uint16_t move) {
	std::cout << "move (" << int((move >> 8) & 7) << "," << int((move >> 11) & 7) << ") to (";
	std::cout << int((move) & 7) << "," << int((move >> 3) & 7) << ")\n";
}

void show_board(bitboard board[2]) {
	std::cout << " ___________________\n|                   |\n";
	for (int i = 0; i < 8; i++) {
		std::cout << "|  ";
		for (int j = 7; j >= 0; j--) {
			if ((board[0] & (LL1 << (j + 8 * i))) != 0) {
				std::cout << "O ";
			}
			else if ((board[1] & (LL1 << (j + 8 * i))) != 0) {
				std::cout << "X ";
			}
			else {
				std::cout << "- ";
			}
		}
		std::cout << " |" << i << "\n";
	}
	std::cout << "|___________________|\n   7 6 5 4 3 2 1 0" << "\n";
}

// translates the player's input to a move
uint16_t translate_to_move(std::string line) {
	int values[4];
	uint16_t move = 0;
	if (line.length() != 4) {
		return 0;
	}
	for (int i = 0; i < 4; i++) {
		values[i] = int(line[i]) - int('0');
	}
	for (int i = 0; i < 2; i++) {
		if (values[i] >= 0 && values[i] < 8) {
			move |= values[i] << (3 * i + 8);
		}
	}
	for (int i = 0; i < 2; i++) {
		if (values[i + 2] >= 0 && values[i + 2] < 8) {
			move |= values[i + 2] << (3 * i);
		}
	}
	return move;
}

void show_board_moves(bitboard board[2], uint8_t is_second_player) {
	uint8_t control_board[64] = {};
	auto moves = find_board_moves(board, is_second_player, control_board);
	///*
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			std::cout << int(control_board[i * 8 + 7 - j]) << "\t";
		}
		std::cout << "\n";
	}
	//*/
	for (int i = 0; i < moves.size(); i++) {
		print_move(moves[i].move);
	}

	std::cout << "move_count = " << moves.size() << "\n";
}