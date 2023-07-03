#pragma once
/*
	This contains many lookup tables and values that
	speed up computation for the game analysis
*/
#include <iostream>
#include <bitset>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>

#endif
#define bitboard unsigned long long
#define movecontainervector std::vector<MoveContainer>
#define ratedmovevector std::vector<RatedMove>

extern uint8_t BYTE_SET_COUNT_LOOKUP[256];
extern uint8_t* ROW_PIECE_COLS_LOOKUP[256];
extern uint8_t max_move_count;
static const bitboard BYTE_END = (bitboard)1 << 63;
static const bitboard LL1 = (bitboard)1;
static const bitboard LLF = (bitboard)0;
static const bitboard LLT = (bitboard)-1;
static const bitboard LLC[2] = { LLF , LLT };

static const float ORIGINAL_MODEL_WEIGHTS[25] = {
		12.0, 10.0,1.0,
		0.2, 0.0, -0.2,
		-3.0, -7,-8.0,

		0.1,0.5,     1.0,2.0,
		0.01,0.25,   0.5,1.5,

		-0.1,-1.0,   -1.0,-4.0,
		-0.01,-0.5,  -0.5,-3.0,
};

static const float MODEL_WEIGHTS1[25] = {
		12.5, 11.5,2.75,
		0.75, 0.0, -0.25,
		-2.5, -7.5,-9.0,

		0.01,4.0,   2.4,4.9,
		0.01,2.5,   0.4,1.7,

		-0.01,-2.3,   -0.3,-4.0,
		-0.01,-4.7,  -0.5,-3.0,
};

static const float MODEL_WEIGHTS[25] = {
		6.3, 6.3,2.75,
		0.3, 0.0, -3.5,
		-1.8, -4.4,-5.3,

		0.01,2.9,   0.95,1.9,
		0.4,2.0,   0.3,0.85,

		-0.01,-1.1,   -0.01,-0.63,
		-0.01,-2.3,  -0.4,-1.8,
};

static struct MoveContainer {
	uint8_t move_count = 0;
	uint16_t moves[5] = {};
	MoveContainer(uint16_t* _moves, uint8_t _move_count) {
		std::memcpy(&moves, _moves, 5 * sizeof(uint16_t));
		move_count = _move_count;
	}
	MoveContainer() {
		move_count = 0;
	}
};

static struct RatedMoveIndex {
	uint8_t index = 0;
	float rating = 0.0;
};

static struct RatedMove {
	uint16_t move = 0;
	float rating = 0;
	RatedMove() {}
	RatedMove(uint16_t _move) {
		move = _move;
	}
	RatedMove(uint16_t _move, float _rating) {
		rating = _rating;
	}
};

extern uint8_t* AXIAL_MOVES_LOOKUP[256][16];
extern uint8_t AXIAL_MOVE_COUNT_LOOKUP[256][16];
extern uint8_t AXIAL_MOVES_CONTROL_LOOKUP[256][16];

//up << 8
//down >> 8
//right << 1
//left >> 1

//(ul)(dl)(ur)(dr)
static const uint8_t AXIAL_MASKS[8] = {
	0,
	0b1100,
	0,
	0b0011,
	0,
	0b1010,
	0,
	0b0101,
};

static const bitboard DIAG_MAIN_BOARD = 0x8040201008040201;
static const bitboard DIAG_RVRS_BOARD = 0x0102040810204080;


// urdl(ur)(dr)(dl)(ul)
static const uint8_t DIRECTIONAL_POSITIONS[8] = {
	0b0001,
	0b0100,
	0b1001,
	0b0110,

	0b0000,
	0b1000,
	0b1010,
	0b0010
};

// urdl(ur)(dr)(dl)(ul)
const uint8_t CIRCULAR_MASK_LOOKUP[8] = {
	0b0100,
	0b1000,
	0b0010,
	0b0001,
	0b0010,
	0b0001,
	0b0100,
	0b1000
};


/*

example board starting configurations
{
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b11111111,
0b11111111,

0b11111111,
0b11111111,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
}
{
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
}
;*/