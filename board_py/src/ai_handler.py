import board_cpp_wrapper
import math
from random import choices

def choose_move(scored_moves,determination=5):
	moves = [x[0] for x in scored_moves]
	scores = [math.e**(determination*x[1]) for x in scored_moves]
	choice = choices(population=moves,weights=scores,k=1)[0]
	idx = moves.index(choice)
	print(f"chose move number {len(moves)-idx} from the top")
	print(f"move score = {scored_moves[idx][1]*10:.2f}/{scored_moves[-1][1]*10:.2f}")
	return choice

find_rated_moves=board_cpp_wrapper.find_rated_moves