//////////////////////////////////////////////////////////////////////////////

#include "test2.h"

#include <signal.h>

//////////////////////////////////////////////////////////////////////////////

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

int
App::main(int argc, char * argv[])
{
	bool ok;

	IpPort ipPort;
	ipPort.setPort(::atoi(argv[2]));
	ipPort.addr.set(argv[1]);

	m_commandServer.start(*this, ipPort);

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		::usleep(1000);
	}

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

	if (!ok || (tokens.size()==0)) {
		response.push_back("Error parsing command line : " + command);
		return response;
	}

	if (tokens[0]=="help") {
		response.push_back("Available Commands :");
		response.push_back("tcpserver-add <ipaddress> <port>");
		response.push_back("tcpclient-add <ipaddress> <port>");
		response.push_back("tcpclient-send <ipaddress> <port>");
		response.push_back("tcpserver-close <ipaddress> <port>");
		response.push_back("tcpclient-close <ipaddress> <port>");
		response.push_back("udpserver-add <ipaddress> <port>");
		response.push_back("udpserver-close <ipaddress> <port>");
		response.push_back("udpserver-send <lipaddress> <lport> <ripaddress> <rport>");
		response.push_back("udpclient-sendanon <ipaddress> <port>");
		response.push_back("list");
		return response;
	}

	if (tokens[0]=="udpserver-add") {
		udpserver_add(tokens, response);
		return response;
	}

	if (tokens[0]=="udpserver-close") {
		udpserver_close(tokens, response);
		return response;
	}

	if (tokens[0]=="list") {
		list(tokens, response);
		return response;
	}

	if (tokens[0]=="udpserver-send") {
		udpserver_send(tokens, response);
		return response;
	}

	if (tokens[0]=="udpclient-sendanon") {
		udpclient_sendanon(tokens, response);
		return response;
	}

	response.push_back("Unknown command");

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
