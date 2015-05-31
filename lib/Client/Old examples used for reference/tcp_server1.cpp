// tcp_server1.cpp
// VERSION 2
// Glenn G. Chappell
// 17 Apr 2013
//
// For CS 321 Spring 2013
// Simple HTTP Server

#include "osl/socket.h"
// Note: THIS PROGRAM CANNOT BE RUN ON NETRUN,
// since it requires too much time

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

unsigned int port = 8888;  // Port to listen on
// Note: We chose a number above 1024 here,
// to avoid getting in trouble with the OS

int howManyConnects = 0;   // How many connections so far



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
                       const char *term=" \t\r\n")
{
    char c; 
    theString = "";
    while (true) {
        int retval = skt_recvN(skt, &c, 1); // Grab next character
        if (retval != 0) return retval;
        if (strchr(term, c))
        {
            if (c=='\r') continue;  // will be CR/LF; wait for LF
            else return 0;          // Hit terminator-- stop.
        }
        else theString += c; // normal char; add to string, continue
    }
}


void listen(SERVER_SOCKET srv)
{
    int retval;              // For return values of skt funcs
    string buffer;           // Holds chars received over socket

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
    
    // Got connection; print received message
    cout << "Connection received: " << endl;
    cerr << "IP: " << int(client_ip.data[0]) << "."
                   << int(client_ip.data[1]) << "."
                   << int(client_ip.data[2]) << "."
                   << int(client_ip.data[3]) << " - ";
    cerr << "PORT: " << client_port << endl;
    do
    {
        retval = my_skt_recv_string(buffer, s, "\r\n");
        if (retval == -1)
        {
            // Read error; give up (but don't crash!)
            skt_close(s);
            return;
        }
        cout << "Header line: " << buffer << endl;
    }
    while (!buffer.empty());

    // Send response (assume message was HTTP GET)
    cout << "Sending response" << endl;

    // Make return message
    ++howManyConnects;
    ostringstream oss;
    oss << howManyConnects;
    string body = "V " + key[i] + " " + value[i];
    ostringstream oss2;
    oss2 << body.size();  // Get the Content-Length right
    //Wasn't sure what to do with the header portion
    string header = "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + oss2.str() + "\r\n"
        "\r\n";

    // Send the command
    retval = skt_sendN(s, (header+body).c_str(), (header+body).size());
    if (retval == -1)
    {
        cerr << "Error: could not send response" << endl;
        cerr << "Closing connection" << endl;
        skt_close(s);
        return;
    }

    // Done; close socket
    cerr << "Success" << endl;
    cerr << "Closing connection" << endl;
    skt_close(s);
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

