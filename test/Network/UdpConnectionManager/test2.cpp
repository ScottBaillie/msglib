//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>
#include <Network/Command/CommandServer.h>
#include <String/Tokenizer.h>

using namespace msglib;

#include <atomic>
#include <iostream>
#include <random>
#include <unordered_map>

#include <signal.h>

//////////////////////////////////////////////////////////////////////////////

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

class TestUserData : public MsglibData
{
public:
	TestUserData() {}

	TestUserData(const std::string & inaddr, const uint16_t port)
		: ipPort(inaddr, port)
	{
	}

public:
	IpPort ipPort;
};

//////////////////////////////////////////////////////////////////////////////

class Test2UdpConnectionHandler : public UdpConnectionHandler
{
public:
	virtual void onConnectionAccepted()
	{
		std::cout << "Listening on : " << m_ipPort.getString() << "\n";
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		std::cout << "UDP Packet received by : " << m_ipPort.getString() << " : From : " << peer.getString() << " : Length=" << len << "\n";
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		TestUserData & ud = dynamic_cast<TestUserData &>(*data);
		std::vector<uint8_t> buffer(512);
		bool ok = sendMessage(ud.ipPort, buffer.data(), buffer.size());
		if (ok) {
			std::cout << "UDP Packet sent by : " << m_ipPort.getString() << " : To : " << ud.ipPort.getString() << " : Length=" << buffer.size() << "\n";
		} else {
			std::cout << "Test2UdpConnectionHandler::onUserData : Error from sendMessage\n";
		}
	}

	virtual void onError(int error)
	{
		std::cout << "Error on socket : " << m_ipPort.getString() << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<Test2UdpConnectionHandler> Test2UdpConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

class App : public RunCommand
{
public:
	App()
		: m_mgr(10)
	{
	}

	int main(int argc, char * argv[]);

	virtual std::vector<std::string> runCommand(const std::string & command);

private:
	CommandServer							m_commandServer;
	UdpConnectionManager						m_mgr;
	std::unordered_map<IpPort,Test2UdpConnectionHandlerPtr>		m_hlrmap;
};

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
		response.push_back("add <ipaddress> <port>");
		response.push_back("del <ipaddress> <port>");
		response.push_back("list");
		response.push_back("send <lipaddress> <lport> <ripaddress> <rport>");
		response.push_back("sendanon <ipaddress> <port>");
		return response;
	}

	if (tokens[0]=="add") {
		if (tokens.size() != 3) {
			response.push_back("Incorrect arguments : " + command);
			return response;
		}

		Test2UdpConnectionHandlerPtr hlr(new Test2UdpConnectionHandler);

		UdpConnectionData data;

		data.hlr = hlr;
		ok = data.ipPort.addr.set(tokens[1]);
		if (!ok) {
			response.push_back("Invalid IP address");
			return response;
		}
		data.ipPort.setPort(::atoi(tokens[2].c_str()));

		ok = m_mgr.add(data);
		if (!ok) {
			response.push_back("Error adding entry : " + data.ipPort.getString());
			return response;
		}

		m_hlrmap[data.ipPort] = hlr;

		response.push_back("OK");

		return response;
	}

	if (tokens[0]=="del") {
		if (tokens.size() != 3) {
			response.push_back("Incorrect arguments : " + command);
			return response;
		}

		IpPort ipPort;

		ok = ipPort.addr.set(tokens[1]);
		if (!ok) {
			response.push_back("Invalid IP address");
			return response;
		}
		ipPort.setPort(::atoi(tokens[2].c_str()));

		if (m_hlrmap.find(ipPort) == m_hlrmap.end()) {
			response.push_back("Entry not found");
			return response;
		}

		m_mgr.remove(m_hlrmap[ipPort]->m_fd);

		m_hlrmap.erase(ipPort);

		response.push_back("OK");

		return response;
	}

	if (tokens[0]=="list") {
		if (tokens.size() != 1) {
			response.push_back("Incorrect arguments : " + command);
			return response;
		}

		for (auto i : m_hlrmap) {
			response.push_back("Listening : " + i.first.getString());
		}

		response.push_back("OK");

		return response;
	}

	if (tokens[0]=="send") {
		if (tokens.size() != 5) {
			response.push_back("Incorrect arguments : " + command);
			return response;
		}

		IpPort lipPort;
		IpPort ripPort;

		ok = lipPort.addr.set(tokens[1]);
		if (!ok) {
			response.push_back("Invalid IP address");
			return response;
		}
		lipPort.setPort(::atoi(tokens[2].c_str()));

		ok = ripPort.addr.set(tokens[3]);
		if (!ok) {
			response.push_back("Invalid IP address");
			return response;
		}
		ripPort.setPort(::atoi(tokens[4].c_str()));

		if (m_hlrmap.find(lipPort) == m_hlrmap.end()) {
			response.push_back("Entry not found");
			return response;
		}

		std::shared_ptr<TestUserData> ud(new TestUserData);

		ud->ipPort = ripPort;

		m_hlrmap[lipPort]->postUserData(m_hlrmap[lipPort], ud, false);

		response.push_back("OK");

		return response;
	}

	if (tokens[0]=="sendanon") {
		if (tokens.size() != 3) {
			response.push_back("Incorrect arguments : " + command);
			return response;
		}

		IpPort ipPort;

		ok = ipPort.addr.set(tokens[1]);
		if (!ok) {
			response.push_back("Invalid IP address");
			return response;
		}
		ipPort.setPort(::atoi(tokens[2].c_str()));

		std::vector<uint8_t> buffer(128);

		ok = UdpConnectionHandler::sendMessageAnon(ipPort, buffer.data(), buffer.size());
		if (!ok) {
			response.push_back("Error sending packet");
			return response;
		}

		response.push_back("OK");

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
