//getAge.cpp
//Author: Aven Bross
//Date: 4/12/2014
//Little program that can be called to grab the age of a net

#include "checkerAI.h"
#include <iostream>
using std::cout;

int main(int argc, char* argv[]){
	if(argc<2){
		cout<<"Usage: " << argv[0] << " neuralnetfile.bin\n";
	}
	
	NeuralNet s(argv[1]);
	cout << argv[1] << ":\tage = " << s.getAge() << "\tancestors = " << s.getAncestors() << "\t king value = " << s.getKingValue() << "\n";
}
