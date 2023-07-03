# Board-game
 An application with an AI scoring different moves of the board game based on heuristics. It uses simple statistical methods to decide the moves which are more likely to win, using a depth first search with pruning.

## Versions
 ### Python version
 The python version was the original version written in preparation to use more complex machine learning techniques using tensorflow ; However, it was too slow to function as intended even with multi-core processing. For now it provides a good interactive way to play the game.

 ### C++ version
 Created to boost the speed of processing. Some techniques such as bitboards and lookup tables are used for optimization and it does work much faster than the python version. But it lacks the GUI of the python version and is currently only playable on the terminal. Despite that it is still usefull when computing AI vs AI matches.


---
 ## How to play
 ### Win condition
 Make sure the other player has no legal moves on their turn
 ### Legal moves
 Each turn players can make a move that may land one of their pieces either on an empty space or a space with an opposing piece removing it from play.
 There are 2 types of moves:
 #### Diagonal moves
 Move any of their pieces over one or more of their other pieces in one straight diagonal line
 #### Circular moves
 Move any of their pieces that is orthogonally adjacent to one of their other pieces to another place which is also orthogonally adjacent to that second piece as long as it can be moved by sliding the second piece around the first piece (the corners aren't blocked)

### Extra rules
- There are no draw conditions in this game
- If a player repeates the same move (use the same move to return to the same board state) they should be eliminated (currently not implemented)

## How to use
### Python
Run the `app.py` from the board_py directory
#### Controls:
- LMB	:Move a piece
- F2	:Toggle Assist
- F3	:Toggle control overlay
- Q		:Toggle player 1 AI
- A		:Toggle player 2 AI
- S		:Play AI move
- R		:Reset the game

### C++
Run the executable in `x64/Debug` or compile, build and execute the solution
- The controls are described at the beginning of the terminal
- The game runs in interactive mode by default, currently the rest of the functionality is hardcoded and requires hardcode changes.

