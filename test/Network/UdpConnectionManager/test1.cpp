//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>

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

class Test1UdpConnectionHandler : public UdpConnectionHandler
{
public:
	virtual void onConnectionAccepted()
	{
		m_accepted = true;
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		if (!m_server) g_msgcount++;

		Msg * pm = (Msg *)p;

		if ((pm->seq > NUM_MSG) && (m_server==false)) {
			close();
			return;
		}

		checkMsg(pm);

		Msg m;
		initMsg(pm->seq+1, m);
		bool ok = sendMessage(peer, (uint8_t*)&m, sizeof(m));

		if (!ok) {
			std::cout << "Test1UdpConnectionHandler::onMessageReceived : Error from sendMessage()\n";
		}
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		TestUserData & ud = dynamic_cast<TestUserData &>(*data);

		Msg m;
		initMsg(0, m);
		bool ok = sendMessage(ud.ipPort, (uint8_t*)&m, sizeof(m));
		if (!ok) std::cout << "Test1UdpConnectionHandler::onUserData : Error from sendMessage\n";

		m_server = false;
	}

	virtual void onError(int error)
	{
	}

	virtual void onTimer(uint64_t time)
	{
	}

public:
	bool	m_server = true;
	bool	m_accepted = false;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<Test1UdpConnectionHandler> Test1UdpConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	bool ok;
	std::vector<Test1UdpConnectionHandlerPtr> shlrList(NUM_SERVER);
	std::vector<Test1UdpConnectionHandlerPtr> chlrList(NUM_CLIENT);
	UdpConnectionManager mgr(10);

	for (uint32_t u0=0; u0<NUM_SERVER; u0++) {
		UdpConnectionData data;

		shlrList[u0].reset(new Test1UdpConnectionHandler);
		data.hlr = shlrList[u0];
		data.ipPort.addr.set("127.0.0.1");
		data.ipPort.setPort(50000+u0);

		ok = mgr.add(data);
		if (!ok) {std::cout << "main : Error from add\n";return 0;}
	}

	uint32_t u0;
	while (true) {
		for (u0=0; u0<NUM_SERVER; u0++) {
			if (!shlrList[u0]->m_accepted) break;
		}
		if (u0==NUM_SERVER) break;
		::usleep(1000);
	}

	for (uint32_t u0=0; u0<NUM_CLIENT; u0++) {
		UdpConnectionData data;

		chlrList[u0].reset(new Test1UdpConnectionHandler);
		data.hlr = chlrList[u0];
		data.ipPort.addr.set("127.0.0.1");
		data.ipPort.setPort(50000+u0+NUM_SERVER);

		ok = mgr.add(data);
		if (!ok) {std::cout << "main : Error from add\n";return 0;}
	}

	while (true) {
		for (u0=0; u0<NUM_CLIENT; u0++) {
			if (!chlrList[u0]->m_accepted) break;
		}
		if (u0==NUM_CLIENT) break;
		::usleep(1000);
	}

	for (uint32_t u0=0; u0<NUM_CLIENT; u0++) {
		MsglibDataPtr mdata(new TestUserData("127.0.0.1",50000+(u0%NUM_SERVER)));
		chlrList[u0]->postUserData(chlrList[u0], mdata, false);
	}

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		::usleep(1000);
		if (g_msgcount == (NUM_CLIENT*(1+(NUM_MSG/2))) ) break;
	}

	std::cout << "main : g_msgcount=" << g_msgcount << "\n";
	if (g_msgcount == (NUM_CLIENT*(1+(NUM_MSG/2))) ) std::cout << "main : OK\n";

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
