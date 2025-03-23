//////////////////////////////////////////////////////////////////////////////

#include <String/Tokenizer.h>
#include <Network/Command/CommandServer.h>

using namespace msglib;

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
// testapp <ipAddr> <port>
//
//////////////////////////////////////////////////////////////////////////////

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

class App : public RunCommand
{
public:
	int main(int argc, char * argv[]);

	virtual std::vector<std::string> runCommand(const std::string & command);

private:
	CommandServer	m_commandServer;
	unsigned	m_counter = 0;
};

//////////////////////////////////////////////////////////////////////////////

int App::main(int argc, char * argv[])
{
	if (argc!=3) {
		std::cout << "Wrong args : Usage : testapp <ipAddr> <port>\n";
		return 0;
	}

	IpPort ipPort;
	ipPort.setPort(::atoi(argv[2]));
	ipPort.addr.set(argv[1]);

	std::cout << "Starting Command Server\n";

	m_commandServer.start(*this, ipPort);

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		m_counter++;
		usleep(1000);
	}

	std::cout << "Stopping Command Server\n";

	m_commandServer.stop();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> App::runCommand(const std::string & command)
{
	Tokenizer t;
	std::vector<std::string> tokens;
	std::vector<std::string> response;

	bool ok = t.tokenize(command, tokens);

	if (tokens[0]=="help") {
		response.push_back("Usage : command arg1 arg2");
		return response;
	}

	std::cout << "command=" << command << "\n";

	std::string s = "counter=" + std::to_string(m_counter);

	response.push_back("line1");
	response.push_back("line2");
	response.push_back(s);

	return response;
}

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	App app;
	return app.main(argc, argv);
}

////////////////////////////////////////////////////////////////////////////// 
