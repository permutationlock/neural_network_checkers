// tcp_client1.cpp
// Glenn G. Chappell
// 12 Apr 2013
//
// For CS 321 Spring 2013
// HTTP Client with Error Checking

#include "osl/socket.h"
// TO RUN THIS UNDER NETRUN, uncomment the following line
//#include "osl/socket.cpp"

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
                       const char *term="")
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


int main()
{
    int retval;              // For return values of skt funcs
    string buffer;           // Holds chars received over socket

    // Set skt abort function to mine (above)
    skt_set_abort(my_skt_abort);

    unsigned int port = 8888;  // Standard HTTP port
    skt_ip_t ip = { 127, 0, 0, 1 };
                             // Use this form to specify the IP no.
    if (skt_ip_match(ip, _skt_invalid_ip))
    {
        cerr << "EXITING" << endl;
        exit(1);
    }

    // Try to connect with the give IP & port number
    SOCKET s = skt_connect(ip, port, 5);
                             // 5-second timeout
    if (s == -1)
    {
        cerr << "EXITING" << endl;
        exit(1);
    }

    string msg =
        "G myid thevalue";              // HTTP request to send

    while(true){
	    cout << "Sending request" << endl;
	    // Send message
	    retval = skt_sendN(s, msg.c_str(), msg.size()+1);
	    if (retval == -1)
	    {
		cerr << "EXITING" << endl;
		exit(1);
	    }

	    cout << "Printing response" << endl << endl;

	    //Reads body of TCP packet
	    do
	    {/*
		retval = my_skt_recv_string(buffer, s, "");
		if (retval == -1)
		{
		    // Read error; give up (but don't crash!)
		    skt_close(s);
		    return 0;
		}
		cout << buffer << " ";*/
	    }
	    while (true);
	    cout << endl;
    }

    // Done; close socket
    skt_close(s);

    return 0;
}

