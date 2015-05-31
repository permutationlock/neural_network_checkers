// tcp_server2.cpp
// Glenn G. Chappell
// 17 Apr 2013
//
// For CS 321 Spring 2013
// Threaded HTTP Server

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
#include <thread>
using std::thread;
#include <mutex>
using std::mutex;

unsigned int port = 8888;  // Port to listen on
// Note: We chose a number above 1024 here,
// to avoid getting in trouble with the OS

int howManyConnects = 0;   // How many connections so far
mutex hmc_lock;            // Lock for above

int numThreads = 0;        // Number of threads running now
mutex nt_lock;             // Lock for above

const int MAX_THREADS = 100; // Maximum number of slave threads at once



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
        if (buffer.substr(0, 15) == "Content-Length:")
        {
            istringstream is(buffer.substr(15));
            is >> conlen;
        }
    }
    while (!buffer.empty());

    // Read body of response, if any, and print it
    if (conlen == 0)
    {
        cout << "Body is empty" << endl;
    }
    else
    {
        buffer.resize(conlen, '?');
        retval = skt_recvN(s, &buffer[0], conlen);
        if (retval == -1)
        {
            cerr << "EXITING" << endl;
            exit(1);
        }
        cout << "Body:" << endl;

        // To print the whole body, replace the 5 lines below
        //  with the commented-out line.
        cout << buffer.substr(0,100) << endl;
        cout << endl;
        cout << "NOTE: To avoid filling up your screen, I have only\n";
        cout << "printed the first 100 characters of the body. You\n";
        cout << "can change this if you wish; see the source code.\n";
        //cout << buffer << endl;
    }

    // Send response (assume message was HTTP GET)
    cout << "Sending response" << endl;

    // Increment connection number & get our own copy of it
    hmc_lock.lock();
    ++howManyConnects;
    int connNum = howManyConnects;  // Local copy of howManyConnects
    hmc_lock.unlock();

    // Make an HTML webpage
    ostringstream oss;
    oss << connNum;
    string body = "<!DOCTYPE html><html><head>"
        "<meta http-equiv='Content-Type' "
          "content='text/html; charset=utf-8'>"
        "<title>A Web Page!</title></head>"
        "<body><h1>A Web Page!</h1>"
        "<p>Sent by a groovy multi-threaded server"
        "<p>You are connection number: "
        + oss.str() + "</body></html>";
    ostringstream oss2;
    oss2 << body.size();  // Get the Content-Length right
    string header = "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + oss2.str() + "\r\n"
        "\r\n";

    // Send the webpage
    retval = skt_sendN(s, (header+body).c_str(), (header+body).size());
    if (retval == -1)
    {
        cerr << "Error: could not send response" << endl;
        cerr << "Closing connection" << endl;
        skt_close(s);
        nt_lock.lock();
        --numThreads;
        nt_lock.unlock();
        return;
    }

    // Done; close socket
    cerr << "Success" << endl;
    cerr << "Closing connection" << endl;
    skt_close(s);

    nt_lock.lock();
    --numThreads;
    nt_lock.unlock();
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

    while (true)
    {
        nt_lock.lock();
        if (numThreads < MAX_THREADS)
            break;
        nt_lock.unlock();
    }
    ++numThreads;
    nt_lock.unlock();
    
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

