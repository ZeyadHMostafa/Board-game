import numpy as np

# most stuff here is depricated
# replaced with the C++ board handler which is much faster

def vec2(x,y):
	return np.array([x,y])

# constants to speed up the possible move calculations
DIAG_DIRS = ((1,1),(-1,1),(-1,-1),(1,-1))
ROTATIONAL_DIRS = (np.array([[1,0],[0,1]]),np.array([[0,1],[-1,0]]),np.array([[-1,0],[0,-1]]),np.array([[0,-1],[1,0]]))
R_DS_I = (vec2(1,1),vec2(1,0),vec2(0,1),vec2(0,2),vec2(-1,2),vec2(1,2),vec2(-1,1),vec2(1,1))
R_DS = []
for i in range(4):
	dir = ROTATIONAL_DIRS[i]
	R_DS.append([])
	for vector in R_DS_I:
		R_DS[i].append(np.matmul(dir,vector))

# R_DS=[[np.matmul(ROTATIONAL_DIRS[i],vector) for vector in R_DS_I] for i in range(4)] # test later

# pseudo moves are moves which aren't possible because there is a player on the tile
# but are useful to calculate how much control each piece has over a certain tile

def find_board_moves(board,player,pseudo = False):
	moves = []
	pseudo_moves = []
	for i in range(8):
		if board[i+8*player] ==0:continue
		for j in range(8):
			if get_player(board,player,(j,i)) == 1:
				extra_moves,extra_pseudo_moves = find_piece_moves(board,player,(j,i),pseudo)
				moves+=extra_moves
				pseudo_moves+=extra_pseudo_moves
	if pseudo:
		return moves,pseudo_moves
	else:
		return moves

def find_piece_moves(board,player,pos,pseudo = False):
	moves,pseudo_moves = find_diagonal_moves(board,player,pos,pseudo)
	moves2,pseudo_moves2 = find_rotational_moves(board,player,pos,pseudo)
	moves += moves2
	pseudo_moves += pseudo_moves2
	return moves,pseudo_moves

# diagonal moves rely on jumping over player tiles and landing on enemy tiles
def find_diagonal_moves(board,player,pos,pseudo = False):
	moves = []
	pseudo_moves = []
	for dir in DIAG_DIRS:
		doable = False
		final_pos = pos
		while True:
			final_pos = add_vec(dir,final_pos)
			x = get_piece(board,player,final_pos)
			if x == -1:
				doable = False
				break
			elif x == 0:
				break
			else:
				if pseudo and doable:
					pseudo_moves.append(bytearray([pos_to_byte(pos),pos_to_byte(final_pos)]))
				doable = True
		if doable:
			moves.append(bytearray([pos_to_byte(pos),pos_to_byte(final_pos)]))
	return moves,pseudo_moves


# rotational moves rely on turning around player tles and landing on orthogonal tiles
# a piece can move a maximum of 3 steps from its starting position using this method
def find_rotational_moves(board,player,pos,pseudo = False):
	moves = []
	pseudo_moves = []
	np_pos = np.array(pos)
	positions = [[-1,-1,-1],[-1,-1,-1],[-1,-1,-1]]
	#first step
	for i in range(4):
		# b c
		# x a
		# check for rotation around a or b to reach c
		check_vec = vec2(1,1)+R_DS[i][0]
		c = get_player(board,player,np_pos+R_DS[i][0])
		if (c == 1 and not pseudo) or c == None:
			continue
		a = get_player(board,player,np_pos+R_DS[i][1])
		b = get_player(board,player,np_pos+R_DS[i][2])
		if a == 1 and b == 0 or a == 0 and b == 1:
			if c == -1:
				positions[check_vec[0]][check_vec[1]] = 1
			elif c == 0:
				positions[check_vec[0]][check_vec[1]] = 0
			elif c==1 and pseudo:
				positions[check_vec[0]][check_vec[1]] = 2

	#second step
	for i in range(4):
		# c a c
		# d b d
		# - x -
		# check for rotation around b to reach a
		check_vec = vec2(1,1)+R_DS[i][2]
		a = get_player(board,player,np_pos+R_DS[i][3])
		if (a == 1 and not pseudo) or a == None:
			continue
		b = get_player(board,player,np_pos+R_DS[i][2])
		if b != 1:
			continue
		for k in (1,0):
			c = get_player(board,player,np_pos+R_DS[i][4+k])
			if c != 0:
				continue
			d1 = R_DS[i][6+k]
			d = positions[d1[0]+1][d1[1]+1]
			if d==0:
				if a == -1:
					positions[check_vec[0]][check_vec[1]] = 1
				elif a == 0:
					positions[check_vec[0]][check_vec[1]] = 0
				elif a==1 and pseudo:
					positions[check_vec[0]][check_vec[1]] = 2
				break
		#third step
		if positions[check_vec[0]][check_vec[1]] == 0:
			for k in (1,0):
				check_vec = vec2(1,1)+R_DS[i][6+k]
				c = get_player(board,player,np_pos+R_DS[i][4+k])
				if c != 0:
					continue
				d = get_player(board,player,np_pos+R_DS[i][6+k])

				if d == -1:
					positions[check_vec[0]][check_vec[1]] = 1
				elif d == 0:
					positions[check_vec[0]][check_vec[1]] = 0
				elif d == 1 and pseudo:
					positions[check_vec[0]][check_vec[1]] = 2
		
	for dir in DIAG_DIRS:
		move = pos_to_move(np_pos,np_pos+dir)
		if positions[dir[0]+1][dir[1]+1] in (0,1):
			moves.append(move)
		elif positions[dir[0]+1][dir[1]+1] == 2:
			pseudo_moves.append(move)
	for dir in (vec2(0,1),vec2(0,-1),vec2(1,0),vec2(-1,0)):
		move = pos_to_move(np_pos,np_pos+dir*2)
		if positions[dir[0]+1][dir[1]+1] in (0,1):
			moves.append(move)
		elif positions[dir[0]+1][dir[1]+1] == 2:
			pseudo_moves.append(move)
	return moves,pseudo_moves

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