//////////////////////////////////////////////////////////////////////////////

#include <FastQueue/FastQueue.h>
#include <LinkedList/LinkedList.h>
#include <Network/Buffer/Buffer.h>
#include <Network/IpAddress/IpAddress.h>

#include <memory>
#include <string>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

class ConnectionThread;

//////////////////////////////////////////////////////////////////////////////

class ConnectionHandler
{
public:
	virtual ~ConnectionHandler() {}

	virtual void onConnectionAccepted() = 0;

	virtual void onConnectionTerminated() = 0;

	virtual void onDataReceived() = 0;

	virtual void onError(int error) = 0;

	virtual void onTimer(uint64_t time) = 0;

	virtual std::shared_ptr<ConnectionHandler> clone() {return(std::shared_ptr<ConnectionHandler>());}

	void close();

	void shutdown();

	bool write(uint8_t * p, size_t len)
	{
		ssize_t ret = ::write(m_fd, p, len);
		if (ret==-1) {
			std::cout << "ConnectionHandler::write : Error from write\n";
			return false;
		}
		return true;
	}

	void setConnectionThread(ConnectionThread * p)
	{
		m_connectionThread = p;
	}

public:
	IpPort		m_ipPort;	  // Set by ConnectionManager before callbacks called.
	bool		m_server = false; // Set by ConnectionManager before callbacks called.
	int		m_fd = 0;	  // Set by ConnectionManager before callbacks called.
	Buffer		m_buffer;	  // written to by ConnectionManager before onDataReceived() called.

private:
	ConnectionThread *	m_connectionThread = 0;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<ConnectionHandler> ConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

struct ConnectionData {
	IpPort			ipPort;
	ConnectionHandlerPtr	hlr;
	bool			server = false;
	bool			clientConnected = false;	// Set by ConnectionManager.
	int			fd = 0;				// Set by ConnectionManager.
};

//////////////////////////////////////////////////////////////////////////////

struct ConnectionControlData
{
	int fd = 0;
};

//////////////////////////////////////////////////////////////////////////////

struct ServerData {
	IpPort			ipPort;
	ConnectionHandlerPtr	hlr;
};

//////////////////////////////////////////////////////////////////////////////

class ThreadTerminateHandler
{
public:
	virtual void onTerminate(pid_t tid) = 0;

	virtual void shutdown() = 0;
};

//////////////////////////////////////////////////////////////////////////////

class ConnectionThread
{
public:
	ConnectionThread();
	~ConnectionThread();

	void	setTerminateHandler(ThreadTerminateHandler * terminateHandler);

	bool	addConnection(const ConnectionData & data);

	pid_t	gettid() const;

	void	stop(); // does not wait.

	size_t	size() const;

	void close(const int fd); // Called by ConnectionHandler to close a connection.

	void shutdown();

private:
	void	start(); // called by constructor , waits until m_tid is known

	// polls all file desc
	void	threadFunction();

	void	threadEventFunction(ConnectionData * q, ConnectionControlData * c, const int ret);
	void	addServerEvent(ConnectionData & data);
	void	addClientEvent(ConnectionData & data);
	void	fileDescChangedEvent(const int ret);
	void	controlEvent(ConnectionControlData & data);

private:
	std::unordered_map<int,ConnectionData>		m_connectionMap;
	ThreadTerminateHandler *			m_terminateHandler;
	size_t						m_size = 0;
	pid_t						m_tid = 0;
	FastQueue<ConnectionData>			m_queue;
	FastQueue<ConnectionControlData>		m_controlq;
	std::vector<pollfd>				m_fdset;
	int						m_timerFd = 0;
	bool						m_stopped = false;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<ConnectionThread> ConnectionThreadPtr;

//////////////////////////////////////////////////////////////////////////////

class ConnectionManager : public ThreadTerminateHandler
{
public:
	enum ThreadModel {
		DEFAULT,
		ONE_THREAD_PER_CONNECTION,
		N_THREADS_FOR_ALL_CONNECTIONS,
	};

	ConnectionManager(const uint32_t serverThreads=0, const uint32_t clentThreads=0);

	~ConnectionManager();

	bool	add(const ConnectionData & data);

	bool	removeServer(const IpPort & ipPort);

	void	stop(); // does not wait.

	void	wait(); // wait until stopped.

	bool	isStopped() { return m_stoppedConfirmed;}

private:
	void	start(); // called by constructor

	// polls accept()
	void	threadFunction();

	void	threadEventFunction(ConnectionData * q, IpPort * r, const int ret);
	void	addServerEvent(ConnectionData & data);
	void	addClientEvent(ConnectionData & data);
	void	fileDescChangedEvent(const int ret);
	void	removeServerEvent(const IpPort & ipPort);

	virtual void onTerminate(pid_t tid);

	virtual void shutdown();

private:
	std::mutex						m_mutex;
	std::unordered_map<pid_t,ConnectionThreadPtr>		m_serverThreadList;
	std::unordered_map<pid_t,ConnectionThreadPtr>		m_clientThreadList;
	std::unordered_map<int,ServerData>			m_serverMap;
	std::unique_ptr<std::thread>				m_thread;
	FastQueue<ConnectionData>				m_queue;
	FastQueue<IpPort>					m_rqueue;
	std::vector<pollfd>					m_fdset;
	bool							m_stopped = false;
	bool							m_stoppedConfirmed = false;
	uint32_t						m_serverThreads = 0;
	uint32_t						m_clientThreads = 0;
	pid_t							m_serverNextTid = 0;
	pid_t							m_clientNextTid = 0;
};

//////////////////////////////////////////////////////////////////////////////

void erase_fd(std::vector<pollfd> & fdset, const int fd);

//////////////////////////////////////////////////////////////////////////////

