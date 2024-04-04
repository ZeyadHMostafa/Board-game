import numpy as np
import board_cpp_wrapper

find_board_moves = board_cpp_wrapper.find_board_moves

def vec2(x,y):
	return np.array([x,y])

# utility functions
def move_piece(board,player,move):
	new_board = board.copy()
	pos1,pos2 = [byte_to_pos(x) for x in move]
	set_piece(new_board,player,pos1,0)
	set_piece(new_board,player,pos2,1)
	set_piece(new_board,1-player,pos2,0)
	return new_board

def print_board(board):
	for i in range(8):
		for j in range(8):
			print(get_player(board,0,(j,i)),end="")
		print("")

def get_player(board,player,pos):
	if (pos[0] >= 8) or (pos[0] < 0) or (pos[1] >= 8) or (pos[1] < 0): return None
	if get_piece(board,player,pos) == 1:
		return 1
	if get_piece(board,1-player,pos) == 1:
		return -1
	return 0

def pos_to_byte(pos):
	return np.byte((pos[0]&7)+((pos[1]&7)<<3))

def pos_to_move(pos_from,pos_to):
	return bytearray([pos_to_byte(pos_from),pos_to_byte(pos_to)])
	
def byte_to_pos(val):
	return(val&7,(val&56)>>3)

def add_vec(vec1,vec2):
	return (vec1[0]+vec2[0],vec1[1]+vec2[1])

def get_piece(board,player,pos):
	if (pos[0] >= 8) or (pos[0] < 0) or (pos[1] >= 8) or (pos[1] < 0): return -1
	return int((board[pos[1]+8*player] & (128>>pos[0]) != 0))

def set_piece(board,player,pos,value):
	mask = 255 if value else 0
	board[pos[1]+8*player] ^= ((board[pos[1]+8*player]^mask) & ((128>>pos[0])))

def array_by_board(board,array,player):
	total = 0
	for j in range(8):
		for i in range(8):
			total += array[i][j]*get_piece(board,player,(i,j))
	
	return total