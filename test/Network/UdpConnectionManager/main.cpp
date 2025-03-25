//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>
#include <Network/UdpConnectionManager/UdpProtobufHandler.h>

using namespace msglib;

#include <iostream>

#include <signal.h>

#include "gen/test2.pb.h"

////////////////////////////////////////////////////////////////////////////// 

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

class Test1aUdpConnectionHandler : public UdpConnectionHandler
{
public:
	virtual void onConnectionAccepted()
	{
		std::cout << "Test1aUdpConnectionHandler::onConnectionAccepted : Entered\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test1aUdpConnectionHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		bool ok = sendMessage(peer, p, len);
		if (!ok) {std::cout << "Test1bUdpConnectionHandler::onConnectionAccepted : Error from sendMessage\n";return;}
		std::cout << "Test1aUdpConnectionHandler::onMessageReceived : len=" << len << "\n";
	}

	virtual void onError(int error)
	{
		std::cout << "Test1aUdpConnectionHandler::onError : Entered\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test1aUdpConnectionHandler::onTimer : Entered\n";
	}
};

////////////////////////////////////////////////////////////////////////////// 

class Test1bUdpConnectionHandler : public UdpConnectionHandler
{
public:
	virtual void onConnectionAccepted()
	{
		std::vector<uint8_t> buffer(512);
		IpPort ipPort;
		ipPort.addr.set("127.0.0.1");
		ipPort.setPort(50000);

		bool ok = sendMessage(ipPort, buffer.data(), buffer.size());
		if (!ok) {std::cout << "Test1bUdpConnectionHandler::onConnectionAccepted : Error from sendMessage\n";return;}

		std::cout << "Test1bUdpConnectionHandler::onConnectionAccepted : Entered\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test1bUdpConnectionHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		std::cout << "Test1bUdpConnectionHandler::onMessageReceived : len=" << len << "\n";
	}

	virtual void onError(int error)
	{
		std::cout << "Test1bUdpConnectionHandler::onError : Entered\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test1bUdpConnectionHandler::onTimer : Entered\n";
	}
};

////////////////////////////////////////////////////////////////////////////// 

int
test1(int argc, char * argv[])
{
	bool ok;
	UdpConnectionManager mgr(10);

	UdpConnectionData data_a;

	data_a.hlr.reset(new Test1aUdpConnectionHandler);
	data_a.ipPort.addr.set("127.0.0.1");
	data_a.ipPort.setPort(50000);

	ok = mgr.add(data_a);
	if (!ok) {std::cout << "test1 : Error from add\n";return 0;}

	UdpConnectionData data_b;

	data_b.hlr.reset(new Test1bUdpConnectionHandler);
	data_b.ipPort.addr.set("127.0.0.1");
	data_b.ipPort.setPort(50001);

	ok = mgr.add(data_b);
	if (!ok) {std::cout << "test1 : Error from add\n";return 0;}

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		::usleep(1000);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 

class Test2aUdpProtobufHandler : public UdpProtobufHandler<UdpProtobufMsg>
{
public:
	virtual void onConnectionAccepted()
	{
		std::cout << "Test2aUdpProtobufHandler::onConnectionAccepted : Entered\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test2aUdpProtobufHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onProtobufMessageReceived(const IpPort & peer)
	{
		std::cout << "Test2aUdpProtobufHandler::onProtobufMessageReceived : str=" << m_msg.str_var_1() << " : int=" << m_msg.int_var_1() << "\n";

		bool ok = sendMessage(peer, m_msg);
		if (!ok) {
			std::cout << "Test2aUdpProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}
	}

	virtual void onError(int error)
	{
		std::cout << "Test2aUdpProtobufHandler::onError : Entered\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test2aUdpProtobufHandler::onTimer : Entered\n";
	}
};

////////////////////////////////////////////////////////////////////////////// 


class Test2bUdpProtobufHandler : public UdpProtobufHandler<UdpProtobufMsg>
{
public:
	virtual void onConnectionAccepted()
	{
		std::cout << "Test2bUdpProtobufHandler::onConnectionAccepted : Entered\n";

		IpPort ipPort("127.0.0.1",50000);

		UdpProtobufMsg msg;

		msg.set_str_var_1("protobuf_string");
		msg.set_int_var_1(1234567);

		bool ok = sendMessage(ipPort, msg);
		if (!ok) {
			std::cout << "Test2bUdpProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test2bUdpProtobufHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onProtobufMessageReceived(const IpPort & peer)
	{
		std::cout << "Test2bUdpProtobufHandler::onProtobufMessageReceived : str=" << m_msg.str_var_1() << " : int=" << m_msg.int_var_1() << "\n";
	}

	virtual void onError(int error)
	{
		std::cout << "Test2bUdpProtobufHandler::onError : Entered\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test2bUdpProtobufHandler::onTimer : Entered\n";
	}
};

////////////////////////////////////////////////////////////////////////////// 

int
test2(int argc, char * argv[])
{
	bool ok;
	UdpConnectionManager mgr(10);

	UdpConnectionData data_a;

	data_a.hlr.reset(new Test2aUdpProtobufHandler);
	data_a.ipPort.addr.set("127.0.0.1");
	data_a.ipPort.setPort(50000);

	ok = mgr.add(data_a);
	if (!ok) {std::cout << "test2 : Error from add\n";return 0;}

	UdpConnectionData data_b;

	data_b.hlr.reset(new Test2bUdpProtobufHandler);
	data_b.ipPort.addr.set("127.0.0.1");
	data_b.ipPort.setPort(50001);

	ok = mgr.add(data_b);
	if (!ok) {std::cout << "test2 : Error from add\n";return 0;}

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		::usleep(1000);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 

int
main(int argc, char * argv[])
{
	int ret;
//	ret = test1(argc, argv);
	ret = test2(argc, argv);
	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
