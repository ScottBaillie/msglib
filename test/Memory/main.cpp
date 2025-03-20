//////////////////////////////////////////////////////////////////////////////

#include <Memory/MemConnectionManager.h>
#include <Memory/IpcMem.h>
#include <Memory/IpcMutex.h>

#include <random>
#include <atomic>

//////////////////////////////////////////////////////////////////////////////

class Test1AConnectionHandler : public MemConnectionHandler
{
public:
	virtual ~Test1AConnectionHandler() {}

	virtual void onConnectionAccepted()
	{
		std::string s = "teststr";

		bool ok = sendMessage("buffer2", (uint8_t*)s.c_str(), s.size());
		if (!ok) std::cout << "Test1AConnectionHandler::onConnectionAccepted : Error from sendMessage()\n";
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(uint8_t * p, const uint64_t size)
	{
	}

	virtual void onTimer(const uint64_t time)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

const uint32_t MAX_COUNT = 500;
std::atomic<uint32_t> g_count = 0;

//////////////////////////////////////////////////////////////////////////////

class Test1BConnectionHandler : public MemConnectionHandler
{
public:
	virtual ~Test1BConnectionHandler() {}

	virtual void onConnectionAccepted()
	{
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(uint8_t * p, const uint64_t size)
	{
		g_count++;
		if (g_count == MAX_COUNT) shutdown();
	}

	virtual void onTimer(const uint64_t time)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

int
test1(int argc, char * argv[])
{
	bool ok;

	std::vector<std::vector<uint8_t>> buffer1(MAX_COUNT);
	for (auto & i : buffer1) i.resize(8192);
	std::vector<uint8_t> buffer2(8192);

	MemConnectionManager mgr(10);

	MemConnectionData d2;
	d2.m_hlr.reset(new Test1BConnectionHandler);
	d2.m_buffer.init(buffer2.data(), buffer2.size(),"buffer2");

	ok = mgr.add(d2);
	if (!ok) std::cout << "main : Error from add()\n";

	for (uint32_t u0=0; u0<MAX_COUNT; u0++) {
		MemConnectionData d1;
		d1.m_hlr.reset(new Test1AConnectionHandler);
		d1.m_buffer.init(buffer1[u0].data(), buffer1[u0].size(),"buffer1_"+std::to_string(u0));
		ok = mgr.add(d1);
		if (!ok) std::cout << "main : Error from add()\n";
	}

	while (true) {
		if (mgr.isStopped()) break;
	}

	std::cout << "main : g_count=" << g_count << "\n";;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

class Test2AConnectionHandler : public MemConnectionHandler
{
public:
	virtual ~Test2AConnectionHandler() {}

	virtual void onConnectionAccepted()
	{
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(uint8_t * p, const uint64_t size)
	{
	}

	virtual void onTimer(const uint64_t time)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

class Test2BConnectionHandler : public MemConnectionHandler
{
public:
	virtual ~Test2BConnectionHandler() {}

	virtual void onConnectionAccepted()
	{
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onMessageReceived(uint8_t * p, const uint64_t size)
	{
	}

	virtual void onTimer(const uint64_t time)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

int
test2(int argc, char * argv[])
{
	bool ok;

	std::vector<uint8_t> buffer1a(8192);
	std::vector<uint8_t> buffer1b(8192);
	std::vector<uint8_t> buffer2(8192);

	MemConnectionManager mgr(10);

	MemConnectionData d2;
	d2.m_hlr.reset(new Test2BConnectionHandler);
	d2.m_buffer.init(buffer2.data(), buffer2.size(),"buffer2");

	ok = mgr.add(d2);
	if (!ok) std::cout << "main : Error from add()\n";

	{
		MemConnectionData d1;
		d1.m_hlr.reset(new Test2AConnectionHandler);
		d1.m_buffer.init(buffer1a.data(), buffer1a.size(),"buffer1a");

		ok = mgr.add(d1);
		if (!ok) std::cout << "main : Error from add()\n";
	}

	{
		MemConnectionData d1;
		d1.m_hlr.reset(new Test2AConnectionHandler);
		d1.m_buffer.init(buffer1b.data(), buffer1b.size(),"buffer1b");

		ok = mgr.add(d1);
		if (!ok) std::cout << "main : Error from add()\n";
	}

	while (true) {
		if (mgr.isStopped()) break;
	}


	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret;

	IpcMem mem;
	IpcMutex mutex;


	ret = test1(argc,argv);
	ret = test2(argc,argv);
	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
