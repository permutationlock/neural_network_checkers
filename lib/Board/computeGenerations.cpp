//computeGenerations.cpp
//Author: Aven Bross
//Date: 3/23/2014
//Functions that put together the neural net and checkerboard functions

#include "checkerAI.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
using std::cout;

const int NUM_GENS = 10000;
const int START_GEN = 233;

int main(){
	vector<NeuralNet> nets;
	if(START_GEN>0){
		std::stringstream ss;
		ss << "gen_" << START_GEN <<"_";
		nets = loadGeneration(ss.str(),20);
	}
	else{
		int s[4] = {32, 40, 10, 1};     //Contains the layer sizes to send to the NeuralNet class

		for(int i=0; i<20; i++){
		        NeuralNet a(s,4);
		        nets.push_back(a);
		}
	}


        for(int i=START_GEN; i<NUM_GENS; i++){
                //cout<<"Generation: " << i << "\n";
                std::stringstream ss;
                ss << "gen_" << i+1 <<"_";
                sortNets(nets);
                saveGeneration(nets, ss.str());
                advanceGeneration(nets,i);
        }
}

