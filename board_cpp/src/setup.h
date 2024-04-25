#pragma once

#include "main.h"

uint8_t generate_byte_set_count(uint8_t byte);

uint8_t* generate_axial_moves(uint8_t sides_byte, uint8_t corners_byte);

extern "C" __declspec(dllexport)
void initialize();