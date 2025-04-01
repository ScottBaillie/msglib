//////////////////////////////////////////////////////////////////////////////

#include <Memory/MemConnectionManager.h>

using namespace msglib;

#include <atomic>
#include <iostream>
#include <random>

#include <signal.h>
#include <unistd.h>

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
	char		name[64];
	uint8_t		data[256];
};

//////////////////////////////////////////////////////////////////////////////

void
initMsg(uint64_t seq, std::string & name, Msg & m)
{
	m.seq = seq;
	uint64_t sum = 0;
	for (uint32_t u0=0; u0<256; u0++) {
		m.data[u0] = ::rand() % 256;
		sum += m.data[u0];
	}
	m.sum = sum;
	for (uint32_t u0=0; u0<name.size(); u0++) m.name[u0] = name[u0];
	m.name[name.size()] = 0;
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

	TestUserData(const std::string & iname)
		: name(iname)
	{
	}

public:
	std::string name;
};

//////////////////////////////////////////////////////////////////////////////

class TestConnectionHandler : public MemConnectionHandler
{
public:
	virtual ~TestConnectionHandler() {}

	virtual void onConnectionAccepted()
	{
		m_accepted = true;
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(uint8_t * p, const uint64_t size)
	{
		if (!m_server) g_msgcount++;

		Msg * pm = (Msg *)p;

		if ((pm->seq > NUM_MSG) && (m_server==false)) {
			close();
			return;
		}

		checkMsg(pm);

		Msg m;
		initMsg(pm->seq+1, m_name, m);
		bool ok = sendMessage(pm->name, (uint8_t*)&m, sizeof(m));

		if (!ok) {
			std::cout << "TestConnectionHandler::onMessageReceived : Error from sendMessage()\n";
		}
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		TestUserData & ud = dynamic_cast<TestUserData &>(*data);

		Msg m;
		initMsg(0, m_name, m);
		bool ok = sendMessage(ud.name, (uint8_t*)&m, sizeof(m));
		if (!ok) std::cout << "TestConnectionHandler::onUserData : Error from sendMessage\n";

		m_server = false;
	}

	virtual void onTimer(const uint64_t time)
	{
	}

public:
	bool	m_server = true;
	bool	m_accepted = false;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<TestConnectionHandler> TestConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	bool ok;
	std::vector<TestConnectionHandlerPtr> shlrList(NUM_SERVER);
	std::vector<TestConnectionHandlerPtr> chlrList(NUM_CLIENT);

	std::vector<std::vector<uint8_t>> sbuffer(NUM_SERVER);
	std::vector<std::vector<uint8_t>> cbuffer(NUM_CLIENT);
	for (auto & i : sbuffer) i.resize(65536);
	for (auto & i : cbuffer) i.resize(65536);

	MemConnectionManager mgr(10);

	for (uint32_t u0=0; u0<NUM_SERVER; u0++) {
		MemConnectionData data;
		shlrList[u0].reset(new TestConnectionHandler);
		data.m_hlr = shlrList[u0];
		data.m_buffer.init(sbuffer[u0].data(), sbuffer[u0].size(),"sbuffer"+std::to_string(u0));
		ok = mgr.add(data);
		if (!ok) std::cout << "main : Error from add()\n";
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
		MemConnectionData data;
		chlrList[u0].reset(new TestConnectionHandler);
		data.m_hlr = chlrList[u0];
		data.m_buffer.init(cbuffer[u0].data(), cbuffer[u0].size(),"cbuffer"+std::to_string(u0));
		ok = mgr.add(data);
		if (!ok) std::cout << "main : Error from add()\n";
	}

	while (true) {
		for (u0=0; u0<NUM_CLIENT; u0++) {
			if (!chlrList[u0]->m_accepted) break;
		}
		if (u0==NUM_CLIENT) break;
		::usleep(1000);
	}

	for (uint32_t u0=0; u0<NUM_CLIENT; u0++) {
		MsglibDataPtr mdata(new TestUserData("sbuffer"+std::to_string(u0%NUM_SERVER)));
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
