import ctypes as c
import pathlib
import board_handler

#relative_path = "../../board_cpp/build/board_cpp.dll"
relative_path = "board_cpp/build/board_cpp.dll"

libname = pathlib.Path().absolute() / relative_path

class RatedMove(c.Structure):
    _fields_ = [
        ('move',c.c_uint16),
        ('rating',c.c_float),
        ]

# should probably use this class more often
class PyRatedMove():
	def __init__(self,rated_move:RatedMove) -> None:
		self.move = rated_move.move
		self.rating = rated_move.rating
	
	def get_py_move(self):
		#x values are reversed compared to python version
		x2 = 7-int((self.move) & 7)
		y2 = int((self.move >> 3) & 7)
		x1 = 7-int((self.move >> 8) & 7)
		y1 = int((self.move >> 11) & 7)
		pos1 = board_handler.vec2(x1,y1)
		pos2 = board_handler.vec2(x2,y2)
		return board_handler.pos_to_move(pos1,pos2)

board_lib = c.CDLL(str(libname))


# VERY IMPORTANT !!!!!!!!!!!
board_lib.initialize.argtypes=()
board_lib.initialize.restype=None
board_lib.initialize()


board_lib.python_find_rated_board_moves.argtypes=(
	c.c_uint64*2,RatedMove*80,c.c_ubyte*64,c.c_ubyte*64,c.c_ubyte,c.POINTER(c.c_float),c.c_ubyte,c.c_float,c.c_float)
board_lib.python_find_rated_board_moves.restype=c.c_ubyte

def find_rated_moves(board,player_index:bool,depth=0,cutoff_percentage=0,cutoff_fade=1,should_sort=True,
					 control_board1=[0]*64,control_board2=[0]*64):
	cboard = (c.c_uint64*2).from_buffer(board)
	cplayer_index = (c.c_ubyte)(player_index)

	ccontrol_board1 = (c.c_ubyte*64)()
	ccontrol_board2 = (c.c_ubyte*64)()
	rated_moves = (RatedMove*80)()
	board_rating = (c.c_float)()
	cdepth = (c.c_ubyte)(depth)
	ccutoff_percentage = (c.c_float)(cutoff_percentage)
	ccutoff_fade = (c.c_float)(cutoff_fade)

	move_count = board_lib.python_find_rated_board_moves(
	cboard,rated_moves,ccontrol_board1,ccontrol_board2,cplayer_index,c.pointer(board_rating),cdepth,ccutoff_percentage,ccutoff_fade)
	
	if move_count==0:return[]

	rated_moves = rated_moves = [PyRatedMove(move) for move in rated_moves[:move_count]]

	# if not just checking the moves
	if depth != 255:
		for i,(c1,c2) in enumerate(zip(ccontrol_board1,ccontrol_board2)):
			control_board1[i] = int(c1)
			control_board2[i] = int(c2)

		# quick fix, should be removed later to work with regular convention
		for move in rated_moves:
			move.rating/=10
		
		base_move_rating = min(rated_moves,key=lambda x:x.rating).rating

		# normalize to avoid negative numbers
		for move in rated_moves:
			move.rating -= base_move_rating
		
		# sorting
		if should_sort:
			rated_moves = sorted(rated_moves,key=lambda x: x.rating)
	
	return [(move.get_py_move(),move.rating) for move in rated_moves]

def find_board_moves(board,player_index:bool):
	return find_rated_moves(board,player_index,depth=255)
	

if __name__ == "__main__":
	MAIN_BOARD_CONFIGURATION = bytearray([
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
		])
	rated_moves = find_rated_moves(MAIN_BOARD_CONFIGURATION,False)
	for move in rated_moves:
		print(f"{(move[0])} : {move[1]:.2f}")