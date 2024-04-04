#include "board_analysis.h"
#include "data_manager.h"
#include "display_utilities.h"
#include "function_utilities.h"
#include "board_moves.h"

// find how effective each move is
void get_move_values(bitboard opponent_half_board, int8_t control_board[64], ratedmovevector moves,
	uint8_t move_values[8], uint8_t reversed) {
	//initial_protected / final_protected / final_attacking ( ifa )
	//___, __a, _f_, _fa,  i__, i_a, if_, ifa,
	for (int i = 0; i < moves.size(); i++) {
		uint8_t initial_protected = (uint8_t)(signed((LLC[reversed] ^ control_board[moves[i].move >> 8]) + reversed) >= 0) << 2;
		uint8_t final_protected = (uint8_t)(signed((LLC[reversed] ^ control_board[moves[i].move & 0xFF]) + reversed) > 0) << 1;
		uint8_t final_attacking = (uint8_t)(((opponent_half_board >> (moves[i].move & 0xFF)) & 1) != 0);
		move_values[initial_protected | final_protected | final_attacking]++;
	}
}

// find who is in control of each tile and by how much
void get_control_values(bitboard board[2], int8_t control_board[64], uint8_t values[9], uint8_t player_index) {
	//p:player, o:opponent, n:neutral, (piece _ control)
	//p_p, p_n, p_o,
	//n_p, n_n, n_o,
	//o_p, o_n, o_o,

	bitboard p1_control_bitboard = 0;
	bitboard p2_control_bitboard = 0;
	for (int i = 0; i < 64; i++) {
		int diff = control_board[i];
		p1_control_bitboard |= (LL1 << i) & LLC[int(diff > 0)];
		p2_control_bitboard |= (LL1 << i) & LLC[int(diff < 0)];
	}
	//print - was used for debugging purposes
	/*
	std::cout << "control board:\n";
	bitboard c_board[2] = {p1_control_bitboard, p2_control_bitboard};
	for (int i = 0; i < 64; i++) {
		std::cout << int(control_board[i]) << " ";
		if (i % 8 == 7) {
			std::cout << "\n";
		}
	}
	std::cout << "control bitboard:\n";
	show_board(c_board);
	//end print
	*/
	uint8_t opponent_index = 1 - player_index;
	bitboard n_conrol_bitboard = ~(p1_control_bitboard | p2_control_bitboard);
	bitboard n_bitboard = ~(board[player_index] | board[opponent_index]);

	values[0] = count_pieces(p1_control_bitboard & board[player_index]);
	values[1] = count_pieces(n_conrol_bitboard & board[player_index]);
	values[2] = count_pieces(p2_control_bitboard & board[player_index]);

	values[3] = count_pieces(p1_control_bitboard & n_bitboard);
	values[4] = count_pieces(n_conrol_bitboard & n_bitboard);
	values[5] = count_pieces(p2_control_bitboard & n_bitboard);

	values[6] = count_pieces(p1_control_bitboard & board[opponent_index]);
	values[7] = count_pieces(n_conrol_bitboard & board[opponent_index]);
	values[8] = count_pieces(p2_control_bitboard & board[opponent_index]);

}

// used to rate the board with zero depth
float pseudo_rate_board(bitboard board[2], uint8_t player_index,
	ratedmovevector& player_moves, ratedmovevector& opponent_moves,
	uint8_t control_board1[64], uint8_t control_board2[64]) {
	uint8_t control_values[9] = {};
	uint8_t player_move_values[8] = {};
	uint8_t opponent_move_values[8] = {};
	float total_rating = 0.0;
	//p: player, o: opponent
	if (player_moves.empty()) {
		player_moves = find_board_moves(board, player_index, control_board1);
	}
	if (player_moves.empty()) {
		return -200.0;
	}
	if (opponent_moves.empty()) {
		opponent_moves = find_board_moves(board, 1 - player_index, control_board2);
	}
	if (opponent_moves.empty()) {
		return 200.0;
	}
	int8_t total_control_board[64];
	for (int i = 0; i < 64; i++) {
		total_control_board[i] = (control_board1[i] - control_board2[i]);
	}

	get_control_values(board, total_control_board, control_values, player_index);
	get_move_values(board[1 - player_index], total_control_board, player_moves, player_move_values);
	get_move_values(board[player_index], total_control_board, opponent_moves, opponent_move_values, 1);
	uint8_t input[25];
	std::memcpy(input, control_values, 9);
	std::memcpy(input + 9, player_move_values, 8);
	std::memcpy(input + 9 + 8, opponent_move_values, 8);
	for (int i = 0; i < 25; i++) {
		total_rating += MODEL_WEIGHTS[i] * input[i];
		if (i == 8) {
			//std::cout << "intermediate rating =\t" << total_rating << "\n";
		}
		else if (i > 8) {
			//std::cout << "\t" << int(input[i]);
		}
		if (i == 9 + 7) {
			//std::cout << "\n";
		}
	}
	//std::cout << "\n";
	//std::cout << "total rating = \t\t" << total_rating << "\n";
	return total_rating;
}

void pseudo_rate_board_moves(bitboard board[2], ratedmovevector& moves, uint8_t player_index) {
	if (moves.size() == 0) {
		return;
	}
	for (int i = 0; i < moves.size(); i++) {
		auto move = &moves[i];
		bitboard new_board[2] = { board[0],board[1] };
		make_move(new_board, move->move, player_index);
		ratedmovevector player_moves, opponent_moves;
		uint8_t control_board1[64] = {}, control_board2[64] = {};
		//print_move(move->move);
		move->rating = pseudo_rate_board(new_board, player_index, player_moves, opponent_moves, control_board1, control_board2);
	}
}

float rate_board_moves(bitboard board[2], ratedmovevector& moves, uint8_t player_index, uint8_t depth,
	float cutoff_percentage, float cutoff_fade) {
	const int move_size = moves.size();
	if (move_size == 0) {
		return -200;
	}
	ratedmovevector* opponent_moves_collection = new ratedmovevector[move_size];
	auto boards_collection = new bitboard[move_size][2];
	for (int i = 0; i < move_size; i++) {
		ratedmovevector player_moves;
		boards_collection[i][0] = board[0];
		boards_collection[i][1] = board[1];
		make_move(boards_collection[i], moves[i].move, player_index);
		uint8_t control_board1[64]={0}, control_board2[64]={0};
		moves[i].rating = pseudo_rate_board(boards_collection[i], player_index,
			player_moves, opponent_moves_collection[i],
			control_board1, control_board2);
	}

	if (depth > 0) {
		/*std::vector<RatedMoveIndex> move_indicies(move_size);
		for (int i = 0; i < move_size; i++) {
			move_indicies[i].index = i;
			move_indicies[i].rating = moves[i].rating;
		std::sort(move_indicies.begin(), move_indicies.end(), less_than_key());
		}*/
		float min_rating = moves[0].rating;
		float max_rating = min_rating;
		for (int i = 0; i < moves.size(); i++) {
			min_rating = std::min(moves[i].rating, min_rating);
			max_rating = std::max(moves[i].rating, max_rating);
		}
		float rating_comparison = cutoff_percentage * (max_rating - min_rating) + min_rating;
		float new_cutoff = 1 - cutoff_fade * (1 - cutoff_percentage);

		for (int i = 0; i < move_size; i++) {
			auto move = &moves[i];
			if (move->rating >= rating_comparison) {
				ratedmovevector player_moves = opponent_moves_collection[i];
				move->rating = -rate_board_moves(boards_collection[i], player_moves, 1 - player_index, depth - 1, new_cutoff);
			}
		}
	}
	delete[] opponent_moves_collection;
	delete[] boards_collection;

	float board_max_rating = moves[0].rating;
	float board_rating = 0;
	for (int i = 0; i < move_size; i++) {
		board_rating += moves[i].rating;
		board_max_rating = std::max(board_max_rating, moves[i].rating);
	}
	board_rating /= move_size;
	board_rating = 1.0 * board_max_rating + 0.0 * board_rating;
	return board_rating;
}

// a wrapper for the python version
uint8_t python_find_rated_board_moves(bitboard board[2], RatedMove pymoves[80],
	uint8_t control_board1[64],uint8_t control_board2[64], uint8_t player_index,
	 float &board_rating, uint8_t depth, float cutoff_percentage, float cutoff_fade){
	ratedmovevector moves = find_board_moves(board, player_index, control_board1);
	
	if (depth!=255){
		find_board_moves(board, 1-player_index, control_board2);
		board_rating = rate_board_moves(board, moves, player_index, depth,
		cutoff_percentage, cutoff_fade);
	}

	for (int i =0; i <moves.size();i++){
		std::memcpy(&pymoves[i],&moves[i],sizeof(moves[i]));
	}
	return moves.size();
}

uint16_t pick_move(bitboard current_board[2], ratedmovevector moves, uint8_t current_player_index,
	uint8_t steps, bool save_result, std::string filename) {
	uint16_t move = 0;
	float board_rating = rate_board_moves(current_board, moves, current_player_index, steps, 0.0, 1.0);
	if (save_result) {
		write_to_file(current_board, board_rating, current_player_index, filename);
	}
	std::sort(moves.begin(), moves.end(), less_than_key());
	int move_diff = (rand() % 3);
	int move_index = std::max(0, (int)(moves.size() - move_diff - 1));

	if (moves[move_index].rating + 3 < moves.back().rating) {
		move_index = moves.size() - 1;
	}

	for (int i = moves.size() - 1; i >= std::max(0, (int)(moves.size() - 3)); i--) {
		print_move(moves[i].move);
		std::cout << moves[i].rating << "\n";
	}

	move = moves[move_index].move;

	return move;
}
