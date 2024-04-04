#include "setup.h"
/*
	Setup file containing all precalculations for the board game
	These calcuations speed up the process significantly
*/

uint8_t BYTE_SET_COUNT_LOOKUP[256];
uint8_t* ROW_PIECE_COLS_LOOKUP[256];
uint8_t max_move_count;
uint8_t* AXIAL_MOVES_LOOKUP[256][16];
uint8_t AXIAL_MOVE_COUNT_LOOKUP[256][16];
uint8_t AXIAL_MOVES_CONTROL_LOOKUP[256][16];

// used to find how many bits there are per byte
// useful to count the amount of pieces on a bitboard
uint8_t generate_byte_set_count(uint8_t byte) {
	static const uint8_t NIBBLE_LOOKUP[16] =
	{
	  0, 1, 1, 2, 1, 2, 2, 3,
	  1, 2, 2, 3, 2, 3, 3, 4
	};

	return NIBBLE_LOOKUP[byte & 0x0F] + NIBBLE_LOOKUP[byte >> 4];
}

// used to find all possible axial moves for a given 3x3 configuration quickly
// the logic behind the byte manipulation is complex for humans but fast for processors
uint8_t* generate_axial_moves(uint8_t sides_byte, uint8_t corners_byte) {
	int index1 = 0;
	int arrindex = 0;
	// index1: moveable piece index
	// index2: moveable place index
	uint8_t* moves = new uint8_t[4];
	while (index1 < 4) {
		if ((CIRCULAR_MASK_LOOKUP[index1] & sides_byte) != 0) {
			int index2 = (index1 + 1) % 4;
			int piece_moves = 0;
			bool real_moves_done = false;
			uint8_t mini_control_mask = 0;
			while ((
				(CIRCULAR_MASK_LOOKUP[(index2 + 3) % 4 + 4] & (corners_byte))) == 0 && piece_moves < 3 &&
				index2 != index1) {
				mini_control_mask |= 1 << index2;
				if ((CIRCULAR_MASK_LOOKUP[index2] & sides_byte) == 0 && !real_moves_done) {
					moves[arrindex] = (DIRECTIONAL_POSITIONS[index1] << 4) | DIRECTIONAL_POSITIONS[index2];
					arrindex++;
					piece_moves++;
					if ((CIRCULAR_MASK_LOOKUP[index2] & (sides_byte >> 4)) != 0) {
						break;
					}
				}
				else {
					real_moves_done = true;
				}
				index2 = (index2 + 1) % 4;

			}
			index2 = (index1 + 3) % 4;
			real_moves_done = false;
			while ((
				(CIRCULAR_MASK_LOOKUP[index2 + 4] & (corners_byte))) == 0 && piece_moves < 3 &&
				index2 != index1) {
				mini_control_mask |= 1 << index2;
				if ((CIRCULAR_MASK_LOOKUP[index2] & sides_byte) == 0 && !real_moves_done) {
					moves[arrindex] = (DIRECTIONAL_POSITIONS[index1] << 4) | DIRECTIONAL_POSITIONS[index2];
					arrindex++;
					piece_moves++;
					if ((CIRCULAR_MASK_LOOKUP[index2] & (sides_byte >> 4)) != 0) {
						break;
					}
				}
				else {
					real_moves_done = true;
				}
				index2 = (index2 + 3) % 4;

			}
			for (int i = 0; i < 4; i++) {
				uint8_t AMCL = AXIAL_MOVES_CONTROL_LOOKUP[sides_byte][corners_byte];
				AXIAL_MOVES_CONTROL_LOOKUP[sides_byte][corners_byte] &= ~(3 << (i * 2));
				AXIAL_MOVES_CONTROL_LOOKUP[sides_byte][corners_byte] |=
					(((AMCL >> i * 2) & 3) + int(((mini_control_mask >> i) & 1) == 1)) << i * 2;
			}

		}
		index1 += 1;
	}
	AXIAL_MOVES_LOOKUP[sides_byte][corners_byte] = moves;
	AXIAL_MOVE_COUNT_LOOKUP[sides_byte][corners_byte] = arrindex;
	return moves;

}

void initialize() {
	srand(time(0));
	//initialize BYTE_SET_COUNT_LOOKUP
	for (int i = 0; i < 256; i++) {
		BYTE_SET_COUNT_LOOKUP[i] = generate_byte_set_count(i);
	}
	BYTE_SET_COUNT_LOOKUP[255] = 8;
	for (int i = 0; i < 4096; i++) {
		generate_axial_moves(i & 0xFF, i >> 8);
	}

}