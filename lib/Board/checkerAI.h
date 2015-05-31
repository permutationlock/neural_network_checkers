//checkerAI.h
//Author: Aven Bross
//Date: 3/23/2014
//Functions that put together the neural net and checkerboard functions

#ifndef CHECKERAI_H
#define CHECKERAI_H

#include "board.h"
#include "neuralnet.h"	//C++11 neural net
#include <algorithm>
using std::max;
using std::min;
using std::sort;
#include <string>
using std::string;
#include <sstream>
#include <fstream>
#include <omp.h>

const int SEARCH_PLY = 8;

//Recursive alpha beta search function
double alphabeta(const CheckerBoard & node, NeuralNet & nn, int depth, double alpha, double beta, bool maximizingPlayer, bool redMax){
    if(depth <= 0 || node.isDone())
        return *(nn.evaluate(node.getDoubleArray(nn.getKingValue(), redMax)));

    if(maximizingPlayer){
	vector<CheckerBoard> children = posMoves(node, redMax);

	if(children.size()==0) alpha = max(alpha, alphabeta(node, nn, depth - 1, alpha, beta, false, redMax));
        for(int i=0; i<children.size(); i++){
            alpha = max(alpha, alphabeta(children[i], nn, depth - 1, alpha, beta, false, redMax));
            if(beta <= alpha)
                break;
	}
        return alpha;
    }
    else{
	vector<CheckerBoard> children = posMoves(node, !redMax);
	if(children.size()==0) beta = min(beta, alphabeta(node, nn, depth - 1, alpha, beta, true, redMax));
        for(int i=0; i<children.size(); i++){
            beta = min(beta, alphabeta(children[i], nn, depth - 1, alpha, beta, true, redMax));
            if(beta <= alpha)
                break;
	}
        return beta;
    }
}

//Returns a best move chosen via Alpha Beta searching
CheckerBoard getMove(const CheckerBoard & board, NeuralNet & nn , bool isRed){
	vector<CheckerBoard> moves = posMoves(board, isRed);

	if(moves.size()==0) return board;

	if(moves.size()==1) return moves[0];

	int bestIndex = 0;
	double bestValue = alphabeta(moves[0], nn, SEARCH_PLY-1, -100, 100, false, isRed);

	for(int i=1; i<moves.size(); i++){
		double value = alphabeta(moves[i], nn, SEARCH_PLY-1, -100, 100, false, isRed);
		if(value>bestValue){ 
			bestIndex = i;
			bestValue = value;
		}
	}
	return moves[bestIndex];
}

//Play a game between the given neural nets
int playGame(NeuralNet & n1, NeuralNet & n2){
	CheckerBoard cb;
	int drawCount=0;
	int pPieces=cb.numPieces();
	bool redTurn = false;
	while(!cb.isDone()){
		redTurn = !redTurn;
		if(redTurn) cb=getMove(cb, n1, true);
		else cb=getMove(cb, n2, false);
		if(cb.numPieces()==pPieces) drawCount++;
		else{
			drawCount=0;
			pPieces=cb.numPieces();
		}
		if(drawCount>10) return 0;
	}
	return redTurn ? 1 : -1;
}

//Hold a tournament for the nets and sorts them based on tournament score
void sortNets(vector <NeuralNet> & gen){
	//Play and score the tournament
	int s = gen.size();

	//Tournament loop done in parrallel with openmp
	#pragma omp parallel for
	for(int c=0; c<s*s; c++){
		int i = c/s;
		int j = c%s;
		if(i!=j){
			int res = playGame(gen[i], gen[j]);
			gen[i].addScore(res);
			gen[j].addScore(res*-1);
			//std::cout<<"Board " << i << " playing Board " << j << " result: " << res <<  "\n";
		}
	}

	//A different way of doing it, in testing the first was a bit faster
	//but we can see how things go on the supercomputer
	/*#pragma omp parallel for
	for(int i=0; i<s; i++){
		for(int j=0; j<s; j++){
			if(i!=j){
				int res = playGame(gen[i], gen[j]);
				gen[i].addScore(res);
				gen[j].addScore(res*-1);
			}
		}
	}*/
	
	//Sort the nets by tournament result
	sort(gen.begin(), gen.end());
}

//Goes to the next generation (assumes prevGen has been sorted)
void advanceGeneration(vector<NeuralNet> & prevGen, int gen){
	std::ofstream ss;
	ss.open("geniology.txt", std::ios_base::out | std::ios_base::app);
	int resources = 500;
	double divisor = prevGen.size();
	for(int i=0; i<prevGen.size(); i++){
		prevGen[i].age();
	}
	int a[4] = {32, 40, 10, 1};
	for(int i=0; i<5; i++){
		NeuralNet n(a, 4);
		prevGen.push_back(n);
	}
	for(int i=0; i<prevGen.size(); i++){
		ss<<i<<" name: "<<prevGen[i]._name<<"\t score: "<<(prevGen[i].getScore()+divisor)/(2*divisor)<<"\tage: "<<prevGen[i].getAge()<<"\tancest: "<<prevGen[i].getAncestors()<<"\tres = "<<resources;
		if(prevGen[i].getAge()==0) { ss<<" new\n"; continue; }
		if(resources<1) { ss<<" removed\n"; prevGen.erase(prevGen.begin()+i); i-=1; }
		else{
			double score = prevGen[i].getScore()+divisor;
			score=score/(2*divisor);
			score*=100;
			ss<<" had "<<(score-50)+1<<" children\n";
			resources-=score;
			if(resources<0 ) score+=resources;
			for(int j=0; j<=(score/50); j++){
				prevGen.push_back(prevGen[i]);
				prevGen[prevGen.size()-1].mutate();
			}
		}
	}
	ss<<"Gen: "<<gen<<" Size: "<<prevGen.size()<<"\n\n";
	for(int i=0; i<prevGen.size(); i++){
		prevGen[i].resetScore();
	}

	ss.close();

	/*//Replace bottom half of nets with mutated versions of top half
	for(int i=8; i<15; i++){
		prevGen[i]=prevGen[i-8];
		prevGen[i-8].age();
		prevGen[i].mutate();
	}

	int a[4] = {32, 40, 10, 1};
	for(int i=16; i<20; i++){
		NeuralNet n(a, 4);
		prevGen[i]=n;
	}

	//Reset tournament scores for all neural nets
	for(int i=0; i<prevGen.size(); i++){
		prevGen[i].resetScore();
	}*/
}

//Saves a generation to file
void saveGeneration(vector<NeuralNet> & gen, const string & prefix){
	std::stringstream ss;
	for(int i=0; i<gen.size(); i++){
		ss.str(prefix);
		ss.seekp(0, std::ios_base::end);
		ss<<"net_"<<i<<".bin";
		gen[i].save(ss.str());
	}
}

vector<NeuralNet> loadGeneration(const string & prefix, int size){
	vector<NeuralNet> gen;
	std::stringstream ss;
	for(int i=0; i<size; i++){
		ss.str(prefix);
		ss.seekp(0, std::ios_base::end);
		ss<<"net_"<<i<<".bin";
		NeuralNet nn(ss.str());
		gen.push_back(nn);
	}
	return gen;
}

#endif
