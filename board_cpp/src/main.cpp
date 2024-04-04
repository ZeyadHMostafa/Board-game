#include "main.h"
#include "setup.h"
#include "display_utilities.h"
#include "function_utilities.h"
#include "data_manager.h"
#include "board_moves.h"
#include "board_analysis.h"

// used to test the amount of time taken during earlier parts of development
void do_stress_test(bitboard test_board[2], int n) {
	clock_t start, end;
	start = clock();
	bitboard new_test_board[2] = { test_board[0], test_board[1] };
	for (int i = 0; i < n; i++) {
		uint8_t control_board[64] = {};
		ratedmovevector player_moves,opponent_moves;
		player_moves = find_board_moves(test_board, 0, control_board);
		rate_board_moves(new_test_board, player_moves, 0,1,0);
		
		//pseudo_rate_board(new_test_board, 0, player_moves, opponent_moves);
		new_test_board[0] = new_test_board[0] + 1;
		show_board(new_test_board);
	}
	end = clock();
	double time_taken = double(end) - double(start) / double(CLOCKS_PER_SEC);
	std::cout << "time taken by program is : " << time_taken << " seconds\n";
}

// an interactive version of the game that plays in the terminal
void auto_play_game(bitboard starting_board[2], int p1_intel = -1, int p2_intel = 2) {
	std::cout << "the user controls the first player (O) and the opponent controls (X)\n";
	std::cout << "to make a move type the starting position followed by the final position\n";
	std::cout << "without spaces, EX: 5755: moves the piece at (5,7) to (5,5)\n";
	std::cout << "type auto to let an AI pick a move for you\n";
	std::cout << "type check to show the AI top moves\n";
	bitboard current_board[2] = { starting_board[0],starting_board[1] };
	int players_intelligence[2] = { p1_intel,p2_intel };

	uint8_t current_player_index = 0;
	// limited to 100,000 moves per game
	for (int i = 0; i < 100000; i++) {
		uint8_t control_board[64] = {};
		// start by finding all legal moves
		ratedmovevector moves =	find_board_moves(current_board, current_player_index, control_board);
		uint16_t move = 0;
		if (moves.size() == 0) {
			break;
		}
		int player_intelligence = players_intelligence[current_player_index];
		if (player_intelligence != -1) {
			move = pick_move(current_board, moves, current_player_index, player_intelligence);
		}
		else {
			// uncomment next line if yo want a preview of the 3 best moves as scored by a depth 1 AI
			// pick_move(current_board, moves, current_player_index, 1);
			ratedmovevector opponent_moves;
			uint8_t o_control_board[64] = {};
			pseudo_rate_board(current_board, current_player_index, moves, opponent_moves,control_board,o_control_board);
			while (!move_legal(moves, move)) {
				// prompt the player to enter a legal move
				std::cout << "play move:\n";
				std::string input;
				std::getline(std::cin, input);
				if (input == "auto") {
					move = pick_move(current_board, moves, current_player_index, 2);
				}
				else if(input == "check") {
					pick_move(current_board, moves, current_player_index, 2);
				}
				else {
					move = translate_to_move(input);
				}
			}
		}
		
		make_move(current_board, move, current_player_index);
		current_player_index = 1 - current_player_index;
		Sleep(500);
		show_board(current_board);
		std::cout << "\n";
		// was used to run long games
		/*if (i < 20 || true) {
			Sleep(500);
			show_board(current_board);
			std::cout << "\n";
		}
		else if (i%5 == 0) {
			std::cout << "skipping five moves\n";
			Sleep(500);
			show_board(current_board);
			std::cout << "\n";
		}*/

	}
	show_board(current_board);
}

int main() {
	initialize();
	uint8_t test_board_pre[16] = {

		0b00001100,
		0b00001100,
		0b00110000,
		0b00110000,
		0b00001100,
		0b00001100,
		0b00110000,
		0b00110000,

		0b00110000,
		0b00110000,
		0b00001100,
		0b00001100,
		0b00110000,
		0b00110000,
		0b00001100,
		0b00001100,
	};

	bitboard test_board[2];
	bytes_to_board(test_board_pre,test_board);

	show_board(test_board);
	
	std::cout << "\n";

	//fix_file_data(); // used to fix deprecated file format
	//gather_data(test_board,"data4"); // used to gather data and place into a file
	//show_board_moves(test_board, 0); // to show available moves for a specific board
	//do_stress_test(test_board, 10); // to do a stress test

	auto_play_game(test_board,-1,2);
	return 0;
}