//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_THREADPOOL_H
#define MSGLIB_THREADPOOL_H

//////////////////////////////////////////////////////////////////////////////

#include <Memory/MsglibData.h>
#include <FastQueue/FastQueue.h>

#include <memory>
#include <thread>
#include <vector>
#include <mutex>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class ThreadPool;

//////////////////////////////////////////////////////////////////////////////

class ThreadConnectionHandler
{
public:
	virtual ~ThreadConnectionHandler() {}

	virtual void onUserData(MsglibDataPtr data) = 0;

	void shutdown();

	void setThreadPool(ThreadPool * p)
	{
		m_threadPool = p;
	}

private:
	ThreadPool *	m_threadPool = 0;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<ThreadConnectionHandler> ThreadConnectionHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

class ThreadPool
{
public:
	ThreadPool(const uint32_t maxThreads=1);

	~ThreadPool();

	void	stop(); // does not wait.

	void	wait(); // wait until stopped.

	bool postUserData(ThreadConnectionHandlerPtr hlr, MsglibDataPtr data, const bool useMutex);

private:
	struct UserDataQueueEntry
	{
		ThreadConnectionHandlerPtr hlr;
		MsglibDataPtr data;
	};

	void	threadFunction(const uint32_t index);

	void	threadEventFunction(UserDataQueueEntry * u);
	void	userdataEvent(UserDataQueueEntry & data);


private:
	typedef std::shared_ptr<std::thread> ThreadPtr;
	typedef std::shared_ptr<FastQueue<UserDataQueueEntry>> FastQueuePtr;

	std::mutex				m_mutex;
	std::vector<ThreadPtr>			m_threadPool;
	std::vector<FastQueuePtr>		m_userdataList;
	bool					m_stopped = false;
	uint32_t				m_maxThreads = 0;
};

//////////////////////////////////////////////////////////////////////////////

inline void
ThreadConnectionHandler::shutdown()
{
	if (m_threadPool) m_threadPool->stop();
}

//////////////////////////////////////////////////////////////////////////////

}

#endif
