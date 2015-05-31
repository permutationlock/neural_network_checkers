//Author: Daniel Aven Bross
//Date: 2/26/2014
//
//Board Representation for AI project

#ifndef CHECKERBOARD_H
#define CHECKERBOARD_H

#include <vector>
using std::vector;
#include <iostream>
using std::ostream;
#include <string>
using std::string;
#include <algorithm>
using std::abs;

const int HEIGHT = 10;
const int WIDTH = 5;

const char BLACK_PIECE = '3';	//Mask for piece present
const char BLACK_KING = '7';	//Mask for is black piece
const char RED_PIECE = '1';	//Mask for is king
const char RED_KING = '5';	//Mask for is king
const char BLANK = '0';	//Mask for possible location (red square)
const char UNUSED = '8';	//max

//Scheme for char vectors:
//RED:		piece - r	king - R
//BLACK: 	piece - b	king - B
//BUFFER: 	X
//EMPTY: 	_

class CheckerBoard {

public:
	//Sets up a basic checker board setup
	CheckerBoard(): _bPieces(12), _rPieces(12) {
		for(int i=0; i<HEIGHT; i++){
			for(int j=0; j<WIDTH; j++){
				if(i==0 || i == HEIGHT-1)	_board.push_back('X');
				else if(i%2==0){
					if(j==0)	_board.push_back('X');
					else if(i<4)	_board.push_back('r');
					else if(i>5)	_board.push_back('b');
					else _board.push_back('_');
				}
				else{
					if(j==WIDTH-1)	_board.push_back('X');
					else if(i<4)	_board.push_back('r');
					else if(i>5)	_board.push_back('b');
					else _board.push_back('_');
				}
			}
		}
	}
	
	//Copy Constructor
	CheckerBoard(const CheckerBoard & other): _bPieces(other._bPieces), _rPieces(other._rPieces), _board(other._board) {}

	//Constructs board from server message
	CheckerBoard(const vector<char> & bytes): _bPieces(0), _rPieces(0){
		int count=0;
		for(int i=-1; i<9; i++){
			for(int j=-1; j<9; j++){
				if(i==-1) { if(abs(j)%2==1) _board.push_back('X'); }
				else if(i==8) { if(j%2==0) _board.push_back('X'); }
				else{
					if(j==-1) { if(i%2==1) _board.push_back('X'); }
					else if(j==8) { if(i%2==0) _board.push_back('X'); }
					else{
						count++;
						char c = bytes[i*8+j];;

						switch(c){
						case BLANK:
							_board.push_back('_');
							break;
						case RED_PIECE:
							_rPieces++;
							_board.push_back('r');
							break;
						case BLACK_PIECE:
							_bPieces++;
							_board.push_back('b');
							break;
						case RED_KING:
							_rPieces++;
							_board.push_back('R');
							break;
						case BLACK_KING:
							_bPieces++;
							_board.push_back('B');
							break;
						default:
							break;
						}
					}
				}
			}
		}
		//std::cout<<count;
	}
	
	//Returns  whether the inputs make a valid move (assumes inputs are for a possible move)
	bool checkMove(int end) const{
		return (_board[end]=='_');
	}
	
	//Returns whether the given inputs make a valid jump (assumes inputs are for a possible jump)
	bool checkJump(int start, int target, int end, bool isRed) const{
		if(isRed){
			if(_board[target]=='X' || (_board[target]!='b' && _board[target]!='B'))	return false;
			if(_board[end]=='_')	return true;
			return false;
		}
		else{
			if(_board[target]=='X' || (_board[target]!='r' && _board[target]!='R'))	return false;
			if(_board[end]=='_')	return true;
			return false;
		}
	}

	//Changes the piece at the given position to a king if at the end of the board
	//Returns whether the piece is a king after the update
	bool updateKings(int pos){
		if(_board[pos]=='B' || _board[pos]=='R') return true;
		else if(_board[pos]=='r' && pos > 8*WIDTH) { _board[pos]='R'; return true; }
		else if(_board[pos]=='b' && pos < 2*WIDTH) { _board[pos]='B'; return true; }
		return false;
	}
	
	//Make the given move (assumes the input values are a valid move)
	//Returns whether the piece is a king
	bool makeMove(int start, int end){
		_board[end]=_board[start];
		_board[start]='_';
		
		//Update kings
		return updateKings(end);
	}
	
	//Make the given jump move (assumes input values are a valid jump)
	//Returns whether the piece is a king
	bool makeJump(int start, int target, int end, bool isRed){
		_board[end]=_board[start];
		_board[start]='_';
		_board[target]='_';

		if(isRed) _bPieces--;
		else _rPieces--;
		
		return updateKings(end);
	}

	//Returns an vector of 32 doubles (1 for each valid board square
	vector<double> getDoubleArray(int kingValue, bool isRed) const{
		vector<double> arr;
		int coef = -1;
		if(isRed) coef=1;
		for(int i=0; i<HEIGHT; i++){
			for(int j=0; j<WIDTH; j++){
				char c = getPiece(i,j);
				if(c!='X'){
					switch(c){
					case '_' : arr.push_back(0);
						   break;
					case 'r' : arr.push_back(coef*1);
						   break;
					case 'b' : arr.push_back(coef*-1);
						   break;
					case 'R' : arr.push_back(coef*kingValue);
						   break;
					case 'B' : arr.push_back(coef*-1*kingValue);
						   break;
					}
				} 
			}
		}
		return arr;
	}
	
	int numPieces() const {
		return _bPieces+_rPieces;
	}

	//Check if the game is done
	bool isDone() const {
		return (_bPieces==0 || _rPieces==0);
	}

	//Convert the board to byteboard
	string byteBoard(){
		string s = "";
		for(int i=0; i<HEIGHT; i++){
			for(int j=0; j<WIDTH; j++){
				char piece = getPiece(i, j);
				if(piece=='X') continue;
				char c = UNUSED;
				switch(piece){
				case '_': c=BLANK; break;
				case 'r': c=RED_PIECE; break;
				case 'R': c=RED_KING; break;
				case 'b': c=BLACK_PIECE; break;
				case 'B': c=BLACK_KING; break;
				}
				if(i%2==0){
					s.append(1, UNUSED);
					s.append(1, c);
				}
				else{
					s.append(1, c);
					s.append(1, UNUSED);
				}
			}
		}
		return s;
	}
	
	//Returns the character for the piece at the given position 
	//(note col doesn't actually mean column but distance from the left since squares are offset)
	char getPiece(int row, int col) const{
		return _board[row*WIDTH+col];
	}

	//Comparison operator
	bool operator==(const CheckerBoard & other){
		for(int i=0; i<HEIGHT; i++){
			for(int j=0; j<WIDTH; j++){
				if(this->getPiece(i,j)!=other.getPiece(i,j)) return false;
			}
		}
		return true;
	}

	//Comparison operator
	bool operator!=(const CheckerBoard & other){
		return !(*this==other);
	}

private:

	//Tracks the number of pieces on the board
	int _rPieces;
	int _bPieces;
	vector<char> _board;
};

//cout print operator overload for the CheckerBoard class
ostream & operator<<(ostream & os, const CheckerBoard & cb){
	//Print the checkerboard, including blank spaces
	// _ is a possible but empty square, blank space are impossible squares
	for(int i=HEIGHT-1; i>=0; i--){
		if(i%2==1) os<<" "<<" ";
		for(int j=0; j<WIDTH; j++){
			os<<cb.getPiece(i,j);
			if(j<WIDTH-1) os<<" "<<" "<<" ";
			else os<< " ";
		}
		if(i%2==0) os<<" "<<" ";
		os<<"\n";
	}
	return os;
}

//Adds the possible jumps for this setup (helper function for posMoves)
void addPosJumps(vector<CheckerBoard> & allmoves, const CheckerBoard & board, int curPos, int coef, bool evenRow, bool isKing, bool isRed, bool jumping){
	int x=coef*1, jx = 0;	//Extra bits to be added depending on odd or even row (jx is for the jump row)

	//Determine whether we are on an odd or even row (jx will be the opposite since it is an adjacent row)
	if((evenRow && coef==1) || (coef==-1 && !evenRow)) { x=0; jx = coef*1; }	
	
	//Calculate first possible move and jump positions
	int adjPos = curPos+coef*(WIDTH)+x;
	int jumPos = (curPos+coef*WIDTH+x)+coef*WIDTH+jx;	

	bool gotJump = false;	//Tracks whether a possible jump is found

	//Check for jump 1
	if(board.checkJump(curPos, adjPos, jumPos, isRed)){
		gotJump=true;
		CheckerBoard tempb(board);
		tempb.makeJump(curPos, adjPos, jumPos, isRed);

		//Save move array size
		int ps = allmoves.size();

		//Check for forward jumps
		addPosJumps(allmoves, tempb, jumPos, coef, evenRow, isKing, isRed, true);

		//Computation for kings
		if(isKing){
			//Make sure we don't get repeats or midways
			bool diff = false;
			for(int i=ps; i<allmoves.size(); i++){
				if(allmoves[i]!=tempb) diff = true;
			}
			if(!diff) allmoves.erase(allmoves.begin()+ps, allmoves.end());

			//Save size again
			ps=allmoves.size();

			//Check for backward jumps
			addPosJumps(allmoves, tempb, jumPos, coef*-1, evenRow, isKing, isRed, true);

			//Make sure we don't get repeats or midways
			if(diff){
				diff=false;
				for(int i=ps; i<allmoves.size(); i++){
					if(allmoves[i]!=tempb) diff = true;
				}
				if(!diff) allmoves.erase(allmoves.begin()+ps, allmoves.end());
			}
		}
	}
	
	//Calculate second possible move and jump positions
	adjPos = curPos+coef*(WIDTH-1)+x;
	jumPos = (curPos+coef*(WIDTH-1)+x)+coef*(WIDTH-1)+jx;

	//Check for jump 2
	if(board.checkJump(curPos, adjPos, jumPos, isRed)){
		gotJump=true;
		CheckerBoard tempb(board);
		tempb.makeJump(curPos, adjPos, jumPos, isRed);

		//Save move array size
		int ps = allmoves.size();

		//Check for forward jumps
		addPosJumps(allmoves, tempb, jumPos, coef, evenRow, isKing, isRed, true);

		//Computation for kings
		if(isKing){
			//Make sure we don't get repeats or midways
			bool diff = false;
			for(int i=ps; i<allmoves.size(); i++){
				if(allmoves[i]!=tempb) diff = true;
			}
			if(!diff) allmoves.erase(allmoves.begin()+ps, allmoves.end());

			//Save size again
			ps=allmoves.size();

			//Check for backward jumps
			addPosJumps(allmoves, tempb, jumPos, coef*-1, evenRow, isKing, isRed, true);

			//Make sure we don't get repeats or midways
			if(diff){
				diff=false;
				for(int i=ps; i<allmoves.size(); i++){
					if(allmoves[i]!=tempb) diff = true;
				}
				if(!diff) allmoves.erase(allmoves.begin()+ps, allmoves.end());
			}
		}
	}

	if(jumping && !gotJump) allmoves.push_back(board);	//If we were jumping and can't find another jump, return this board
}

//Adds the possible moves for this setup (helper function for posMoves)
void addPosMoves(vector<CheckerBoard> & allmoves, const CheckerBoard & board, int curPos, int coef, bool evenRow, bool isKing, bool isRed){
	int x=coef*1, jx = 0;	//Extra bits to be added depending on odd or even row (jx is for the jump row)

	//Determine whether we are on an odd or even row (jx will be the opposite since it is an adjacent row)
	if((evenRow && coef==1) || (coef==-1 && !evenRow)) { x=0; jx = coef*1; }	
	
	//Calculate first possible move and jump positions
	int adjPos = curPos+coef*(WIDTH)+x;

	//Check for move and jump
	if(board.checkMove(adjPos)){
		CheckerBoard tempb(board);
		tempb.makeMove(curPos, adjPos);
		allmoves.push_back(tempb);
	}
	
	//Calculate second possible move and jump positions
	adjPos = curPos+coef*(WIDTH-1)+x;

	//Check for move and jump
	if(board.checkMove(adjPos)){
		CheckerBoard tempb(board);
		tempb.makeMove(curPos, adjPos);
		allmoves.push_back(tempb);
	}
}

//Returns all possible boards that could result from a turn of the player of the given color
vector<CheckerBoard> posMoves(const CheckerBoard & board, bool isRed){

	char piece, king;	//Tracks what characters are pieces and kings for this color
	int coef;	//Tracks the direction we are going
	
	//Vector to store a board for each move
	vector<CheckerBoard> allmoves;
	
	//Set variables up based on color
	if(isRed) { piece = 'r'; king = 'R'; coef = 1; }
	else      { piece = 'b'; king = 'B'; coef = -1; }
	
	//Go and find all possible resultant boards from possible jumps
	for(int i=0; i<HEIGHT; i++){
		for(int j=0; j<WIDTH; j++){
			int curPos = i*WIDTH+j;	//Current position
			//Call recursive check function, once for normal, twice for kings
			if(board.getPiece(i, j) == piece) addPosJumps(allmoves, board, curPos, coef, i%2==0, false, isRed, false);
			else if (board.getPiece(i, j) == king){ 
				addPosJumps(allmoves, board, curPos, coef, i%2==0, true, isRed, false);
				addPosJumps(allmoves, board, curPos, coef*-1, i%2==0, true, isRed, false);
			}
		}
	}
	
	//If there were no possible jumps, find possible moves
	if(allmoves.empty()){
		for(int i=0; i<HEIGHT; i++){
			for(int j=0; j<WIDTH; j++){
				int curPos = i*WIDTH+j;	//Current position
				//Call recursive check function, once for normal, twice for kings
				if(board.getPiece(i, j) == piece) addPosMoves(allmoves, board, curPos, coef, i%2==0, false, isRed);
				else if (board.getPiece(i, j) == king){ 
					addPosMoves(allmoves, board, curPos, coef, i%2==0, true, isRed);
					addPosMoves(allmoves, board, curPos, coef*-1, i%2==0, true, isRed);
				}
			}
		}
	}

	return allmoves;	//Return array of all possible moves
}


#endif
