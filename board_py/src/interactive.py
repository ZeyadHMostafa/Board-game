import pygame
import math
import threading
import numpy as np
from pygame import Vector2, Rect

import board_handler
import AI

# Interactive Constants
FPS_AVERAGING_TIME = 1000
FPS = 60
TILE_SIZE = 64
DISPLAY_W = TILE_SIZE*17
DISPLAY_H = TILE_SIZE*8.5
MOVE_SPEED = 0.1
PLAYER_COLORS = (((170,170,170),(40,40,50)), ((40,40,50),(170,170,170)))
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

# game class that handles the simulation
# having a class allows us to play multiple games at the same time
class Game:
	# initialization functions
	def __init__(self,board_file = None) -> None:
		if board_file:
			self.load_board(board_file)
		else:
			self.board = MAIN_BOARD_CONFIGURATION
		self.initialize_game()

	def initialize_game(self):
		self.game_ended = False

		self.initialize_display()
		self.initialize_clock()

		self.locked = False
		self.auto = [False,False]
		self.step_once = False
		self.current_move = 0
		self.current_player = 0
		self.move_time = 0
		self.piece_selected = None

		# Threads to speed up gameplay
		self.AI_thread1 = threading.Thread(target=self.threaded_AI_move, args = (),daemon=True)
		self.AI_thread2 = threading.Thread(target=self.threaded_AI_display, args = (),daemon=True)

		## GAME LOOP
		while self.running:
			self.advance_clock()
			for event in pygame.event.get():
				handle_event(self,event)
			self.tick()
			self.display()

	def initialize_clock(self):
		self.clock = pygame.time.Clock()
		self.paused = False
		self.dt = 0
		self.game_time = 0
		self.av_fps = 0
		self.fps_counter = FPS_AVERAGING_TIME
		self.frame_counter = 0

	def initialize_display(self):
		pygame.init()
		self.FONT1 = pygame.font.SysFont(None, 32)
		self.FONT2 = pygame.font.SysFont(None, 24)
		self.game_display = pygame.display.set_mode((DISPLAY_W,DISPLAY_H))
		pygame.display.set_caption('board game')
		self.backgrund_image = self.generate_board_background()
		self.show_assist = False
		self.show_control = False
		self.update_board= True
		self.update_assist = True
		self.update_control_boards = True
		self.running = True

	# main functions

	def tick(self):
		self.move_time += self.dt
		if not self.locked and not self.game_ended and ((self.move_time/1000.0) > MOVE_SPEED and self.auto[self.current_player] or self.step_once):
			self.step_once = False
			self.move_time = 0
			self.current_move += 1
			moves = board_handler.find_board_moves(self.board,self.current_player)
			if len(moves) == 0:
				print("black" if self.current_player == 0 else "white"," wins")
				self.game_ended = True
			else:
				self.play_AI_move(moves)

	def advance_clock(self):
		self.dt = self.clock.tick(60)
		self.game_time += self.dt*int(not self.paused)
		self.fps_counter += self.dt
		self.frame_counter +=1
		if self.fps_counter >= FPS_AVERAGING_TIME:
			self.av_fps = self.frame_counter/self.fps_counter
			self.fps_counter = 0
			self.frame_counter = 0
	
	def display(self):
		self.game_display.fill((50,50,70))
		self.game_display.blit(self.backgrund_image,Vector2(0,0))
		if self.update_board:
			self.board_image = self.image_from_board(self.board)
			self.update_board = False
		self.game_display.blit(self.board_image,Vector2(0,0))
		if self.show_assist:
			if self.update_assist:
				if not self.AI_thread2.is_alive():
					self.AI_thread2 = threading.Thread(target=self.threaded_AI_display, args = (),daemon=True)
					self.AI_thread2.start()
			if not self.update_assist:
				self.game_display.blit(self.assist_image,Vector2(0,0))
			#extra
		if self.show_control:
			if self.update_control_boards:
				self.control_board_image = self.generate_control_board_image()
				self.update_control_boards = False
			self.game_display.blit(self.control_board_image,Vector2(0,0))
		self.display_playing_overlay()
		pygame.display.update()

	# generate display layers
	def generate_board_background(self):
		image = pygame.Surface((TILE_SIZE*8.5,TILE_SIZE*8.5))
		image.fill((30,30,40))
		image.fill((50,60,70),((0,0),(TILE_SIZE*8,TILE_SIZE*8)))
		for i in range(8*4):
			row = i%4
			col = math.floor(i/4)
			image.fill((20,20,30),Rect(row*TILE_SIZE*2+TILE_SIZE*(col%2),col*TILE_SIZE,TILE_SIZE,TILE_SIZE))
		for i in range(8):
			text_image = self.FONT1.render("ABCDEFGH"[i], True, (255,255,255))
			image.blit(text_image,(TILE_SIZE*(i+0.5)-text_image.get_rect().right*0.5,TILE_SIZE*8.25-text_image.get_rect().bottom*0.5))
		for i in range(8):
			text_image = self.FONT1.render(str(8-i), True, (255,255,255))
			image.blit(text_image,(TILE_SIZE*8.25-text_image.get_rect().right*0.5,TILE_SIZE*(i+0.5)-text_image.get_rect().bottom*0.5))
		return image

	## display pieces on the board
	def image_from_board(self,board):
		image = pygame.Surface((TILE_SIZE*8,TILE_SIZE*8),pygame.SRCALPHA, 32)
		for k in range(2):
			for i in range(8):
				for j in range(8):
					if board[i+8*k] & (128>>j):
						pygame.draw.circle(image,PLAYER_COLORS[k][1],((j+0.5)*TILE_SIZE,(i+0.5)*TILE_SIZE),TILE_SIZE/2-7)
						pygame.draw.circle(image,PLAYER_COLORS[k][0],((j+0.5)*TILE_SIZE,(i+0.5)*TILE_SIZE),TILE_SIZE/2-7-2)
		return image	
	
	## display moves on the board
	def image_from_moves(self,board,player,moves = None, move_count = 5):
		image = pygame.Surface((DISPLAY_W,DISPLAY_H),pygame.SRCALPHA, 32)
		if not moves:
			moves = board_handler.find_board_moves(board,player)
		if len(moves) == 0 :
			return image
		sorted_moves = AI.sort_moves(AI.threaded_score_moves(board,player,moves,True,1))
		moves_to_display = sorted_moves[-min(move_count,len(moves)-1):]
		moves_to_display.reverse()
		for i in range(len(moves_to_display)):
			move = moves_to_display[i]
			amount = (1-i/(move_count-1))*0.8+0.2#move[1]/moves_to_display[move_count-1][1]
			start_tile = Vector2(board_handler.byte_to_pos(move[0][0]))
			end_tile = Vector2(board_handler.byte_to_pos(move[0][1]))
			start_pos = (start_tile+Vector2(0.5,0.5))*TILE_SIZE
			end_pos = (end_tile+Vector2(0.5,0.5))*TILE_SIZE
			color= (120*amount,130*amount,150*amount)
			pygame.draw.line(image,(255,255,255),start_pos,end_pos,11)
			pygame.draw.line(image,color,start_pos,end_pos,7)
			pygame.draw.circle(image,(255,255,255),start_pos,10)
			pygame.draw.circle(image,(255,255,255),end_pos,10)
			pygame.draw.circle(image,color,start_pos,7)
			pygame.draw.circle(image,color,end_pos,7)
			explanation_image = self.FONT2.render(str(i+1)+" : "+str(round(move[1]*10,1)), True, (255,255,255))
			image.fill(color,((TILE_SIZE*8.5,i*0.5*TILE_SIZE),(TILE_SIZE*8.5,0.5*TILE_SIZE)))
			image.blit(explanation_image, (TILE_SIZE*9,(i*0.5+0.25)*TILE_SIZE-explanation_image.get_rect().height*0.5))
			pygame
		return image	

	## show how controlled each tile is
	def generate_control_board_image(self):
		image = pygame.Surface((TILE_SIZE*8,TILE_SIZE*8),pygame.SRCALPHA, 32)
		p_moves,p_pseudo_moves = board_handler.find_board_moves(self.board,self.current_player,True)
		o_moves,o_pseudo_moves = board_handler.find_board_moves(self.board,1-self.current_player,True)
		self.control_boards = [AI.generate_control_board(p_moves+p_pseudo_moves),AI.generate_control_board(o_moves+o_pseudo_moves)]
		for i in range(8):
				for j in range(8):
					total_control = self.control_boards[self.current_player][j][i] - self.control_boards[1-self.current_player][j][i]
					text_image = self.FONT2.render(str(total_control), True, (255,255,255),(0,0,0))
					image.blit(text_image,(TILE_SIZE*j,TILE_SIZE*i))
		return image

	# show currently held piece
	def display_playing_overlay(self):
		if not self.piece_selected:return
		image = pygame.Surface((TILE_SIZE*8,TILE_SIZE*8),pygame.SRCALPHA, 32)
		image.fill((255,255,255),(Vector2(self.piece_selected)*TILE_SIZE,(TILE_SIZE,TILE_SIZE)))
		k = self.current_player
		m_pos = pygame.mouse.get_pos()
		
		snapped_pos = ((math.floor(m_pos[0]/TILE_SIZE)+0.5)*TILE_SIZE,(math.floor(m_pos[1]/TILE_SIZE)+0.5)*TILE_SIZE)

		pygame.draw.circle(image,PLAYER_COLORS[k][1],snapped_pos,TILE_SIZE/2-7)
		pygame.draw.circle(image,PLAYER_COLORS[k][0],snapped_pos,TILE_SIZE/2-7-2)
		
		pygame.draw.circle(image,PLAYER_COLORS[k][1],m_pos,TILE_SIZE/2-7)
		pygame.draw.circle(image,PLAYER_COLORS[k][0],m_pos,TILE_SIZE/2-7-2)

		image.set_alpha(128)
		self.game_display.blit(image,(0,0),)
	
	# show AI rated moves (separate function for threading purposes)
	def threaded_AI_display(self):
		self.assist_image = self.image_from_moves(self.board,self.current_player)
		self.update_assist = False
	
	# playing moves

	def play_AI_move(self,moves):
		self.locked = True
		self.AI_thread1 = threading.Thread(target=self.threaded_AI_move, args=[moves],daemon=True)
		self.AI_thread1.start()
	
	def threaded_AI_move(self,moves):
		
		scored_moves = AI.threaded_score_moves(self.board,self.current_player,moves,difficulty=9)
		move = AI.choose_move(scored_moves)
		self.make_move(move)
		self.locked = False
	
	def make_move(self,move):
		self.current_player = 1-self.current_player
		self.set_board(board_handler.move_piece(self.board,1-self.current_player,move))

	# board manipulation

	def load_board(self,file_path = "data/data"):
		board = bytearray()
		with open(file_path,"rb") as file:
			board = bytearray(file.read(16))
			rating:float = np.fromfile(file, '<f4',1)
		print(rating)
		return board

	def set_board(self,new_board):
		self.board = new_board
		self.piece_selected = None
		self.update_board = True
		self.update_assist = True
		self.update_control_boards = True

# Event handler
def handle_event(game:Game,event):
	if event.type == pygame.QUIT:
		game.running = False
		return
	
	if event.type == pygame.KEYDOWN:
		if event.key ==  pygame.K_F2:
			game.show_assist = not game.show_assist # Toggle Assist
		if event.key ==  pygame.K_F3:
			game.show_control = not game.show_control # Toggle control overlay
		elif event.key ==  pygame.K_a:
			game.auto[1] = not game.auto[1] # Switch player 2 between manual and automatic
		elif event.key ==  pygame.K_q:
			game.auto[0] = not game.auto[0] # Switch player 1 between manual and automatic
		elif event.key ==  pygame.K_s:
			game.step_once = True # Request that the AI plays one move
		elif event.key ==  pygame.K_ESCAPE:
			pass#paused = not paused # Pausing curretly doesn't exist
		elif event.key == pygame.K_KP_PLUS:
			pass#fps_next_index= min(len(FPS_SELECTOR)-1,fps_next_index+1) # Depracted
		elif event.key == pygame.K_KP_MINUS:
			pass#fps_next_index= max(0,fps_next_index-1) # Depracted
		elif event.key == pygame.K_r:
			game.initialize_game() # reset game
		return

	# handle mouse controls
	if event.type == pygame.MOUSEBUTTONDOWN and not game.locked:
		if event.button == 1:
			m_pos = pygame.mouse.get_pos()
			tile_pos = (math.floor(m_pos[0]/64),math.floor(m_pos[1]/64))
			if board_handler.get_piece(game.board,game.current_player,tile_pos) == 1 and not game.piece_selected == tile_pos:
				game.piece_selected = tile_pos
			elif game.piece_selected:
				move = bytearray([board_handler.pos_to_byte(game.piece_selected),board_handler.pos_to_byte(tile_pos)])
				if move in board_handler.find_board_moves(game.board,game.current_player):
					game.make_move(move)
				game.piece_selected = None
	elif event.type == pygame.MOUSEMOTION:
		buttons = pygame.mouse.get_pressed()
		rel = pygame.mouse.get_rel()
		if buttons[1] == 1:
			pass

