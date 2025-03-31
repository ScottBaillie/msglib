//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>
#include <Network/ConnectionHandler/MessageHandler.h>

using namespace msglib;

#include <atomic>
#include <iostream>
#include <random>

#include <signal.h>

//////////////////////////////////////////////////////////////////////////////

const uint32_t NUM_SERVER = 50;
const uint32_t NUM_CLIENT = 500;
const uint32_t NUM_MSG = 100;

//////////////////////////////////////////////////////////////////////////////

std::atomic<uint64_t> g_msgcount = 0;
std::atomic<uint64_t> g_count = 0;

//////////////////////////////////////////////////////////////////////////////

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

struct Msg
{
	uint64_t	seq = 0;
	uint64_t	sum = 0;
	uint8_t		data[256];
};

//////////////////////////////////////////////////////////////////////////////

void
initMsg(uint64_t seq, Msg & m)
{
	m.seq = seq;
	uint64_t sum = 0;
	for (uint32_t u0=0; u0<256; u0++) {
		m.data[u0] = ::rand() % 256;
		sum += m.data[u0];
	}
	m.sum = sum;
}

//////////////////////////////////////////////////////////////////////////////

void
checkMsg(Msg * pm)
{
	uint64_t sum = 0;
	for (uint32_t u0=0; u0<256; u0++) {
		sum += pm->data[u0];
	}
	if (sum != pm->sum) {
		std::cout << "checkMsg : Checksum does not match\n";
	}
}

//////////////////////////////////////////////////////////////////////////////

class ServerMessageHandler : public MessageHandler
{
public:
	ServerMessageHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(uint8_t * p, const size_t len)
	{
		Msg * pm = (Msg *)p;

		checkMsg(pm);

		Msg m;
		initMsg(pm->seq+1, m);
		bool ok = sendMessage((uint8_t*)&m, sizeof(m));

		if (!ok) {
			std::cout << "ServerMessageHandler::onMessageReceived : Error from sendMessage()\n";
		}
	}

	virtual void onUserData(MsglibDataPtr data)
	{
	}

	virtual void onError(int error)
	{
	}

	virtual void onTimer(uint64_t time)
	{
	}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		std::shared_ptr<ConnectionHandler> ret(new ServerMessageHandler);
		ret->m_ipPort = m_ipPort;
		ret->m_server = m_server;
		ret->m_fd = m_fd;
		return(ret);
	}
};

//////////////////////////////////////////////////////////////////////////////

class ClientMessageHandler : public MessageHandler
{
public:
	ClientMessageHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
		Msg m;
		initMsg(0, m);
		bool ok = sendMessage((uint8_t*)&m, sizeof(m));

		if (!ok) {
			std::cout << "ClientMessageHandler::onConnectionAccepted : Error from sendMessage()\n";
		}
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(uint8_t * p, const size_t len)
	{
		g_msgcount++;

		Msg * pm = (Msg *)p;

		if (pm->seq > NUM_MSG) {
			close();
			g_count++;
			return;
		}

		checkMsg(pm);

		Msg m;
		initMsg(pm->seq+1, m);
		bool ok = sendMessage((uint8_t*)&m, sizeof(m));

		if (!ok) {
			std::cout << "ClientMessageHandler::onMessageReceived : Error from sendMessage()\n";
		}
	}

	virtual void onUserData(MsglibDataPtr data)
	{
	}

	virtual void onError(int error)
	{
	}

	virtual void onTimer(uint64_t time)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret = 0;
	bool ok;
	uint16_t port;
	ConnectionManager mgr(10,10);

	for (uint32_t u0=0; u0<NUM_SERVER; u0++) {
		ConnectionData data;
		data.ipPort.setPort(50000+u0);
		data.ipPort.addr.set("127.0.0.1");
		data.hlr = ConnectionHandlerPtr(new ServerMessageHandler);
		data.server = true;

		ok = mgr.add(data);
		if (!ok) {
			std::cout << "main : Error from mgr.add()\n";
			return 0;
		}
	}

	for (uint32_t u0=0; u0<NUM_CLIENT; u0++) {
		ConnectionData data;
		port = 50000 + (u0%NUM_SERVER);
		data.ipPort.setPort(port);
		data.ipPort.addr.set("127.0.0.1");
		data.hlr = ConnectionHandlerPtr(new ClientMessageHandler);
		data.server = false;

		ok = mgr.add(data);
		if (!ok) {
			std::cout << "main : Error from mgr.add()\n";
			return 0;
		}

		::usleep(1000);
	}

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		::usleep(1000);
		if (g_count == NUM_CLIENT) break;
	}

	std::cout << "main : g_count=" << g_count << " : g_msgcount=" << g_msgcount << "\n";
	if (g_count==NUM_CLIENT) std::cout << "main : OK\n";

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
