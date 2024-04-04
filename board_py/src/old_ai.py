import board_handler
import numpy as np
import math
import time
import multiprocessing
from random import choices

vec2 = board_handler.vec2

# WARNING
# this module is depricated
# board_cpp handles this much faster

def rate_board(board,player):
	pass

# used for terminal decision making
def approximate_board_score(board,player):
	score = 0
	# +2.9
	player_moves,player_pseudo_moves = board_handler.find_board_moves(board,player,True)
	opponent_moves,opponent_pseudo_moves = board_handler.find_board_moves(board,1-player,True)
	# + 0.0 seconds
	player_control_board = generate_control_board(player_moves+player_pseudo_moves)
	opponent_control_board = generate_control_board(opponent_moves+opponent_pseudo_moves)
	p_1,p_2,p_3,p_4,p_5,p_6 = get_control_counts(board,player,player_control_board,opponent_control_board,player_moves)
	o_1,o_2,o_3,o_4,o_5,o_6 = get_control_counts(board,player,opponent_control_board,player_control_board,opponent_moves)
	# +0.3 seconds
	p_p_control = board_handler.array_by_board(board,player_control_board,0)
	p_o_control = board_handler.array_by_board(board,player_control_board,1)
	p_e_control = np.sum(player_control_board)-p_p_control-p_o_control
	o_o_control = board_handler.array_by_board(board,opponent_control_board,1)
	o_p_control = board_handler.array_by_board(board,opponent_control_board,0)
	o_e_control = np.sum(player_control_board)-o_o_control-o_p_control

	score += rate_board_union(board,player) # + 0.9 second
	score += p_p_control*0.2
	score += p_o_control*0.4
	score += p_e_control*0.05
	score += p_1*1*2
	score += p_2*0.9*2
	score += p_3*0.5*2
	score += p_4*0.3*2
	score += p_5*0.2*2
	score += p_6*0.1*2

	score -= o_o_control*0.2
	score -= o_p_control*0.4
	score -= o_e_control*0.05
	score -= o_1*1*2
	score -= o_2*0.9*2
	score -= o_3*0.5*2
	score -= o_4*2*2
	score -= o_5*1.5*2
	score -= o_6*0.5*2

	if o_1+o_2+o_3+o_4+o_5+o_6 == 0:
		score+=100
	return score

def generate_control_board(moves):
	control_board = np.zeros((8,8))
	for move in moves:
		pos = board_handler.byte_to_pos(move[1])
		control_board[pos[0]][pos[1]] += 1
	return control_board

# useful parameters
def get_control_counts(board,player,player_control_board,opponent_control_board,moves):
	total_control = player_control_board-opponent_control_board
	
	positive_move_count = 0
	equal_move_count = 0
	negative_move_count = 0
	positive_trade_count = 0
	equal_trade_count = 0
	negative_trade_count = 0

	for move in moves:
		pos = board_handler.byte_to_pos(move[1])
		trade = board_handler.get_piece(board,player,pos) == 1
		tile_control = total_control[pos[0]][pos[1]]
		if trade:
			if tile_control > 0:
				positive_trade_count += 1
			elif tile_control == 0:
				equal_trade_count += 1
			else:
				negative_trade_count += 1
		else:
			if tile_control > 1:
				positive_move_count += 1
			elif tile_control == 1:
				equal_move_count += 1
			else:
				negative_move_count += 1

	return (positive_move_count, equal_move_count, negative_move_count, positive_trade_count, equal_trade_count, negative_trade_count)

def rate_board_union(board,player):
	score = get_board_union_score(board,player)-get_board_union_score(board,1-player)
	return score

def get_board_union_score(board,player):
	def get_piece_union_score(board,player,pos):
		score = 0
		for i in (-1,0,1):
			for j in (-1,0,1):
				if i == 0 and j == 0:continue
				if board_handler.get_player(board,player,vec2(pos[0],pos[1])+vec2(j,i)) == 1:
					score+=1
		return score
	score = 0
	for i in range(8):
		for j in range(8):
			if board_handler.get_player(board,player,(j,i)) == 1:
				score+=get_piece_union_score(board,player,(j,i))
	return score

def rate_move(board,player,move):
		return approximate_board_score(board_handler.move_piece(board,player,move),player)

# TODO: fix scoring probability selection (takes too much time)
# depricated anyways, so not a priority. use c_score_moves instead
def threaded_score_moves(board,player,moves,relative = True, difficulty = 2,depth = 1,thread_data_list = [],thread_index = 0):
	if relative:
		st = time.time()
	if moves is None:
		moves = board_handler.find_board_moves(board,player)

	if depth > 0 :
		scored_moves = []
		tertiary_scored_moves = []
		args = []
		for move in moves:
			new_board = board_handler.move_piece(board,player,move)
			args.append((new_board,1-player,None,False,difficulty,depth-1))
		
		with multiprocessing.Pool(processes=8) as pool:
			tertiary_scored_moves = pool.starmap(score_moves, args)

		for i,move in enumerate(moves):
			secondary_scored_moves = tertiary_scored_moves[i]
			scores = [x[1] for x in secondary_scored_moves]
			if len(scores) == 0:
				scored_moves.append([move,100])
				continue
			base = min(scores)
			top = max(scores)-base
			extra = 0
			if top == 0: top,extra = 1,1
			score_probabilities = [pow(((x-base)/top+extra),1) for x in scores]
			scored_moves.append([move,-sum(np.multiply(scores,score_probabilities))/sum(score_probabilities)])

		if relative:
			scores = [x[1] for x in scored_moves]
			base = min(scores)
			top = max(scores)-base
			extra = 0
			if top == 0: top,extra = 1,1
			scored_moves = [(x[0],pow(((x[1]-base)/top+extra),difficulty)) for x in scored_moves]
	else:
		scored_moves = [(x,rate_move(board,player,x)) for x in moves]
	if relative:
		print(f"time spent sorting all moves = {time.time()-st}")
	if not thread_data_list:
		return scored_moves
	else:
		thread_data_list[thread_index] = scored_moves

def score_moves(board,player,moves,relative = True, difficulty = 2,depth = 1):
	if relative:
		st = time.time()
	if moves is None:
		moves = board_handler.find_board_moves(board,player)

	if depth > 0 :
		scored_moves = []
		for move in moves:
			new_board = board_handler.move_piece(board,player,move)
			secondary_scored_moves = score_moves(new_board,1-player,None,False,difficulty,depth-1)
			scores = [x[1] for x in secondary_scored_moves]
			if len(scores) == 0:
				scored_moves.append([move,100])
				continue
			base = min(scores)
			top = max(scores)-base
			extra = 0
			if top == 0: top,extra = 1,1
			score_probabilities = [pow(((x-base)/top+extra),1) for x in scores]
			scored_moves.append([move,-sum(np.multiply(scores,score_probabilities))/sum(score_probabilities)])

		if relative:
			scores = [x[1] for x in scored_moves]
			base = min(scores)
			top = max(scores)-base
			extra = 0
			if top == 0: top,extra = 1,1
			scored_moves = [(x[0],pow(((x[1]-base)/top+extra),difficulty)) for x in scored_moves]
	else:
		scored_moves = [(x,rate_move(board,player,x)) for x in moves]
	if relative:
		print(f"time spent sorting all moves = {time.time()-st}")
	return scored_moves

def sort_moves(scored_moves):
	return sorted(scored_moves, key=lambda x: x[1])

def choose_move(scored_moves,determination=5):
	moves = [x[0] for x in scored_moves]
	scores = [math.e**(determination*x[1]) for x in scored_moves]
	choice = choices(population=moves,weights=scores,k=1)[0]
	idx = moves.index(choice)
	print(f"chose move number {len(moves)-idx} from the top")
	print(f"move score = {scored_moves[idx][1]*10:.2f}/{scored_moves[-1][1]*10:.2f}")
	return choice
