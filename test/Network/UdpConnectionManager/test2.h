//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>
#include <Network/UdpConnectionManager/UdpConnectionManager.h>
#include <Network/Command/CommandServer.h>
#include <String/Tokenizer.h>

using namespace msglib;

#include <atomic>
#include <iostream>
#include <random>
#include <unordered_map>

//////////////////////////////////////////////////////////////////////////////

class TcpUserData : public MsglibData
{
public:
	std::vector<uint8_t>	m_buffer;
	bool			m_close = false;
};

//////////////////////////////////////////////////////////////////////////////

class TcpServerConnectionHandler : public ConnectionHandler
{
public:
	TcpServerConnectionHandler()
	{
		m_buffer.setSize(8*1024*1024);
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "TCP Server Connection Accepted : " << m_ipPort.getString() << "\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "TCP Server Connection Terminated : " << m_ipPort.getString() << "\n";
	}

	virtual void onDataReceived()
	{
		std::cout << "TCP Server Data Received : " << m_ipPort.getString() << " : readSize=" << m_buffer.getReadSize() << "\n";

		m_buffer.advanceRead(m_buffer.getReadSize());
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		TcpUserData & ud = dynamic_cast<TcpUserData &>(*data);

		std::cout << "TCP Server posted message received : " << m_ipPort.getString() << "\n";
	}

	virtual void onError(int error)
	{
		std::cout << "TCP Server Connection Error : " << m_ipPort.getString() << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
	}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		std::shared_ptr<ConnectionHandler> ret(new TcpServerConnectionHandler);
		ret->m_ipPort = m_ipPort;
		ret->m_server = m_server;
		ret->m_fd = m_fd;
		return(ret);
	}
};

//////////////////////////////////////////////////////////////////////////////

class TcpClientConnectionHandler : public ConnectionHandler
{
public:
	TcpClientConnectionHandler()
	{
		m_buffer.setSize(8*1024*1024);
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "TCP Client Connection Accepted : " << m_ipPort.getString() << "\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "TCP Client Connection Terminated : " << m_ipPort.getString() << "\n";
	}

	virtual void onDataReceived()
	{
		std::cout << "TCP Client Data Received : " << m_ipPort.getString() << " : readSize=" << m_buffer.getReadSize() << "\n";

		m_buffer.advanceRead(m_buffer.getReadSize());
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		TcpUserData & ud = dynamic_cast<TcpUserData &>(*data);

		if (ud.m_close) {
			std::cout << "TCP Client posted message received (close) : " << m_ipPort.getString() << "\n";
			close();
		} else {
			std::cout << "TCP Client posted message received (send) : " << m_ipPort.getString() << "\n";
			write(ud.m_buffer.data(), ud.m_buffer.size());
		}
	}

	virtual void onError(int error)
	{
		std::cout << "TCP Client Connection Error : " << m_ipPort.getString() << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<TcpServerConnectionHandler> TcpServerConnectionHandlerPtr;
typedef std::shared_ptr<TcpClientConnectionHandler> TcpClientConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

class UdpUserData : public MsglibData
{
public:
	UdpUserData() {}

	UdpUserData(const std::string & inaddr, const uint16_t port)
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
		std::cout << "UDP Connection Accepted : " << m_ipPort.getString() << "\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "UDP Connection Terminated : " << m_ipPort.getString() << "\n";
	}

	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		std::cout << "UDP Packet received by : " << m_ipPort.getString() << " : From : " << peer.getString() << " : Length=" << len << "\n";
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		UdpUserData & ud = dynamic_cast<UdpUserData &>(*data);
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
		std::cout << "UDP Connection Error : " << m_ipPort.getString() << "\n";
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
		: m_tcpmgr(10,10)
		, m_udpmgr(10)
	{
	}

	int main(int argc, char * argv[]);

	virtual std::vector<std::string> runCommand(const std::string & command);

private:
	void tcpserver_add(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void tcpclient_add(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void tcpclient_send(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void tcpserver_close(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void tcpclient_close(std::vector<std::string> & tokens, std::vector<std::string> & response);

	void udpserver_add(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void udpserver_close(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void list(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void udpserver_send(std::vector<std::string> & tokens, std::vector<std::string> & response);
	void udpclient_sendanon(std::vector<std::string> & tokens, std::vector<std::string> & response);

private:
	CommandServer							m_commandServer;
	ConnectionManager						m_tcpmgr;
	std::unordered_map<IpPort,TcpServerConnectionHandlerPtr>	m_tcpservermap;
	std::unordered_map<IpPort,TcpClientConnectionHandlerPtr>	m_tcpclientmap;
	UdpConnectionManager						m_udpmgr;
	std::unordered_map<IpPort,Test2UdpConnectionHandlerPtr>		m_udphlrmap;
};

//////////////////////////////////////////////////////////////////////////////
