#include "board_moves.h"
#include "function_utilities.h"

// converts to circular format and uses lookup table to find rotational moves
void get_axial_moves(bitboard board[2], uint8_t pos, uint8_t is_second_player, MoveContainer& moves, uint8_t control_board[64]) {
	uint8_t player_byte, opponent_byte, sides_byte, corners_byte;
	uint8_t off_limits_mask = AXIAL_MASKS[int((pos & 7) == 7)] | AXIAL_MASKS[2 + int((pos & 7) == 0)]
		| AXIAL_MASKS[4 + int((pos & 56) == 0)] | AXIAL_MASKS[6 + int((pos & 56) == 56)];
	uint8_t x = (pos & 7);
	uint8_t y = ((pos >> 3) & 7);
	player_byte = get_circular_byte(board[is_second_player], x, y);
	opponent_byte = get_circular_byte(board[1 - is_second_player], x, y);
	sides_byte = get_sides_from_circular_bytes(player_byte, opponent_byte);
	corners_byte = get_corners_from_circular_bytes(player_byte, opponent_byte) | off_limits_mask;
	uint8_t* premade_moves = AXIAL_MOVES_LOOKUP[sides_byte][corners_byte];
	uint8_t move_count = AXIAL_MOVE_COUNT_LOOKUP[sides_byte][corners_byte];

	moves.move_count += move_count;
	for (int i = 0; i < move_count; i++) {
		uint8_t rpos1 = ((premade_moves[i] >> 4) & 0b0011) | ((premade_moves[i] >> 3) & 0b11000);
		uint8_t pos1 = int8_t(pos) + int8_t(rpos1) - 9;
		uint8_t rpos2 = ((premade_moves[i]) & 0b0011) | ((premade_moves[i] << 1) & 0b11000);
		uint8_t pos2 = int8_t(pos) + int8_t(rpos2) - 9;
		moves.moves[i] = (uint16_t(pos1) << 8) | uint16_t(pos2);
	}
	uint8_t axial_mask = AXIAL_MOVES_CONTROL_LOOKUP[sides_byte][corners_byte];
	control_board[(pos + 64 - 8) % 64] += (axial_mask) & 3;
	control_board[(pos + 64 - 1) % 64] += (axial_mask >> 2) & 3;
	control_board[(pos + 8) % 64] += (axial_mask >> 4) & 3;
	control_board[(pos + 1) % 64] += (axial_mask >> 6) & 3;
}

// uses bitscanning to go through diagonals
void get_diagonal_moves(bitboard board[2], uint8_t pos, uint8_t is_second_player, MoveContainer& moves, uint8_t control_board[64]) {
	uint8_t x = (pos & 7);
	uint8_t y = ((pos >> 3) & 7);
	unsigned long final_pos;
	bitboard main_diagonal = ((DIAG_MAIN_BOARD >> (8 * (x - y))) | LLC[int((x - y) < 0)])
		& ((DIAG_MAIN_BOARD << (8 * (y - x))) | LLC[int((x - y) >= 0)]);
	bitboard rvrs_diagonal = ((DIAG_RVRS_BOARD << (8 * (x + y - 7))) | LLC[int((x + y - 7) < 0)])
		& ((DIAG_RVRS_BOARD >> (8 * (7 - x - y))) | LLC[int((x + y - 7) >= 0)]);
	bitboard upper_mask = (LLT >> 8 * (8 - y)) & LLC[int(y != 0)];
	bitboard higher_upper_mask = (LLT >> 8 * (9 - y)) & LLC[int(y > 1)];
	bitboard bottom_mask = (LLT << 8 * (y + 1)) & LLC[int(y != 7)];
	bitboard lower_bottom_mask = (LLT << 8 * (y + 2)) & LLC[int(y < 6)];
	uint16_t pos16 = (uint16_t(pos) << 8);

	//UP RIGHT

	bitboard vur_positions = upper_mask & ~board[is_second_player] & main_diagonal;

	_BitScanReverse64(&final_pos, vur_positions);
	if (x - (final_pos & 7) != 1 && vur_positions != 0) {
		for (int i = pos - 9 * 2; (unsigned(i) >= final_pos) && (i > 0); i -= 9) { control_board[i] ++; }
		moves.moves[moves.move_count] = pos16 | uint16_t(final_pos);
		moves.move_count++;
	}

	//UP LEFT
	bitboard vul_positions = upper_mask & ~board[is_second_player] & rvrs_diagonal;

	_BitScanReverse64(&final_pos, vul_positions);
	if ((final_pos & 7) - x != 1 && vul_positions != 0) {
		for (int i = pos - 7 * 2; (unsigned(i) >= final_pos) && (i > 0); i -= 7) { control_board[i] ++; }
		moves.moves[moves.move_count] = pos16 | uint16_t(final_pos);
		moves.move_count++;
	}

	//DOWN LEFT
	bitboard vdl_positions = bottom_mask & ~board[is_second_player] & main_diagonal;

	_BitScanForward64(&final_pos, vdl_positions);
	if ((final_pos & 7) - x != 1 && vdl_positions != 0) {
		for (int i = pos + 9 * 2; unsigned(i) <= final_pos; i += 9) { control_board[i] ++; }
		moves.moves[moves.move_count] = pos16 | uint16_t(final_pos);
		moves.move_count++;
	}

	//DOWN RIGHT
	bitboard vdr_positions = bottom_mask & ~board[is_second_player] & rvrs_diagonal;

	_BitScanForward64(&final_pos, vdr_positions);
	if (x - (final_pos & 7) != 1 && vdr_positions != 0) {
		for (int i = pos + 7 * 2; unsigned(i) <= final_pos; i += 7) { control_board[i] ++; }
		moves.moves[moves.move_count] = pos16 | uint16_t(final_pos);
		moves.move_count++;
	}
}

// find all moves and rate them
ratedmovevector find_board_moves(bitboard board[2], uint8_t is_second_player, uint8_t control_board[64]) {
	int total_move_count = 0;
	int piece_count = count_pieces(board[is_second_player]);
	if (piece_count == 0) {
		//TODO check count pieces
		return ratedmovevector();
	}
	movecontainervector move_groups(piece_count);
	int current_piece = 0;
	int move_count = 0;
	for (int i = 0; i < 8; i++) {
		uint8_t current_byte = (board[is_second_player] >> (i * 8)) & 0xFF;
		uint8_t row_piece_count = BYTE_SET_COUNT_LOOKUP[current_byte];
		for (int j = 0; j < row_piece_count; j++) {
			unsigned long pos = 0;
			_BitScanForward(&pos, current_byte);

			MoveContainer moves;
			get_axial_moves(board, i * 8 + pos, is_second_player, moves, control_board);
			get_diagonal_moves(board, i * 8 + pos, is_second_player, moves, control_board);
			// there used to be an assert for if there were more than 5 moves available
			move_groups[current_piece] = moves;
			current_piece++;
			move_count += moves.move_count;
			if (current_piece == piece_count) {
				break;
			}
			// remove checked bit
			current_byte = current_byte & ~(-current_byte);
		}
		if (current_piece == piece_count) {
			break;
		}
	}
	ratedmovevector final_moves(move_count);
	int move_index = 0;
	for (int i = 0; i < move_groups.size(); i++) {
		for (int j = 0; j < move_groups[i].move_count; j++) {

			final_moves[move_index] = RatedMove(move_groups[i].moves[j]);
			move_index++;
		}
	}
	return final_moves;
}

void make_move(bitboard board[2], uint16_t move, uint8_t player_index) {
	board[player_index] = (board[player_index] | (LL1 << (move & 0xFF))) & ~(LL1 << ((move >> 8) & 0xFF));
	board[1 - player_index] = (board[1 - player_index] & ~(LL1 << (move & 0xFF)));
}

bool move_legal(ratedmovevector moves, uint16_t move) {
	for (int i = 0; i < moves.size(); i++) {
		if (move == moves[i].move) {
			return true;
		}
	}
	return false;
}