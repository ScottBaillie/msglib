//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>

using namespace msglib;

#include <atomic>
#include <iostream>
#include <random>

//////////////////////////////////////////////////////////////////////////////

class MyServerConnectionHandler : public ConnectionHandler
{
public:
	MyServerConnectionHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "MyServerConnectionHandler::onConnectionAccepted() : fd=" << m_fd << " : this=" << this << "\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "MyServerConnectionHandler::onConnectionTerminated() : fd=" << m_fd << " : this=" << this << "\n";
	}

	virtual void onDataReceived()
	{
		std::cout << "MyServerConnectionHandler::onDataReceived() : fd=" << m_fd << " : this=" << this << "\n";

		m_buffer.advanceRead(m_buffer.getReadSize());

		std::string s = "123";
		write((uint8_t*)s.c_str(), s.size());
	}

	virtual void onError(int error)
	{
		std::cout << "MyServerConnectionHandler::onError() : fd=" << m_fd << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "MyServerConnectionHandler::onTimer() : fd=" << m_fd << " : this=" << this << "\n";
	}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		std::shared_ptr<ConnectionHandler> ret(new MyServerConnectionHandler);
		ret->m_ipPort = m_ipPort;
		ret->m_server = m_server;
		ret->m_fd = m_fd;
		return(ret);
	}
};

//////////////////////////////////////////////////////////////////////////////

class MyClientConnectionHandler : public ConnectionHandler
{
public:
	MyClientConnectionHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "MyClientConnectionHandler::onConnectionAccepted() : fd=" << m_fd << " : this=" << this << "\n";

		std::string s = "abc";
		write((uint8_t*)s.c_str(), s.size());
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "MyClientConnectionHandler::onConnectionTerminated() : fd=" << m_fd << " : this=" << this << "\n";
	}

	virtual void onDataReceived()
	{
		std::cout << "MyClientConnectionHandler::onDataReceived() : fd=" << m_fd << " : this=" << this << "\n";

		m_buffer.advanceRead(m_buffer.getReadSize());
//		close();
	}

	virtual void onError(int error)
	{
		std::cout << "MyClientConnectionHandler::onError() : fd=" << m_fd << " : error=" << error << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "MyClientConnectionHandler::onTimer() : fd=" << m_fd << " : this=" << this << "\n";
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
	data.hlr = ConnectionHandlerPtr(new MyServerConnectionHandler);
	data.server = true;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	::usleep(1000000*4);

	data.ipPort.port = ::htons(50000);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new MyClientConnectionHandler);
	data.server = false;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	::usleep(1000000*4);

	data.ipPort.port = ::htons(50000);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new MyClientConnectionHandler);
	data.server = false;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	::usleep(1000000*4);

	data.ipPort.port = ::htons(50001);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new MyServerConnectionHandler);
	data.server = true;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	::usleep(1000000*4);

	data.ipPort.port = ::htons(50001);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new MyClientConnectionHandler);
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

class Test2ServerConnectionHandler : public ConnectionHandler
{
public:
	Test2ServerConnectionHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onDataReceived()
	{
		if (m_buffer.getReadSize()<256) return;
		uint8_t * p = m_buffer.getReadPtr();

		uint8_t checksum = 0;

		for (unsigned u0=0; u0<256;u0++) {
			checksum = checksum + p[u0];
		}

		write(&checksum,1);

		m_buffer.advanceRead(256);
	}

	virtual void onError(int error)
	{
	}

	virtual void onTimer(uint64_t time)
	{
	}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		std::shared_ptr<ConnectionHandler> ret(new Test2ServerConnectionHandler);
		ret->m_ipPort = m_ipPort;
		ret->m_server = m_server;
		ret->m_fd = m_fd;
		return(ret);
	}
};

//////////////////////////////////////////////////////////////////////////////

const unsigned NCONNECTION = 1024;

std::atomic<unsigned> matchCount = 0;

//////////////////////////////////////////////////////////////////////////////

class Test2ClientConnectionHandler : public ConnectionHandler
{
public:
	Test2ClientConnectionHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
		std::vector<uint8_t> buffer(256);
		for (unsigned u0=0; u0<256;u0++) {
			buffer[u0] = rand() % 256;
			m_checksum = m_checksum + buffer[u0];
		}
		write((uint8_t*)buffer.data(), buffer.size());
	}

	virtual void onConnectionTerminated()
	{
	}

	virtual void onDataReceived()
	{
		uint8_t * p = m_buffer.getReadPtr();
		if (*p != m_checksum) {
			std::cout << "Test2ClientConnectionHandler::onDataReceived : Checksum does not match\n";
		} else {
			matchCount++;
		}
		m_buffer.advanceRead(m_buffer.getReadSize());
		close();
		if (matchCount==NCONNECTION) {
			std::cout << "Test2ClientConnectionHandler::onDataReceived : shutdown\n";
			shutdown();
		}
	}

	virtual void onError(int error)
	{
	}

	virtual void onTimer(uint64_t time)
	{
	}

private:
	uint8_t		m_checksum = 0;
};

//////////////////////////////////////////////////////////////////////////////

int
test2(int argc, char * argv[])
{
	int ret = 0;
	bool ok;
	ConnectionManager mgr(10,10);
	ConnectionData data;

	data.ipPort.port = ::htons(50000);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new Test2ServerConnectionHandler);
	data.server = true;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	std::cout << "Server started 127.0.0.1:50000\n";

	::usleep(1000000);

	std::cout << "Starting clients\n";

	for (unsigned u0=0; u0<NCONNECTION;u0++) {
		data.ipPort.port = ::htons(50000);
		data.ipPort.addr.set("127.0.0.1");
		data.hlr = ConnectionHandlerPtr(new Test2ClientConnectionHandler);
		data.server = false;

		ok = mgr.add(data);
		if (!ok) {
			std::cout << "Error from mgr.add()\n";
			return 0;
		}
		::usleep(1000*10);
	}

	std::cout << NCONNECTION << " clients started 127.0.0.1:50000\n";

	mgr.wait();

	std::cout << "matchCount=" << matchCount << "\n";

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

class Test3ServerConnectionHandler : public ConnectionHandler
{
public:
	Test3ServerConnectionHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "onConnectionAccepted : " << m_fd << "\n";
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "onConnectionTerminated : " << m_fd << "\n";
	}

	virtual void onDataReceived()
	{
		std::cout << "onDataReceived : " << m_fd << "\n";
		m_buffer.advanceRead(m_buffer.getReadSize());
		close();
	}

	virtual void onError(int error)
	{
		std::cout << "onError : " << m_fd << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "onTimer : " << m_fd << "\n";
	}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		std::shared_ptr<ConnectionHandler> ret(new Test3ServerConnectionHandler);
		ret->m_ipPort = m_ipPort;
		ret->m_server = m_server;
		ret->m_fd = m_fd;
		return(ret);
	}
};

//////////////////////////////////////////////////////////////////////////////

class Test3ClientConnectionHandler : public ConnectionHandler
{
public:
	Test3ClientConnectionHandler()
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted()
	{
		std::cout << "onConnectionAccepted : " << m_fd << "\n";
		std::string s = "123";
		write((uint8_t*)s.c_str(), s.size());
	}

	virtual void onConnectionTerminated()
	{
		std::cout << "onConnectionTerminated : " << m_fd << "\n";
	}

	virtual void onDataReceived()
	{
		std::cout << "onDataReceived : " << m_fd << "\n";
		m_buffer.advanceRead(m_buffer.getReadSize());
	}

	virtual void onError(int error)
	{
		std::cout << "onError : " << m_fd << "\n";
	}

	virtual void onTimer(uint64_t time)
	{
		std::cout << "onTimer : " << m_fd << "\n";
	}
};

////////////////////////////////////////////////////////////////////////////// 

int
test3(int argc, char * argv[])
{
	int ret = 0;
	bool ok;
	ConnectionManager mgr(10,10);
	ConnectionData data;

	data.ipPort.port = ::htons(50000);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new Test3ServerConnectionHandler);
	data.server = true;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	std::cout << "Server started 127.0.0.1:50000\n";

	::usleep(1000000);

	data.ipPort.port = ::htons(50000);
	data.ipPort.addr.set("127.0.0.1");
	data.hlr = ConnectionHandlerPtr(new Test3ClientConnectionHandler);
	data.server = false;

	ok = mgr.add(data);
	if (!ok) {
		std::cout << "Error from mgr.add()\n";
		return 0;
	}

	std::cout << "Client started, Sleeping ...\n";

	::usleep(1000000*8);

	std::cout << "Stopping mgr\n";

	mgr.stop();

	std::cout << "mgr stopped\n";

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 

int
main(int argc, char * argv[])
{
//	test1(argc, argv);
	test2(argc, argv);
//	test3(argc, argv);
	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
