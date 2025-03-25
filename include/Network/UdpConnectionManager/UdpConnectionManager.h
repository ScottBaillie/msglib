//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_UDPCONNECTIONMANAGER_H
#define MSGLIB_UDPCONNECTIONMANAGER_H

//////////////////////////////////////////////////////////////////////////////

#include <FastQueue/FastQueue.h>
#include <Network/IpAddress/IpAddress.h>

#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <iostream>

#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class UdpConnectionThread;

//////////////////////////////////////////////////////////////////////////////

class UdpConnectionHandler
{
public:
	virtual ~UdpConnectionHandler() {}

	virtual void onConnectionAccepted() = 0;

	virtual void onConnectionTerminated() = 0;

	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len) = 0;

	virtual void onError(int error) = 0;

	virtual void onTimer(uint64_t time) = 0;

	void close();

	void shutdown();

	bool sendMessage(const IpPort & ipPort, uint8_t * p, const size_t len);

	static bool sendMessage(const int fd, const IpPort & ipPort, uint8_t * p, const size_t len);
	static bool sendMessageAnon(const IpPort & ipPort, uint8_t * p, const size_t len);

	void setConnectionThread(UdpConnectionThread * p)
	{
		m_connectionThread = p;
	}

public:
	IpPort			m_ipPort;
	int			m_fd = 0;
	std::vector<uint8_t>	m_buffer;

private:
	UdpConnectionThread *	m_connectionThread = 0;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<UdpConnectionHandler> UdpConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

struct UdpConnectionData {
	IpPort			ipPort;
	UdpConnectionHandlerPtr	hlr;
	int			fd = 0;
};

//////////////////////////////////////////////////////////////////////////////

struct UdpConnectionControlData
{
	int fd = 0;
};

//////////////////////////////////////////////////////////////////////////////

class UdpThreadTerminateHandler
{
public:
	virtual void onTerminate(pid_t tid) = 0;

	virtual void shutdown() = 0;
};

//////////////////////////////////////////////////////////////////////////////

class UdpConnectionThread
{
public:
	UdpConnectionThread();

	void	setTerminateHandler(UdpThreadTerminateHandler * terminateHandler);

	bool	addConnection(const UdpConnectionData & data);

	pid_t	gettid() const;

	void	stop(); // does not wait.

	size_t	size() const;

	void close(const int fd);

	void shutdown();

private:
	void	start(); // called by constructor , waits until m_tid is known

	// polls all file desc
	void	threadFunction();

	void	threadEventFunction(UdpConnectionData * q, UdpConnectionControlData * c, const int ret);
	void	addConnectionEvent(UdpConnectionData & data);
	void	fileDescChangedEvent(const int ret);
	void	controlEvent(UdpConnectionControlData & data);

private:
	std::mutex					m_mutex;
	std::unordered_map<int,UdpConnectionData>	m_connectionMap;
	UdpThreadTerminateHandler *			m_terminateHandler;
	size_t						m_size = 0;
	pid_t						m_tid = 0;
	FastQueue<UdpConnectionData>			m_queue;
	FastQueue<UdpConnectionControlData>		m_controlq;
	std::vector<pollfd>				m_fdset;
	int						m_timerFd = 0;
	bool						m_stopped = false;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<UdpConnectionThread> UdpConnectionThreadPtr;

//////////////////////////////////////////////////////////////////////////////

class UdpConnectionManager : public UdpThreadTerminateHandler
{
public:
	enum ThreadModel {
		DEFAULT,
		ONE_THREAD_PER_CONNECTION,
		N_THREADS_FOR_ALL_CONNECTIONS,
	};

	UdpConnectionManager(const uint32_t maxThreads=0);

	~UdpConnectionManager();

	bool	add(UdpConnectionData & data);

	bool	remove(const int fd);

	void	stop(); // does not wait.

	void	wait(); // wait until stopped.

	bool	isStopped() { return m_stoppedConfirmed;}

private:
	virtual void onTerminate(pid_t tid);

	virtual void shutdown();

private:
	std::mutex						m_mutex;
	std::unordered_map<pid_t,UdpConnectionThreadPtr>	m_threadList;
	bool							m_stopped = false;
	bool							m_stoppedConfirmed = false;
	uint32_t						m_maxThreads = 0;
	pid_t							m_nextTid = 0;
};

//////////////////////////////////////////////////////////////////////////////

void erase_fd(std::vector<pollfd> & fdset, const int fd);

//////////////////////////////////////////////////////////////////////////////

}

#endif
