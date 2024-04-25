#include "main.h"


void bytes_to_board(uint8_t bytes[16], bitboard board[2]) {
	std::memcpy(&board[0], bytes, sizeof(bitboard));
	std::memcpy(&board[1], bytes + 8, sizeof(bitboard));
}

int count_pieces(uint8_t board[16], uint8_t is_second_player) {
	int count = 0;
	for (int i = 0; i < 8; i++) {
		count += BYTE_SET_COUNT_LOOKUP[board[i + 8 * is_second_player]];
	}
	return count;
}

int count_pieces(bitboard half_board) {
	int count = 0;
	uint8_t* board_bytes = reinterpret_cast<uint8_t*>(&half_board);
	for (int i = 0; i < 8; i++) {
		count += BYTE_SET_COUNT_LOOKUP[(*(board_bytes + i))];
	}
	return count;
}

uint8_t get_piece_at_pos(uint8_t board[16], uint8_t x, uint8_t y, uint8_t is_second_player) {
	if (x < 0 || x > 7 || y < 0 || y > 7) {
		return 2;
	}
	if ((board[y + 8 * is_second_player] & (1 << x)) != 0) {
		return 1;
	}
	return 0;
}
uint8_t get_piece_from_byte(uint8_t board[16], uint8_t pos, uint8_t is_second_player) {
	if ((board[(pos >> 3) + 8 * is_second_player] & (1 << (pos & 7))) != 0) {
		return 1;
	}
	return 0;
}

uint8_t get_circular_byte(bitboard half_board, uint8_t x, uint8_t y) {
	//(ul)(u)(ur)(l)(r)(dl)(d)(dr)
	//cscsscsc
	uint8_t circular_byte = 0;
	uint8_t* board_bytes = reinterpret_cast<uint8_t*>(&half_board);
	//L-R
	circular_byte |= ((((*(board_bytes + y)) >> (x - 1)) << 3) & 0b00001000);
	circular_byte |= ((((*(board_bytes + y)) >> (x + 1)) << 4) & 0b00010000);
	//U
	circular_byte |= (((*(board_bytes + y - 1))) >> 1) & 0b11100000 & LLC[int(x == 7)];
	circular_byte |= (((*(board_bytes + y - 1))) << (6 - x)) & 0b11100000 & LLC[int(x != 7)];
	//D
	circular_byte |= ((*(board_bytes + y + 1)) << 1) & 0b00000111 & LLC[int(x == 0)];
	circular_byte |= ((*(board_bytes + y + 1)) >> (x - 1)) & 0b00000111 & LLC[int(x != 0)];

	return circular_byte;
}

uint8_t get_sides_from_circular_bytes(uint8_t b1, uint8_t b2) {
	// rudlrudl
	return (b1 & 0b1010) | ((b1 >> 4) & 0b0101) | (b2 & 0b01010000) | ((b2 << 4) & 0b10100000);
}
uint8_t get_corners_from_circular_bytes(uint8_t b1, uint8_t b2) {
	//(ul)(dl)(ur)(dr)
	return (b1 & 0b0101) | ((b1 >> 4) & 0b1010) | (b2 & 0b0101) | ((b2 >> 4) & 0b1010);
}

bool boards_equal(bitboard board1[2], bitboard board2[2]) {
	return (board1[0] == board2[0]) && (board1[1] == board2[1]);
}