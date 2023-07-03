# Set train to True to find suitable weights for pre-calculated data
# Otherwise the interactive game will start
train = False
training_file_name = 'data/data'

if __name__ == "__main__":
	import interactive
	import AI_trainer
	if train:
		AI_trainer.find_best_fit_line(training_file_name)
	else:
		game = interactive.Game()

# Controls:
# LMB	:Move a piece
# F2	:Toggle Assist
# F3	:Toggle control overlay
# Q		:Toggle player 1 AI
# A		:Toggle player 2 AI
# S		:Play AI move
# R		:Reset the game
