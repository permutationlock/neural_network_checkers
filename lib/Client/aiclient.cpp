// aiclient.cpp
// Aven Bross, Nathan Helms, Nick
// 3-12-2014
//
// Client set up for checkers system
// Code uses based on client by Glenn Chappel and uses Orion Lawlor's socket library

#include "osl/socket.h"
#include "osl/socket.cpp"

#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
#include <string>
using std::string;
#include <sstream>
using std::istringstream;
#include <cstdlib>
using std::exit;
#include <vector>
#include <unistd.h>
using std::vector;
#include "../Board/checkerAI.h"

NeuralNet nn("../Generations/gen2076net0.bin");
vector<CheckerBoard> pBoards;

//Returns the error code if error, otherwise returns 0
int handleStatus(char c){
	if(c & 0b10000000) return 0;
	c = c & 0b01111111; //Truncate first bit
	return c;
}

// my_skt_abort
// Substitute for Lawlor's default skt_abort function
// Prints err msg to stderr & returns -1
// Does NOT exit
int my_skt_abort(int code,const char *msg)
{
    cerr << "Fatal socket error-- "
         << msg << " (" << code << ")" << endl;
    return -1;
}


// my_skt_recv_string
// Substitute for Lawlor's skt_recv_string
// Does error checking
// Returns 0 on success, -1 on error, using skt_abort
// String read is in theString
int my_skt_recv_string(vector<char> & thebytes,
                       SOCKET skt)
{
    char c;
    //Grab the 65 byte message
    for(int i=0; i<65; i++){
        int retval = skt_recvN(skt, &c, 1); // Grab next character
        if (retval != 0){ return retval; }
        else { thebytes.push_back(c); } // normal char; add to buffer, continue
    }
}




int main()
{
	int retval;              // For return values of skt funcs
	vector<char> buffer;           // Holds chars received over socket

	// Set skt abort function to mine (above)
	skt_set_abort(my_skt_abort);

	unsigned int port = 42069;  // Standard HTTP port
	skt_ip_t ip = {172,20,229,207}; 
		             // Use this form to specify the IP no.

	cout<<"Connecting to 172.20.227.188...\n";

	if (skt_ip_match(ip, _skt_invalid_ip))
	{
		cerr << "EXITING" << endl;
		exit(1);
	}

	// Try to connect with the give IP & port number
	SOCKET s = skt_connect(ip, port, 60);
		             // 5-second timeout
	if (s == -1)
	{
		cerr << "EXITING" << endl;
		exit(1);
	}
	
	cout<<"Connected!\n";

	string msg = "Ready to Play";              // HTTP request to send

	//Set up game
	cout << "Sending Ready" << endl;
	// Send message
	retval = skt_sendN(s, msg.c_str(), msg.size()+1);
	if (retval == -1)
	{
		cerr << "EXITING" << endl;
		exit(1);
	}

	cout << "Getting Response" << endl << endl;

	while(buffer.size()!=65){
		buffer.clear();
		//Reads body of TCP packet
		retval = my_skt_recv_string(buffer, s);
		if (retval == -1)
		{
			// Read error; give up (but don't crash!)
			skt_close(s);
			return 0;
		}
	}

	cout<<buffer.size()<<"\n";

	for(int i=0; i<buffer.size(); i++)
		cout<<buffer[i];
	cout<<"\n";

	buffer.erase(buffer.begin());

	CheckerBoard currBoard(buffer);

	//Compare the given board against a starting board to see what color you are
	CheckerBoard initial;

	bool isRed = (currBoard == initial);

	cout<< (isRed ? "true" : "false") << endl;

	bool isOver = false;

	bool isIllegal=false;

	int bfcount=0;

	while(!isOver){
		//Set up game
		// Send message (for right now just the current board back again)
		//retval = skt_sendN(s, ((currBoard).byteBoard()).c_str(), msg.size()+1);

		cout<<currBoard<<"\n";
		currBoard = getMove(currBoard, nn, isRed);
		sleep(2);

		cout << "Sending Board" << endl;
		char c = '0';
		if(currBoard.isDone()) c = '1';
		if(isIllegal || bfcount>4) c = '2'; 

		string temp = "";
		temp+=c;
		temp+=currBoard.byteBoard();
		cout<<"bsize: "<<temp.size()<<"\n";
		cout<<temp;

		retval = skt_sendN(s,temp.c_str(),temp.size());
		if (retval == -1)
		{
			cerr << "EXITING" << endl;
			exit(1);
		}

		cout<<"Reading Board" << endl;

		while(buffer.size()!=65){
			buffer.clear();
			//Reads body of TCP packet
			retval = my_skt_recv_string(buffer, s);
			if (retval == -1)
			{
				// Read error; give up (but don't crash!)
				skt_close(s);
				return 0;
			}
		}
		int err = handleStatus(buffer[0]);
		switch(err){
			default:
			case 0: break;
			//All errors just end the game for now
			case 1: 
			case 2: 
			case 3: isOver = true;
				break;
		}

		buffer.erase(buffer.begin());	//Delete the status byte

		vector<CheckerBoard> opponentMoves = posMoves(currBoard, !isRed);

		currBoard = CheckerBoard(buffer);	//Make a board out of the buffer of chars

		isIllegal=true;
		for(int i=0; i<opponentMoves.size(); i++){
			if(currBoard == opponentMoves[i]) { isIllegal = false; break; }
		}

		bool dup=false;
		for(int i=0; i<pBoards.size(); i++){
			if(currBoard==pBoards[i]){
				dup=true;
				break;
			}
		}
		if(dup){
			bfcount++;
		}
		else{
			bfcount=0;
		}

		pBoards.push_back(currBoard);

		if(pBoards.size()>4){
			pBoards.erase(pBoards.begin());
		}
	}

	// Done; close socket
	skt_close(s);

	return 0;
}


















