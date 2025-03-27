//////////////////////////////////////////////////////////////////////////////

#include <Memory/MessageQueue.h>
#include <Memory/MemProtobufHandler.h>
#include <Memory/MemConnectionManager.h>
#include <Memory/IpcMem.h>
#include <Memory/IpcMutex.h>
#include <Memory/Alloc.h>
#include <Memory/Array.h>

using namespace msglib;

#include <atomic>
#include <iostream>

 #include <unistd.h>

#include "gen/test3.pb.h"

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

	virtual void onUserData(MsglibDataPtr data)
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

	virtual void onUserData(MsglibDataPtr data)
	{
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

	virtual void onUserData(MsglibDataPtr data)
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

	virtual void onUserData(MsglibDataPtr data)
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

class Test3aMemProtobufHandler : public MemProtobufHandler<UdpProtobufMsg>
{
public:
	virtual void onConnectionAccepted()
	{
		std::cout << "Test3aMemProtobufHandler::onConnectionAccepted : Entered\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test3aMemProtobufHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onProtobufMessageReceived()
	{
		std::cout << "Test3aMemProtobufHandler::onProtobufMessageReceived : str=" << m_msg.str_var_1() << " : int=" << m_msg.int_var_1() << "\n";

		bool ok = sendMessage("buffer_b", m_msg);
		if (!ok) {
			std::cout << "Test3aMemProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}
	}

	virtual void onUserData(MsglibDataPtr data)
	{
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test3aMemProtobufHandler::onTimer : Entered\n";
	}
};

//////////////////////////////////////////////////////////////////////////////

class Test3bMemProtobufHandler : public MemProtobufHandler<UdpProtobufMsg>
{
public:
	virtual void onConnectionAccepted()
	{
		std::cout << "Test3bMemProtobufHandler::onConnectionAccepted : Entered\n";

		UdpProtobufMsg msg;

		msg.set_str_var_1("protobuf_string");
		msg.set_int_var_1(1234567);

		bool ok = sendMessage("buffer_a", msg);
		if (!ok) {
			std::cout << "Test3bMemProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test3bMemProtobufHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onProtobufMessageReceived()
	{
		std::cout << "Test3bMemProtobufHandler::onProtobufMessageReceived : str=" << m_msg.str_var_1() << " : int=" << m_msg.int_var_1() << "\n";
	}

	virtual void onUserData(MsglibDataPtr data)
	{
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test3bMemProtobufHandler::onTimer : Entered\n";
	}
};

//////////////////////////////////////////////////////////////////////////////

int
test3(int argc, char * argv[])
{
	bool ok;

	std::vector<uint8_t> buffer_a(8192);
	std::vector<uint8_t> buffer_b(8192);

	MemConnectionManager mgr(10);

	MemConnectionData data_a;
	data_a.m_hlr.reset(new Test3aMemProtobufHandler);
	data_a.m_buffer.init(buffer_a.data(), buffer_a.size(),"buffer_a");

	ok = mgr.add(data_a);
	if (!ok) std::cout << "test3 : Error from add()\n";


	MemConnectionData data_b;
	data_b.m_hlr.reset(new Test3bMemProtobufHandler);
	data_b.m_buffer.init(buffer_b.data(), buffer_b.size(),"buffer_b");

	ok = mgr.add(data_b);
	if (!ok) std::cout << "test3 : Error from add()\n";

	::usleep(1000000*4);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

class Test4UserData : public MsglibData
{
public:
	int		m_int = 1234;
	std::string	m_str = "Test4-test-string";
};

//////////////////////////////////////////////////////////////////////////////

class Test4aMemProtobufHandler : public MemProtobufHandler<UdpProtobufMsg>
{
public:
	virtual void onConnectionAccepted()
	{
		std::cout << "Test4aMemProtobufHandler::onConnectionAccepted : Entered\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test4aMemProtobufHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onProtobufMessageReceived()
	{
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		Test4UserData & userdata = dynamic_cast<Test4UserData &>(*data);

		std::cout << "Test4aMemProtobufHandler::onUserData : " << userdata.m_str << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test4aMemProtobufHandler::onTimer : Entered\n";
	}
};

//////////////////////////////////////////////////////////////////////////////

class Test4bMemProtobufHandler : public MemProtobufHandler<UdpProtobufMsg>
{
public:
	Test4bMemProtobufHandler(MemConnectionHandlerPtr hlr)
		: m_hlr(hlr)
	{
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "Test4bMemProtobufHandler::onConnectionAccepted : Entered\n";

		MsglibDataPtr data(new Test4UserData);

		bool ok = m_hlr->postUserData(m_hlr, data, false);
		if (!ok) {
			std::cout << "Test4bMemProtobufHandler::onConnectionAccepted : Error from postUserData()\n";
		}
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test4bMemProtobufHandler::onConnectionTerminated : Entered\n";
	}

	virtual void onProtobufMessageReceived()
	{
	}

	virtual void onUserData(MsglibDataPtr data)
	{
		std::cout << "Test4bMemProtobufHandler::onUserData : Entered\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test4bMemProtobufHandler::onTimer : Entered\n";
	}

private:
	MemConnectionHandlerPtr m_hlr;
};

//////////////////////////////////////////////////////////////////////////////

int
test4(int argc, char * argv[])
{
	bool ok;

	MemConnectionManager mgr(10);

	MemConnectionHandlerPtr hlr_a(new Test4aMemProtobufHandler);
	MemConnectionHandlerPtr hlr_b(new Test4bMemProtobufHandler(hlr_a));

	MemConnectionData data_a;
	data_a.m_hlr = hlr_a;
	data_a.m_buffer.m_name = "Test4a"; // If buffer is not being used, only the name has to be set.

	ok = mgr.add(data_a);
	if (!ok) std::cout << "test4 : Error from add()\n";

	MemConnectionData data_b;
	data_b.m_hlr = hlr_b;
	data_b.m_buffer.m_name = "Test4b"; // If buffer is not being used, only the name has to be set.

	ok = mgr.add(data_b);
	if (!ok) std::cout << "test4 : Error from add()\n";

	::usleep(1000000*4);

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
//	ret = test2(argc,argv);
	ret = test3(argc,argv);
	ret = test4(argc,argv);

	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
