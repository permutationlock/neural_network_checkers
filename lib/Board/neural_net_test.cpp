//test.cpp
//Author: Aven Bross
//Date: 2/03/2014
//Simple place to test neural net class

#include "checkerAI.h"
#include <iostream>
#include <chrono>

int main(){
	int s[4] = {32, 40, 10, 1};	//Contains the layer sizes to send to the NeuralNet class

	NeuralNet n1(s, 4);	//Create NeuralNet with 4 layers of the given sizes

	float in[32];			//Array to store input for evaluate function
	for(int i=0; i<32; i++)		//Make each input 1 for the test
		in[i]=1;
	
	//Set up timing variables
	auto start = std::chrono::steady_clock::now();
	auto curr = start;
	float count = 0;
	//std::cout<< *(n1.evaluate(in)) << "\n";
	//n1.mutate();
	//std::cout<< *(n1.evaluate(in)) << "\n";

	CheckerBoard cb;
	NeuralNet nn1(s, 4);
	NeuralNet nn2(s, 4);

	double max = 0.0;

	std::cout << "Search Ply: " << SEARCH_PLY << "\n";

	bool redTurn = false;
	//See how many evaluations can be performed in 1 second (do 10 seconds then divide)
	do{
		auto tstart = std::chrono::steady_clock::now();
		cb = redTurn ? getMove(cb, nn1, true) : getMove(cb, nn2, false);
		count++;
		auto tcurr = std::chrono::steady_clock::now();
		double temp = std::chrono::duration<float>(tcurr-tstart).count();
		if(max<temp) max=temp;
		std::cout<<"count " << count << ": " << temp << "\n";
		redTurn=!redTurn;
	}
	while(count <20 && !cb.isDone());	//.2 found to be time for empty loop

	curr = std::chrono::steady_clock::now();

	std::cout << "Average: " << std::chrono::duration<float>(curr-start).count()/count << " Max: " << max << "\n";
}
