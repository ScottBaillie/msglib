//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_MEMCONNECTIONMANAGER_H
#define MSGLIB_MEMCONNECTIONMANAGER_H

//////////////////////////////////////////////////////////////////////////////

#include <FastQueue/FastQueue.h>
#include <Memory/MessageQueue.h>
#include <Memory/MsglibData.h>

#include <memory>
#include <string>
#include <mutex>

#include <unordered_map>
#include <thread>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class MemConnectionThread;

//////////////////////////////////////////////////////////////////////////////

class MemConnectionHandler
{
public:
	virtual ~MemConnectionHandler() {}

	virtual void onConnectionAccepted() = 0;

	virtual void onConnectionTerminated() = 0;

	virtual void onMessageReceived(uint8_t * p, const uint64_t size) = 0;

	virtual void onTimer(const uint64_t time) = 0;

	virtual void onUserData(MsglibDataPtr data) = 0;


	bool postUserData(std::shared_ptr<MemConnectionHandler> hlr, MsglibDataPtr data, const bool useMutex);

	bool sendMessage(const std::string & bufferName, uint8_t * p, const uint64_t size);

	void close();

	void shutdown();

	static bool sendMessage(uint8_t * buffer, uint8_t * p, const uint64_t size)
	{
		MessageQueue msgq(buffer);
		bool ok = msgq.add(p, size, false);
		return ok;
	}

	void setConnectionThread(MemConnectionThread * p)
	{
		m_connectionThread = p;
	}

public:
	std::string			m_name;

private:
	MemConnectionThread *		m_connectionThread = 0;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<MemConnectionHandler> MemConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

class MemBufferData
{
public:
	bool init(uint8_t * buffer, const uint64_t size, const std::string & name);
	bool init(uint8_t * buffer, const std::string & name);

public:
	std::string			m_name;
	uint8_t *			m_buffer = 0;

	// Set to true if initialised without a size.
	// If true, can only send messages, not receive.
	// Dont add this object to a thread, just store in MemConnectionManager.
	bool				m_writeOnly = false;
};

//////////////////////////////////////////////////////////////////////////////

class MemConnectionData {
public:
	MemConnectionHandlerPtr			m_hlr;
	MemBufferData				m_buffer;
};

//////////////////////////////////////////////////////////////////////////////

struct MemConnectionControlData
{
	std::string			m_name;
};

//////////////////////////////////////////////////////////////////////////////

class MemThreadTerminateHandler
{
public:
	virtual void onTerminate(pid_t tid) = 0;

	virtual void shutdown() = 0;

	virtual void close(const std::string & name) = 0;

	virtual bool sendMessage(const std::string & bufferName, uint8_t * p, const uint64_t size) = 0;
};

//////////////////////////////////////////////////////////////////////////////

class MemConnectionThread
{
public:
	MemConnectionThread();

	void	setTerminateHandler(MemThreadTerminateHandler * terminateHandler);

	bool	addConnection(const MemConnectionData & data);

	pid_t	gettid() const;

	void	stop(); // does not wait.

	size_t	size() const;

	//
	//

	void close(const std::string & name); // Called by MemConnectionHandler to close a connection.

	void shutdown(); // Called by MemConnectionHandler

	bool postUserData(MemConnectionHandlerPtr hlr, MsglibDataPtr data, const bool useMutex);

	bool sendMessage(const std::string & bufferName, uint8_t * p, const uint64_t size); // Called by MemConnectionHandler

private:
	struct UserDataQueueEntry
	{
		std::shared_ptr<MemConnectionHandler> hlr;
		MsglibDataPtr data;
	};

	void	start(); // called by constructor , waits until m_tid is known

	void	threadFunction();

	void	threadEventFunction(MemConnectionData * q, MemConnectionControlData * c, UserDataQueueEntry * u);
	void	addConnectionEvent(MemConnectionData & data);
	void	controlEvent(MemConnectionControlData & data);
	void	userdataEvent(UserDataQueueEntry & data);

private:
	std::mutex					m_mutex;
	std::unordered_map<std::string,MemConnectionData>	m_connectionMap;
	MemThreadTerminateHandler *			m_terminateHandler;
	size_t						m_size = 0;
	pid_t						m_tid = 0;
	FastQueue<MemConnectionData>			m_queue;
	FastQueue<MemConnectionControlData>		m_controlq;
	FastQueue<UserDataQueueEntry>			m_userdataq;
	bool						m_stopped = false;
	std::string					m_nextName;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<MemConnectionThread> MemConnectionThreadPtr;

//////////////////////////////////////////////////////////////////////////////

class MemConnectionManager : public MemThreadTerminateHandler
{
public:
	enum ThreadModel {
		DEFAULT,
		ONE_THREAD_PER_CONNECTION,
		N_THREADS_FOR_ALL_CONNECTIONS,
	};

	MemConnectionManager(const uint32_t maxThreads=0);

	~MemConnectionManager();

	bool	add(const MemConnectionData & data);

	bool	remove(const std::string & name);

	void	stop(); // does not wait.

	void	wait(); // wait until stopped.

	bool	isStopped() { return m_stoppedConfirmed;}

private:
	virtual void onTerminate(const pid_t tid);

	virtual void shutdown();

	virtual void close(const std::string & name);

	virtual bool sendMessage(const std::string & bufferName, uint8_t * p, const uint64_t size);

private:
	std::mutex						m_mutex;
	std::unordered_map<pid_t,MemConnectionThreadPtr>	m_threadList;
	std::unordered_map<std::string,MemBufferData>		m_nameToBufferMap;
	bool							m_stopped = false;
	bool							m_stoppedConfirmed = false;
	uint32_t						m_maxThreads = 0;
	pid_t							m_nextTid = 0;
};

//////////////////////////////////////////////////////////////////////////////

}

#endif
