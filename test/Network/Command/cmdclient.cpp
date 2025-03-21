//////////////////////////////////////////////////////////////////////////////

#include <String/Tokenizer.h>
#include <Network/Command/CommandClient.h>

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <random>
#include <vector>
#include <set>

#include <cstdlib>

#include <signal.h>

//////////////////////////////////////////////////////////////////////////////
//
// cmdlient <ipAddr> <port> <request>
//
//////////////////////////////////////////////////////////////////////////////

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	if (argc<4) {
		std::cout << "Wrong args : Usage : cmdlient <ipAddr> <port> <arg1> <arg2> <arg3> ...\n";
		return 0;
	}

	int ret = 0;
	std::string request;

	for (int i=3; i<argc; i++) {
		request += argv[i];
		if (i!=(argc-1)) request += " ";
	}

	IpPort ipPort;
	ipPort.setPort(::atoi(argv[2]));
	ipPort.addr.set(argv[1]);

	CommandClient cmdclient;
	cmdclient.start(request, ipPort);

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		if (cmdclient.isStopped()) break;
		::usleep(1000);
	}

	cmdclient.stop();

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
