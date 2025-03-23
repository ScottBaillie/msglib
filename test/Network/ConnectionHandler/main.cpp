//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/ProtobufHandler.h>

using namespace msglib;

#include "gen/test1.pb.h"

#include <atomic>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <random>
#include <vector>
#include <set>
#include <cstdlib>

#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

class Test1ServerProtobufHandler : public ProtobufHandler<Test1Message2>
{
public:
	virtual ~Test1ServerProtobufHandler() {}

	Test1ServerProtobufHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "Test1ServerProtobufHandler::onConnectionAccepted : fd=" << m_fd << "\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test1ServerProtobufHandler::onConnectionTerminated : fd=" << m_fd << "\n";
	}

	virtual void onProtobufMessageReceived()
	{
		std::cout << "Test1ServerProtobufHandler::onProtobufMessageReceived : message received : " << m_msg.string1() << " : " << m_msg.string2() << "\n";
	}

	virtual void onError(int error)
	{
		std::cout << "Test1ServerProtobufHandler::onError : fd=" << m_fd << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test1ServerProtobufHandler::onTimer : fd=" << m_fd << "\n";
	}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		std::shared_ptr<ConnectionHandler> ret(new Test1ServerProtobufHandler);
		return ret;
	}
};

//////////////////////////////////////////////////////////////////////////////
//
// The protobuf message is defined as follows :
//
//	message Test1Message { // sender uses this.
//	  int32 version = 1;
//	  string string1 = 2;
//	  string string2 = 3;
//	  int32 i1 = 4;
//	}
//
//	message Test1Message2 {	// receiver uses this.
//	  int32 version = 1;
//	  string string1 = 2;
//	  string string2 = 3;
//	  int32 i1 = 4;
//	  int32 i2 = 5;
//	}
//
// NOTE : if sender sends Test1Message2 and receiver gets Test1Message, this combination also works.
//
//////////////////////////////////////////////////////////////////////////////

class Test1ClientProtobufHandler : public ProtobufHandler<Test1Message>
{
public:
	Test1ClientProtobufHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual ~Test1ClientProtobufHandler() {}

	virtual void onConnectionAccepted()
	{
		std::cout << "Test1ClientProtobufHandler::onConnectionAccepted : fd=" << m_fd << "\n";

		Test1Message msg;

		msg.clear_string1();

		msg.set_string1("test1");
		msg.set_string2("test2");
		msg.set_i1(123);

		int32_t value = msg.i1();

  		const std::string& str = msg.string1();

		std::string* ms = msg.mutable_string1();

		bool ok = sendMessage(msg);
		if (!ok) {
			std::cout << "Test1ClientProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}

		msg.set_string1("abcdefg---111111111");
		ok = sendMessage(msg);
		if (!ok) {
			std::cout << "Test1ClientProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}

		msg.set_string1("abcdefg---2222222");
		ok = sendMessage(msg);
		if (!ok) {
			std::cout << "Test1ClientProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}

		msg.set_string1("abcdefg---33333333");
		ok = sendMessage(msg);
		if (!ok) {
			std::cout << "Test1ClientProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}

		msg.set_string1("abcdefg---44444444");
		ok = sendMessage(msg);
		if (!ok) {
			std::cout << "Test1ClientProtobufHandler::onConnectionAccepted : Error from sendMessage()\n";
		}
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "Test1ClientProtobufHandler::onConnectionTerminated : fd=" << m_fd << "\n";
	}

	virtual void onProtobufMessageReceived()
	{
		std::cout << "Test1ClientProtobufHandler::onProtobufMessageReceived : message received : " << m_msg.string1() << " : " << m_msg.string2() << "\n";
	}

	virtual void onError(int error)
	{
		std::cout << "Test1ClientProtobufHandler::onError : fd=" << m_fd << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "Test1ClientProtobufHandler::onTimer : fd=" << m_fd << "\n";
	}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		return std::shared_ptr<ConnectionHandler>();
	}
};

//////////////////////////////////////////////////////////////////////////////

int
test1(int argc, char * argv[])
{
	int ret = 0;
	bool ok;
	ConnectionManager mgr;
	ConnectionData data;

	data.ipPort.port = ::htons(50000);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new Test1ServerProtobufHandler);
	data.server = true;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	::usleep(1000000*1);

	data.ipPort.port = ::htons(50000);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new Test1ClientProtobufHandler);
	data.server = false;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	::usleep(1000000*6);

	mgr.stop();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret = test1(argc,argv);
	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
