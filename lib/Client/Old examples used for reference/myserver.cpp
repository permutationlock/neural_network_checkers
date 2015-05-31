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
