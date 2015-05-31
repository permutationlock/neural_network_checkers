//neuralnet_boost.h
//Author: Aven Bross
//Date: 1/30/2014
//Neural net class using boost rather than c++11

/* NOTE:
 * Apparently I like making things overly complicated
 * and decided to use arrays and pointers to do my net.
 * I also wrote a version with simply a vector of nodes
 * after I realised how easy and readable that is, but
 * decided to turn this one in just to add a different
 * way of doing it to the pool of trials.  I did this
 * at around 4:00 in the morning and was really proud
 * at the time, looking back though I have no idea
 * what I was thinking.
*/

/* DESCRIPTION:
 * Neural net with variable layers and layer size.
 * Initial weights are set via a uniform distribution.
 * Random numbers use a 32 bit mersenne twister
 * Evaluate function returns a pointer so you can grab
 *   multiple output nodes if needed.
*/

#ifndef NEURALNET_H
#define NEURALNET_H

#include <cmath>
#include <random>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <boost/random.hpp>
using std::swap;
using std::vector;

const int WT_AMP = 1.0;	//Initial weights will be set using the distribution [-WT_AMP,WT_AMP]
const double MUT_STD_DEV = .002;	//The standard deviation of the normal distribution used for mutations

class NeuralNet {
public:
	typedef boost::mt19937 RNGType;

	//Contstruct a neural net with the given layers
	NeuralNet(int* size, int nLayers): _nLayers(nLayers), _age(0){
		RNGType rng;
		boost::uniform_real<double> udist(-1*WT_AMP,WT_AMP);
		boost::uniform_real<double> kdist(1.0, 2.0);
		boost::variate_generator< RNGType, boost::uniform_real<double> >
		            wDice(rng, udist);
		boost::variate_generator< RNGType, boost::uniform_real<double> >
		            kDice(rng, kdist);

		_kingValue = kDice();
		
		//Set up array storing layer sizes
		_size = new int[_nLayers];
		for(int i=0; i<_nLayers; i++)
			_size[i] = size[i];

		//Set up weight arrays (uses uniform distribution)
		_weights = new double**[_nLayers-1];
		for(int k=0; k<_nLayers-1; k++){			//For each layer k,
			_weights[k] = new double*[_size[k+1]];		//create an array of the size of the layer k+1
			for(int i=0; i<_size[k+1]; i++){		//Then for each element of this array,
				_weights[k][i] = new double[_size[k]];	//create an array the size of layer k to fill with weights.
				for(int j=0; j<_size[k]; j++)		//Then, for each element in that array,
					_weights[k][i][j] = wDice();	//fill with a random number from our uniform distribution
			}
		}

		//Set up output arrays for layers
		_values = new double*[_nLayers];
		for(int i=0; i<_nLayers; i++)			//For each layer,
			_values[i]= new double[_size[i]];	//create an array of the size of that layer
	}

	//Copy constructor, same as normal constructor but copying values (scores don't copy)
	NeuralNet(const NeuralNet & other): _nLayers(other._nLayers), _kingValue(other._kingValue),  _score(other._score), _age(other._age) {
		
		//Copy size array
		_size = new int[_nLayers];
		for(int i=0; i<_nLayers; i++)
			_size[i] = other._size[i];

		//Copy weight arrays
		_weights = new double**[_nLayers-1];
		for(int k=0; k<_nLayers-1; k++){
			_weights[k] = new double*[_size[k+1]];
			for(int i=0; i<_size[k+1]; i++){
				_weights[k][i] = new double[_size[k]];
				for(int j=0; j<_size[k]; j++)
					_weights[k][i][j] = other._weights[k][i][j];
			}
		}

		//Set up output array
		_values = new double*[_nLayers];
		for(int i=0; i<_nLayers; i++)
			_values[i]= new double[_size[i]];
	}

	//Copy constructor, same as normal constructor but copying values (scores don't copy)
	NeuralNet(string fname): _nLayers(0), _score(0), _age(0) {
		//Set up file I/O
		std::ifstream file;
		file.open(fname, std::ios::binary);
		file.seekg(0);

		//Read _age from the file
		file.read((char*)(&_age), sizeof(int));

		//Read _kingValue from the file
		file.read((char*)(&_kingValue), sizeof(double));
		
		//Read _nlayers from the file
		file.read((char*)(&_nLayers), sizeof(int));

		//Read _size array from the file
		_size = new int[_nLayers];
		file.read((char*)(_size), _nLayers*sizeof(int));

		//Read the weight array from the file
		_weights = new double**[_nLayers-1];
		for(int k=0; k<_nLayers-1; k++){
			_weights[k] = new double*[_size[k+1]];
			for(int i=0; i<_size[k+1]; i++){
				_weights[k][i] = new double[_size[k]];
				file.read((char*)(_weights[k][i]),  _size[k]*sizeof(double));
			}
		}

		//Cloes file
		file.close();

		//Set up output array
		_values = new double*[_nLayers];
		for(int i=0; i<_nLayers; i++)
			_values[i]= new double[_size[i]];
	}

	//Save the neural net to file
	void save(string fname){
		//Set up file I/O
		std::ofstream file;
		file.open(fname, std::ios::binary);
		file.seekp(0);

		//Write _kingValue first
		file.write((char*)(&_age), sizeof(int));

		//Write _kingValue first
		file.write((char*)(&_kingValue), sizeof(double));

		//Write _nlayers first
		file.write((char*)(&_nLayers), sizeof(int));

		//Write the size array
		file.write((char*)(_size), _nLayers*sizeof(int));

		//Write the weight array
		for(int k=0; k<_nLayers-1; k++){
			for(int i=0; i<_size[k+1]; i++){
				file.write((char*)(_weights[k][i]),  _size[k]*sizeof(double));
			}
		}

		file.close();
	}

	//Evaluate the network with the given inputs
	double* evaluate(const vector<double> & input){

		//Grab input values and store in 1st layer
		for(int i=0; i<_size[0]; i++)
			_values[0][i]=input[i];

		//Evaluate through each layer of the neural net (skipping input layer)
		for(int k=1; k<_nLayers; k++){
			for(int i=0; i<_size[k]; i++){
				
				double temp = 0.0;

				//Sum each input*weight product
				for(int j=0; j<_size[k-1]; j++){
					temp+=_weights[k-1][i][j]*_values[k-1][j];
				}

				//Convert to the range (-1.0,1.0) using a simplification of sigmoid function
				temp = temp/(1+abs(temp));

				//Save value in output array
				_values[k][i] = temp;
			}
		}

		//Return a pointer to output neuron values
		return _values[_nLayers-1];
	}

	//Mutate all weights in the set
	void mutate(){
		RNGType rng;
		boost::normal_distribution<> ndist(0,MUT_STD_DEV);	//create a normal distribution with mean 0 and the set std dev

		boost::variate_generator< RNGType, boost::normal_distribution<> >
		            nDice(rng, ndist);

		_age = 0; //Reset age since you are essentially making a new net

		//Mutate weight arrays (uses normal distribution)
		for(int k=0; k<_nLayers-1; k++){
			for(int i=0; i<_size[k+1]; i++){
				for(int j=0; j<_size[k]; j++){
					_weights[k][i][j] += nDice();
				}
			}
		}
	}
	
	void resetScore(){
		_score=0;
	}
	
	void addScore(int num){
		_score+=num;
	}
	
	int getScore() const{
		return _score;
	}

	double getKingValue() const{
		return _kingValue;
	}

	void age(){
		_age++;
	}

	int getAge(){
		return _age;
	}
	
	friend void swap(NeuralNet & first, NeuralNet & second){
		swap(first._age, second._age);
		swap(first._kingValue, second._kingValue);
		swap(first._nLayers, second._nLayers);
		swap(first._size, second._size);
		swap(first._weights, second._weights);
		swap(first._values, second._values);
		swap(first._score, second._score);
	}

	bool operator<(const NeuralNet & other)const{
		return _score > other.getScore();
	}
	
	NeuralNet & operator=(const NeuralNet& other){
		NeuralNet temp(other);
		swap(*this, temp);
		return *this;
	}
	
	//Destructor (not sure if I used too many deletes)
	~NeuralNet(){
		for(int k=0; k<_nLayers-1; k++){
			for(int i=0; i<_size[k+1]; i++){
				delete _weights[k][i];
			}
			delete _weights[k];
		}
		delete _weights;

		for(int i=0; i<_nLayers; i++)
			delete _values[i];
		delete _values;

		delete _size;
	}

private:
	double _kingValue;	//How much this net values kings
	int _age;
	int _nLayers;		//Stores number of layers in network			STRUCTURE:
	int* _size;		//Stores size of each layer. 				_size[layer]
	double*** _weights;	//Stores weights each non-input node			_weights[layer][current node][input node]
	double** _values;	//Stores calculated values for each non input node	_values[layer][node]
	int _score;
};

#endif
