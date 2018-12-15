import java.util.Random;
import java.util.Hashtable;
/// A sample AI that uses a Monte Carlo approach to play Connect Four.

/**
 * A sampleAI that uses a Monte Carlo approach to play Connect Four.  Unlike the heuristic
 * searches and minimax approaches we've covered in class, the Monte Carlo player plays
 * Connect Four by simulating purely random games and choosing the move that has the highest
 * expected outcome.  Since the Monte Carlo player plays moves randomly,
 * it does not always play the optimal move (see if you can convince yourself about why this is),
 * but is good at strategic play and likes to make threats.
 * 
 * Unlike StupidAI and RandomAI, this AI player's getNextMove function will continues to play
 * random games indefinitely until the terminate flag is set.
 * 
 * @author Leonid Shamis
 */
public class minimax_SODONE extends AIModule
{
	/// Random number generator to play random games.
	//private final Random r = new Random(System.currentTimeMillis());
	/// Used as a helper when picking random moves.
	private int ourPlayer;				//Our AI
	private int opponent;				//Opponent
	private int boardHeight;			//Board height
	private int boardWidth;				//Board width
	private boolean maxPlayer = true;	//var for easy use in minimax (MAX)
	private boolean minPlayer = false;	//var for easy use in minimax (MIN)
	private int maxDepth = 2;		//maximum depth to search
	private boolean init = true;		//If true, do initial setup stuff
	private int centerPref[] = {25, 50, 75, 100, 75, 50, 25};	//Array for center weighting. Hardcoded: will only work if width = 7
	private int blankScores[] = {25000,300,100,10};
	private int bstScore;
	/// Simulates random games and chooses the move that leads to the highest expected value.
	@Override
	public void getNextMove(final GameStateModule state)
	{		

		// Default to choosing the first column (should be fixed after a few rounds)
		chosenMove = 0;
		GameStateModule gameCopy = state.copy();
		// Get our player
		ourPlayer = gameCopy.getActivePlayer();

		// Gets our opponent
		if(ourPlayer == 1)
			opponent = 2;
		else
			opponent = 1;

		//If the very first call, init our board array for scores
		if(init)
		{
			boardHeight = state.getHeight();
			boardWidth = state.getWidth();
			init = false;
		}
		//Copy the game so we can work with it
		//System.out.println("OurPlayer: " + ourPlayer);
		//System.out.println("New Move ---------------------------------------------------------");

		//call with copy of the game, cur depth of 0, and the maxPlayer
		while(!terminate)
		{
			minimax(gameCopy, 0, maxPlayer, -1);
		}
		//System.out.println("Chose Column: " + chosenMove + " Score: " + bstScore);
	}

	//TODO: POSSIBLE SPEEDUP BY STORING STATES FROM PREVIOUS MINIMAX RUNS PER GETNEXTMOVE()
	public int minimax(GameStateModule gameCopy, int curDepth, boolean minOrMaxPlayer, int lastMove)
	{
		int bestValue = 0, score;

		//System.out.println(minOrMaxPlayer + " Current Player (Max True, Min False)");

		//If the gameCopy has gotten to the end or we've reached our depth limit, return score
		if(gameCopy.isGameOver() || curDepth >= maxDepth)
		{
			///System.out.println("Winner! " + gameCopy.getWinner() + " Won the game!" );
		//	if(gameCopy.getWinner() == ourPlayer) //we won
		//	{
		//		System.out.println("Returning Active Player: " + gameCopy.getActivePlayer() + " OurPlayer: " + ourPlayer);
		//		return Integer.MAX_VALUE;
		//	}
		//	else //we lost
		//	{
		//		System.out.println("Returning Active Player: " + gameCopy.getActivePlayer() + " OurPlayer: " + ourPlayer);
		//		return -Integer.MAX_VALUE;
		//	}
			return getScore(gameCopy, lastMove, curDepth);
		}
		else
		{
			//Below is a translation of the MINIMAX psuedo code from Wikipedia

			//TRUE for MAX player
			if(minOrMaxPlayer)
			{
				//Set best value to -infty for first run
				bestValue = -Integer.MAX_VALUE;
				for(int col = 0; col < boardWidth; col++)
				{
					//If move is possible
					if(gameCopy.canMakeMove(col))
					{
						//Make move in the copy of the game 
						gameCopy.makeMove(col);
						//System.out.println("Calling min");
						//Call minimax with MIN player with the above move made
						score = minimax(gameCopy, (curDepth + 1), minPlayer, col);
						//unmake the previous move (recursion stuff)
						gameCopy.unMakeMove();

						//System.out.println("Max, Col: " + col + " Score: " + score);

						//same as max(score, bestvalue) but we need to keep track of the best move, ONLY if we're at depth 0 (top of the tree)
						if(score > bestValue)
						{
							bestValue = score;
							if(curDepth == 0)
							{
								chosenMove = col;
								bstScore = score;
							}
						}
					}
				}
				//return the best value of max up
				return bestValue;
			}
			else
			{
				//Same as above except MIN is swapped for MAX and we want score < bestValue
				bestValue = Integer.MAX_VALUE;
				for(int col = 0; col < boardWidth; col++)
				{
					if(gameCopy.canMakeMove(col))
					{
						gameCopy.makeMove(col);
						//System.out.println("Calling max");
						score = minimax(gameCopy, (curDepth + 1), maxPlayer, col);
						gameCopy.unMakeMove();
						if(score < bestValue)
						{
							bestValue = score;
							if(curDepth == 0)
							{
								chosenMove = col;
								//bstScore = score;
								//System.out.println("Min, Col: " + col + " Score: " + bestValue);
							}
						}
					}
				}
				return bestValue;
			}
			
		}
			
	}

	//Gets the score for the entire board state.
	//Our heuristic is to check every position of the board that was created in minimax
	//Check horizontal, vertical, and all diagonals. If our opponent is block a move, give score 0
	//For each possible path to getting 4 in a row, add the score depending on how many blanks there are.
	//Because it does it for every single space, will also count the opponents coins so we can properly block him from winning
	public int getScore(GameStateModule gameCopy, int lastMove, int curDepth)
	{
		//System.out.println("----------------------------------------");
		int totalStateScore = 0;
		int hScore = 0, vScore = 0, rdScore = 0, ldScore = 0;
		//Checks left-right
		
		//for(int y = (boardHeight - 1); y >= 0 ; y--)
		//{
		//	for(int x = 0; x < boardWidth; x++)
		//	{
		//		System.out.print(gameCopy.getAt(x,y) + " ");
		//	}
		//	System.out.println();
		//}

		//Get the scores of the entire state by checking right, up, right diagonal, and left diagonoal		
		hScore = checkHor(gameCopy);
		//System.out.println("Hor Score: " + hScore);
		//Checks up-down
		vScore = checkVert(gameCopy);
		//System.out.println("Ver Score: " + vScore);
		//Checks bottom left to upper right
		rdScore = checkUpperRightDia(gameCopy);
		//System.out.println("rdScore: " + rdScore);
		//Checks bottom right to upper left
		ldScore = checkUpperLeftDia(gameCopy);
		//System.out.println("ldScore: " + ldScore);
		//System.out.println(hScore + vScore + rdScore + ldScore);	

		//Return summed scores
		totalStateScore = hScore + vScore + rdScore + ldScore;
		//System.out.println("Total Score For Move " + lastMove + ": " + totalStateScore);
		if(gameCopy.isGameOver())
		{
		//	///System.out.println("Winner! " + gameCopy.getWinner() + " Won the game!" );
			if(gameCopy.getWinner() == ourPlayer) //we won
			{
				//If there exists a better/worse state than a winning state (idk why this would happen, but juuust in case)
				if(totalStateScore < 50000)
					return 50000 / curDepth;	//I found this helped a ton. If a winning state is found closer to depth 0, it should have a higher weight. This helpde
				else							//where we found like 5 different "winning" states, but each at different depths. Now we block better and win faster
					;//return totalStateScore;
			}
			else //we lost
			{
				if(totalStateScore > -50000)	//Same as above but for opponent
					return -50000 / (curDepth*2);
				else 
					;//return totalStateScore;
			}
		//	System.out.println("--- IN GAME OVER, ourPlayer: " + ourPlayer + " getWinner(): " + gameCopy.getWinner());
		}
		return (hScore + vScore + rdScore + ldScore);
	}	

	public int checkHor(GameStateModule gameCopy)
	{
		//Sum of the score
		int totalScore = 0;
		//Temp for summing
		int tempScore;
		//Total blanks for current check
		int blanks;
		//Total pieces for current check
		int pieces;
		//Current piece from the board
		int curPiece;
		//Keeps track of who the current checked piece is
		int pieceType;
		//Boolean for keeping track of whether a move is playable

		//System.out.print("Blanks: ");
		//System.out.println("We are: " + ourPlayer + " Opponent is: " + opponent + " CURRENT PLAYER: " + gameCopy.getActivePlayer());
		for(int row = 0; row < boardHeight; row++)
		{
			for(int col = 0; col < boardWidth; col++)
			{	
				//Per coin, set number of blanks and pieces to 0
				blanks = 0;
				//Since we're checking a coin, it'll be the first
				pieces = 1;
				//Set temp score
				tempScore = 0;
				//Init noMove to false
				//Get the current piece we're looking at
				curPiece = gameCopy.getAt(col, row);
				pieceType = curPiece;
				
				//System.out.print(" Current Piece at [" + col + "][" + row + "] = " + curPiece + " ||| ");
				if(curPiece == 0) //If the current spot is blank, nothing to check
					continue;

				//System.out.print("Col + 3: " + (col + 3) + " >? " + boardWidth);
				if( (col + 3) >= boardWidth) //If its not possible to get four to the right
					continue;	//TODO: CAN POSSIBLY JUST MAKE THE FOR LOOP GO TO THE CENTER THEN STOP
							
				//System.out.print(" - In not stop right ");
					//Check the next 3 pieces
				for(int pos = 1; pos < 4; pos++)
				{
					curPiece = gameCopy.getAt(col + pos, row);
					if(curPiece != pieceType && curPiece != 0)
					{
						//System.out.println("Move: " + col + " Checking: [" + (col+pos) + "][" + row + "] For Piece: " + curPiece + " Piece Type: " + pieceType + " hit Opp Piece  ");
						break;
					}
					
					if(curPiece == 0)
						blanks++;
					else
						pieces++;
				}
				

				//System.out.print(blanks + " ");

				//If we check atleast 4 pieces:
				if( (pieces + blanks) >= 4)
				{
					if(blanks == 0)
						tempScore = blankScores[0]; //If we win? Not sure why I added this, more of a "just in case" kind of thing
					if(blanks == 1) //We're 1 away from 4-in-a-row
						tempScore = blankScores[1];
					if(blanks == 2) //We're 2 away from 4-in-a-row
						tempScore = blankScores[2];
					if(blanks == 3) //We're 3 away from 4-in-a-row
						tempScore = blankScores[3];
					tempScore += centerPref[col];
					if(pieceType == opponent) //If the pieces we were checking was for our opponent, make it negative so our totalscore goes down
						tempScore *= -1;
						//This above check is so that the overall state score will go way down the closer our opponent is
					//System.out.print("tempScore = " + tempScore + "\n");

					totalScore += tempScore;
				}
			}
		}
		//System.out.println("Score: " + totalScore + "\n");
		return totalScore;
	}

	//Checks the verticals
	public int checkVert(GameStateModule gameCopy)
	{
		//Sum of the score
		int totalScore = 0;
		//Temp for summing
		int tempScore;
		//Total blanks for current check
		int blanks;
		//Total pieces for current check
		int pieces;
		//Current piece from the board
		int curPiece;
		//Keeps track of who the current checked piece is
		int pieceType;
		//Boolean for keeping track of whether a move is playable

		//System.out.print("Blanks: ");

		for(int row = 0; row < boardHeight; row++)
		{
			for(int col = 0; col < boardWidth; col++)
			{	
				//Per coin, set number of blanks and pieces to 0
				blanks = 0;
				//Since we're checking a coin, it'll be the first
				pieces = 1;
				//Set temp score
				tempScore = 0;
				//Init noMove to false
				//Get the current piece we're looking at
				curPiece = gameCopy.getAt(col, row);
				pieceType = curPiece;
				
				//System.out.print(" Current Piece at [" + col + "][" + row + "] = " + curPiece + " ||| ");
				if(curPiece == 0) //If the current spot is blank, nothing to check
					continue;

				//System.out.print("Col + 3: " + (col + 3) + " >? " + boardWidth);
				if( (row + 3) >= boardHeight) //If its not possible to get four to the right
					continue;	//TODO: CAN POSSIBLY JUST MAKE THE FOR LOOP GO TO THE CENTER THEN STOP
							
				//System.out.print(" - In not stop right ");
				for(int pos = 1; pos < 4; pos++)
				{
					curPiece = gameCopy.getAt(col, row + 1);
					if(curPiece != pieceType && curPiece != 0)
						break;
					
					if(curPiece == 0)
						blanks++;
					else
						pieces++;
				}
				

				//System.out.print(blanks + " ");

				//If we didn't break from the for loop, we can get a valid score
				if( (pieces + blanks) >= 4)
				{
					if(blanks == 0)
						tempScore = blankScores[0];
					if(blanks == 1) //We're 1 away from 4-in-a-row
						tempScore = blankScores[1];
					if(blanks == 2) //We're 2 away from 4-in-a-row
						tempScore = blankScores[2];
					if(blanks == 3) //We're 3 away from 4-in-a-row
						tempScore = blankScores[3];
					tempScore += centerPref[col];
					if(pieceType == opponent) //If the pieces we were checking was for our opponent, make it negative so our totalscore goes down
						tempScore *= -1;
						//This above check is so that the overall state score will go way down the closer our opponent is
					//System.out.print("tempScore = " + tempScore + "\n");
					totalScore += tempScore;
				}
			}
		}
		//System.out.println("Score: " + totalScore + "\n");
		return totalScore;
	}

	//Checks bottom left to upper right
	public int checkUpperRightDia(GameStateModule gameCopy)
	{
		//Sum of the score
		int totalScore = 0;
		//Temp for summing
		int tempScore;
		//Total blanks for current check
		int blanks;
		//Total pieces for current check
		int pieces;
		//Current piece from the board
		int curPiece;
		//Keeps track of who the current checked piece is
		int pieceType;
		//Boolean for keeping track of whether a move is playable

		//System.out.print("Blanks: ");

		for(int row = 0; row < boardHeight; row++)
		{
			for(int col = 0; col < boardWidth; col++)
			{	
				//Per coin, set number of blanks and pieces to 0
				blanks = 0;
				//Since we're checking a coin, it'll be the first
				pieces = 1;
				//Set temp score
				tempScore = 0;
				//Init noMove to false
				//Get the current piece we're looking at
				curPiece = gameCopy.getAt(col, row);
				pieceType = curPiece;
				
				//System.out.print(" Current Piece at [" + col + "][" + row + "] = " + curPiece + " ||| ");
				if(curPiece == 0) //If the current spot is blank, nothing to check
					continue;

				//System.out.print("Col + 3: " + (col + 3) + " >? " + boardWidth);
				if( (col + 3) >= boardWidth || (row + 3) >= boardHeight) //If its not possible to get four to the right
					continue;	//TODO: CAN POSSIBLY JUST MAKE THE FOR LOOP GO TO THE CENTER THEN STOP
							
				//System.out.print(" - In not stop right ");
				for(int pos = 1; pos < 4; pos++)
				{
					curPiece = gameCopy.getAt(col + pos, row + pos);
					if(curPiece != pieceType && curPiece != 0)
						break;
					
					if(curPiece == 0)
						blanks++;
					else
						pieces++;
				}
				

				//System.out.print(blanks + " ");

				//If we didn't break from the for loop, we can get a valid score
				if( (pieces + blanks) >= 4)
				{
					if(blanks == 0)
						tempScore = blankScores[0];
					if(blanks == 1) //We're 1 away from 4-in-a-row
						tempScore = blankScores[1];
					if(blanks == 2) //We're 2 away from 4-in-a-row
						tempScore = blankScores[2];
					if(blanks == 3) //We're 3 away from 4-in-a-row
						tempScore = blankScores[3];
					totalScore += centerPref[col];	//Pieces near the center get a higher score
					if(pieceType == opponent) //If the pieces we were checking was for our opponent, make it negative so our totalscore goes down
						tempScore *= -1;
						//This above check is so that the overall state score will go way down the closer our opponent is
					//System.out.print("tempScore = " + tempScore + "\n");
					totalScore += tempScore;
				}
			}
		}
		//System.out.println("Score: " + totalScore + "\n");
		return totalScore;
	}

	//Checks the bottom right to the upper left
	public int checkUpperLeftDia(GameStateModule gameCopy)
	{
		//Sum of the score
		int totalScore = 0;
		//Temp for summing
		int tempScore;
		//Total blanks for current check
		int blanks;
		//Total pieces for current check
		int pieces;
		//Current piece from the board
		int curPiece;
		//Keeps track of who the current checked piece is
		int pieceType;
		//Boolean for keeping track of whether a move is playable

		//System.out.print("Blanks: ");

		for(int row = 0; row < boardHeight; row++)
		{
			for(int col = 0; col < boardWidth; col++)
			{	
				//Per coin, set number of blanks and pieces to 0
				blanks = 0;
				//Since we're checking a coin, it'll be the first
				pieces = 1;
				//Set temp score
				tempScore = 0;
				//Init noMove to false
				//Get the current piece we're looking at
				curPiece = gameCopy.getAt(col, row);
				pieceType = curPiece;
				
				//System.out.print(" Current Piece at [" + col + "][" + row + "] = " + curPiece + " ||| ");
				if(curPiece == 0) //If the current spot is blank, nothing to check
					continue;

				//System.out.print("Col + 3: " + (col + 3) + " >? " + boardWidth);
				if( (col - 3) >= 0 || (row + 3) >= boardHeight) //If its not possible to get four to the right
					continue;	//TODO: CAN POSSIBLY JUST MAKE THE FOR LOOP GO TO THE CENTER THEN STOP
							
				//System.out.print(" - In not stop right ");
				for(int pos = 1; pos < 4; pos++)
				{
					curPiece = gameCopy.getAt(col - pos, row + pos);
					if(curPiece != pieceType && curPiece != 0)
						break;
					
					if(curPiece == 0)
						blanks++;
					else
						pieces++;
				}
				

				//System.out.print(blanks + " ");

				//If we didn't break from the for loop, we can get a valid score
				if( (pieces + blanks) >= 4)
				{
					if(blanks == 0)
						tempScore = blankScores[0];
					if(blanks == 1) //We're 1 away from 4-in-a-row
						tempScore = blankScores[1];
					if(blanks == 2) //We're 2 away from 4-in-a-row
						tempScore = blankScores[2];
					if(blanks == 3) //We're 3 away from 4-in-a-row
						tempScore = blankScores[3];
					totalScore += centerPref[col];
					if(pieceType == opponent) //If the pieces we were checking was for our opponent, make it negative so our totalscore goes down
						tempScore *= -1;
						//This above check is so that the overall state score will go way down the closer our opponent is
					//System.out.print("tempScore = " + tempScore + "\n");
					totalScore += tempScore;
				}
			}
		}
		//System.out.println("Score: " + totalScore + "\n");
		return totalScore;
	}


	/*
	*	Psuedo from Wikipedia
	*
	*	function minimax(node, depth, maximizingPlayer)
	*		if depth = 0 or node is a terminal node
	*			return the heuristic value of node
	*		if maximizingPlayer
	*			bestValue = -inf
	*			for each child of node
	*				v = minimax(child, depth - 1, FALSE)
	*				bestValue = max(bestValue, v)
	&			return bestValue
	*		else //MINIMIZING
	*			bestValue = inf
	*			for each child of node
	*				v = minimax(child, depth - 1, TRUE)
	*				bestValue = min(bestValue, v)
	*			return bestValue
	*/				
}



