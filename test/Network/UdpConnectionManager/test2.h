//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>
#include <Network/Command/CommandServer.h>
#include <String/Tokenizer.h>

using namespace msglib;

#include <atomic>
#include <iostream>
#include <random>
#include <unordered_map>

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
	void udpserver_add(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void udpserver_close(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void list(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void udpserver_send(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void udpclient_sendanon(std::vector<std::string> & tokens, std::vector<std::string> & response);

private:
	CommandServer							m_commandServer;
	UdpConnectionManager						m_mgr;
	std::unordered_map<IpPort,Test2UdpConnectionHandlerPtr>		m_hlrmap;
};

//////////////////////////////////////////////////////////////////////////////
