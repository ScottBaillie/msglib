//////////////////////////////////////////////////////////////////////////////

#include <Thread/ThreadPool.h>

using namespace msglib;

#include <iostream>

#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(const uint32_t maxThreads)
	: m_threadPool(maxThreads)
	, m_userdataList(maxThreads)
	, m_maxThreads(maxThreads)
{
}

//////////////////////////////////////////////////////////////////////////////

ThreadPool::~ThreadPool()
{
	stop();
	wait();
}

//////////////////////////////////////////////////////////////////////////////

void
ThreadPool::stop()
{
	m_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

void
ThreadPool::wait()
{
	for (auto & i : m_threadPool) {
		if (!i) continue;
		i->join();
		i.reset();
	}
}

//////////////////////////////////////////////////////////////////////////////

bool
ThreadPool::postUserData(ThreadConnectionHandlerPtr hlr, MsglibDataPtr data, const bool useMutex)
{
	std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
	if (useMutex) lock.lock();
	if (m_stopped) return false;

	uint64_t index = data->getHash() % m_maxThreads;

	FastQueuePtr & ptr = m_userdataList[index];

	if (!ptr) {
		ptr.reset(new FastQueue<UserDataQueueEntry>(1024));
		ThreadPtr & tptr = m_threadPool[index];
		tptr.reset(new std::thread(&ThreadPool::threadFunction,this,index));
	}

	FastQueue<UserDataQueueEntry> & userdataq = *ptr;

	UserDataQueueEntry * q = userdataq.next();
	q->hlr = hlr;
	q->hlr->setThreadPool(this);
	q->data = data;
	bool ok = userdataq.add();
	return ok;
}

//////////////////////////////////////////////////////////////////////////////

void
ThreadPool::threadFunction(const uint32_t index)
{
	UserDataQueueEntry * u;
	bool qempty = false;

	FastQueue<UserDataQueueEntry> & userdataq = *m_userdataList[index];

	while (!m_stopped) {

		u = userdataq.get();
		if (u==0) qempty = true; else qempty = false;

		if (qempty) {
			::usleep(1000);
			continue;
		}

		threadEventFunction(u);

		if (u) userdataq.release();
	}
}

//////////////////////////////////////////////////////////////////////////////

void
ThreadPool::threadEventFunction(UserDataQueueEntry * u)
{
	if (u) {userdataEvent(*u);}
}

//////////////////////////////////////////////////////////////////////////////

void
ThreadPool::userdataEvent(UserDataQueueEntry & data)
{
	data.hlr->onUserData(data.data);
	data.hlr.reset();
	data.data.reset();
}

//////////////////////////////////////////////////////////////////////////////

