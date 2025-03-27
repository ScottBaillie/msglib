//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>

using namespace msglib;

//////////////////////////////////////////////////////////////////////////////

ConnectionThread::ConnectionThread()
	: m_queue(1024)
	, m_controlq(1024)
	, m_userdataq(1024)
{
	start();
}

////////////////////////////////////////////////////////////////////////////// 

ConnectionThread::~ConnectionThread()
{
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionThread::setTerminateHandler(ThreadTerminateHandler * terminateHandler)
{
	m_terminateHandler = terminateHandler;
}

////////////////////////////////////////////////////////////////////////////// 

bool
ConnectionThread::addConnection(const ConnectionData & data)
{
	if (m_stopped) return false;
	ConnectionData * q = m_queue.next();
	*q = data;
	bool ok = m_queue.add();
	return ok;
}

////////////////////////////////////////////////////////////////////////////// 

pid_t
ConnectionThread::gettid() const
{
	return m_tid;
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionThread::stop() // does not wait
{
	m_stopped = true;
}

////////////////////////////////////////////////////////////////////////////// 

size_t
ConnectionThread::size() const
{
	return m_size;
}

////////////////////////////////////////////////////////////////////////////// 

bool
ConnectionThread::postUserData(ConnectionHandlerPtr hlr, MsglibDataPtr data, const bool useMutex)
{
	std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
	if (useMutex) lock.lock();
	if (m_stopped) return false;
	UserDataQueueEntry * q = m_userdataq.next();
	q->hlr = hlr;
	q->data = data;
	bool ok = m_userdataq.add();
	return ok;
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionThread::start() // called by constructor , waits until m_tid is know
{
	std::thread t(&ConnectionThread::threadFunction,this);
	t.detach();

	for (unsigned u0=0;u0<4000;u0++) {
		if (m_tid != 0) return;
		::usleep(1000);
	}
	std::cout << "ConnectionThread::start : Timed out waiting for thread to start\n";
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionThread::threadFunction()
{
	m_tid = ::gettid();

	ConnectionData * q;
	ConnectionControlData * c;
	UserDataQueueEntry * u;
	int ret;
	unsigned sleep_count = 0;
	bool qempty = false;

	while (!m_stopped) {

		q = m_queue.get();
		c = m_controlq.get();
		u = m_userdataq.get();
		if ((q==0)&&(c==0)&&(u==0)) qempty = true; else qempty = false;

		ret = 0;
		if (qempty && (m_fdset.size() > 0)) {
			ret = ::poll(&m_fdset[0], m_fdset.size(), 1);
			if (ret == -1) {
				std::cout << "ConnectionThread::threadFunction : Error from poll()\n";
			}
		}

		if (qempty && (ret<=0)) {
			if (m_fdset.size() == 0) ::usleep(1000);
			sleep_count++;
			const unsigned SLEEP_COUNT_MAX = 1000;
			if (sleep_count < SLEEP_COUNT_MAX) continue;
			sleep_count = 0;
			if (m_connectionMap.size() == 0) continue;

			auto i = m_connectionMap.find(m_timerFd);
			if (i==m_connectionMap.end()) i = m_connectionMap.begin();
			const unsigned TIMER_BATCH_SIZE = 4;
			for (unsigned u0=0; u0<TIMER_BATCH_SIZE; u0++) {
				i->second.hlr->onTimer(0);
				i++;
				if (i==m_connectionMap.end()) {
					m_timerFd = m_connectionMap.begin()->first;
					break;
				}
				m_timerFd = i->first;
			}

			continue;
		}

		threadEventFunction(q, c, u, ret);

		if (q) m_queue.release();
		if (c) m_controlq.release();
		if (u) m_userdataq.release();
	}

	for (auto i : m_connectionMap) {
		i.second.hlr->onConnectionTerminated();
		::close(i.first);
	}

	if (m_terminateHandler) m_terminateHandler->onTerminate(m_tid);
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionThread::close(const int fd)
{
	ConnectionControlData * q = m_controlq.next();
	q->fd = fd;
	bool ok = m_controlq.add();
	if (!ok) std::cout << "ConnectionThread::close : Error from add()\n";
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionThread::shutdown()
{
	if (m_terminateHandler) m_terminateHandler->shutdown();
}

////////////////////////////////////////////////////////////////////////////// 

bool
ConnectionHandler::postUserData(ConnectionHandlerPtr hlr, MsglibDataPtr data, const bool useMutex)
{
	if (m_connectionThread) return m_connectionThread->postUserData(hlr, data, useMutex);
	return false;
}

void
ConnectionHandler::close()
{
	if (!m_connectionThread) return;
	m_connectionThread->close(m_fd);
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionHandler::shutdown()
{
	if (!m_connectionThread) return;
	m_connectionThread->shutdown();
}

////////////////////////////////////////////////////////////////////////////// 
