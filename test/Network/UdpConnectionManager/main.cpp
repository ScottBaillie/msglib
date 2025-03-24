//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>

using namespace msglib;

#include <iostream>

#include <signal.h>

////////////////////////////////////////////////////////////////////////////// 

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

class Test1UdpConnectionHandler : public UdpConnectionHandler
{
public:
	virtual ~Test1UdpConnectionHandler() {}

	virtual void onConnectionAccepted()
	{
		std::cout << "Test1UdpConnectionHandler::onConnectionAccepted : Entered\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test1UdpConnectionHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		std::cout << "Test1UdpConnectionHandler::onMessageReceived : len=" << len << "\n";
	}

	virtual void onError(int error)
	{
		std::cout << "Test1UdpConnectionHandler::onError : Entered\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test1UdpConnectionHandler::onTimer : Entered\n";
	}
};

////////////////////////////////////////////////////////////////////////////// 

int
test1(int argc, char * argv[])
{
	UdpConnectionManager mgr(10);

	UdpConnectionData data;

	data.hlr.reset(new Test1UdpConnectionHandler);
	data.ipPort.addr.set("127.0.0.1");
	data.ipPort.setPort(50000);

	bool ok = mgr.add(data);
	if (!ok) {std::cout << "test1 : Error from add\n";return 0;}

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	std::vector<uint8_t> buffer(512);
	IpPort ipPort;
	ipPort.addr.set("127.0.0.1");
	ipPort.setPort(50000);

	ok = UdpConnectionHandler::sendMessage(ipPort, buffer.data(), buffer.size());
	if (!ok) {std::cout << "test1 : Error from sendMessage\n";return 0;}

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
	ret = test1(argc, argv);
	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
