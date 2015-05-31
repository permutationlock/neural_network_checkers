// kvserver.cpp
// Daniel Aven Bross
// Modified from code by Glenn G. Chappell
// 19 Apr 2013
// Key Value Server

#include "osl/socket.h"

#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
#include <string>
using std::string;
#include <sstream>
using std::ostringstream;
#include <cstdlib>
using std::exit;
#include <thread>
using std::thread;
#include <mutex>
using std::mutex;
#include <vector>
using std::vector;

unsigned int port = 8888;  // Port to listen on

vector<string> key;
vector<string> value;
mutex val_lock; 

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
int my_skt_recv_string(std::string & theString,
                       SOCKET skt,
                       const char *term=" ")
{
    char c; 
    theString = "";
    while (true) {
        int retval = skt_recvN(skt, &c, 1); // Grab next character
        if (retval != 0) return retval;
        if (strchr(term, c))
        {
            return 0;          // Hit terminator-- stop.
        }
        else theString += c; // normal char; add to string, continue
    }
}


void respond(SOCKET s,
             const skt_ip_t & client_ip,
             unsigned int client_port)
{
    int retval;              // For return values of skt funcs
    string buffer;           // Holds chars received over socket

    // Got connection; print received message
    cout << "Connection received: " << endl;
    cerr << "IP: " << int(client_ip.data[0]) << "."
                   << int(client_ip.data[1]) << "."
                   << int(client_ip.data[2]) << "."
                   << int(client_ip.data[3]) << " - ";
    cerr << "PORT: " << client_port << endl;
    
    //Stor
    vector<string> message;
    
    //Reads body of TCP packet
    for(int i=0; i<3; i++)
    {
        retval = my_skt_recv_string(buffer, s, "");
        if (retval == -1)
        {
            // Read error; give up (but don't crash!)
            skt_close(s);
            return;
        }
        cout << buffer << " ";
	message.push_back(buffer);
    }
    cout << endl;

    //If a valid command is recieved and has a valid number of arguments
    if((message[0].at(0)=='S' && message.size()>3) || ( message[0].at(0)=='G' && message.size()>2)){
	//Stores index of value
	int i;

	//If a set command is recieved
	if(message[0].at(0)=='S'){
	    cout << "Setting key: " << message[2] << " to value: " << message[3] << endl;
	    //Lock the mutex, set the key and value then record the index
	    val_lock.lock();
	    key.push_back(message[2]);
	    value.push_back(message[3]);
	    i=value.size()-1;
	}
	//If a get command is recieved
	else{
	    cout<< "Locating Value" << endl;
            //Lock the mutex, find the corresponding index
	    val_lock.lock();
	    for(i=0; i<key.size(); i++)
		if(message[2].compare(key[i])) break;
	    //If the index is not found, record -1
	    i=-1;
	}
	//Unlock the mutex
	val_lock.unlock();
	
	//Send a correct response
	if(i>=0){
	    cout << "Sending response" << endl;
	    //Construct a value response consisting in the form: "V ID VALUE"
	    string response = ("V"+message[1]+value[i]);
	    cout << "Response: "<<response << endl;
	    retval = skt_sendN(s, response.c_str(), response.size());
	    if (retval == -1)
	    {
		cerr << "Error: could not send response" << endl;
		cerr << "Closing connection" << endl;
		skt_close(s);
		return;
	    }
	}
	else cout << "Key Not Found" << endl;

    }
    else cout << "Invalid Command" << endl;
    // Done; close socket
    cerr << "Success" << endl;
    cerr << "Closing connection" << endl;
    skt_close(s);
}


void listen(SERVER_SOCKET srv)
{
    skt_ip_t client_ip;      // IP & port of other end of connection
    unsigned int client_port;

    // Listen for TCP connection
    cout << endl;
    cout << "Listening" << endl;
    SOCKET s = skt_accept(srv, &client_ip, &client_port);
    if (s == -1)
    {
        // Could not get socket; give up (but don't crash!)
        return;
    }
    
    thread t(respond, s, client_ip, client_port);
    t.detach();
}


int main()
{
    // Set skt abort function to mine (above)
    skt_set_abort(my_skt_abort);

    SERVER_SOCKET srv = skt_server(&port);
    if (srv == -1)
    {
        cerr << "Error: Could not reserve port #" << port << endl;
        cerr << "EXITING" << endl;
        exit(1);
    }

    while (true)
    {
        listen(srv);
    }

    // Done; close server socket
    skt_close(srv);

    return 0;
}

