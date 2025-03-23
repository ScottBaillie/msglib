//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>

using namespace msglib;

//////////////////////////////////////////////////////////////////////////////

ConnectionManager::ConnectionManager(const uint32_t serverThreads, const uint32_t clentThreads)
	: m_queue(1024)
	, m_rqueue(1024)
	, m_serverThreads(serverThreads)
	, m_clientThreads(clentThreads)
{
	start();
}

////////////////////////////////////////////////////////////////////////////// 

ConnectionManager::~ConnectionManager()
{
	stop();
	wait();
}

////////////////////////////////////////////////////////////////////////////// 

bool
ConnectionManager::add(const ConnectionData & data)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	if (m_stopped) return false;
	ConnectionData * q = m_queue.next();
	*q = data;
	bool ok = m_queue.add();
	return ok;
}

////////////////////////////////////////////////////////////////////////////// 

bool
ConnectionManager::removeServer(const IpPort & ipPort)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	if (m_stopped) return false;
	IpPort * q = m_rqueue.next();
	*q = ipPort;
	bool ok = m_rqueue.add();
	return ok;
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionManager::stop()
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	m_stopped = true;
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionManager::wait()
{
	if (!m_thread) return;
	m_thread->join();
	m_thread.reset();

	unsigned u0;
	for (u0=0;u0<10000;u0++) {
		if (m_stoppedConfirmed) break;
		::usleep(1000);
	}
	if (u0==10000) std::cout << "ConnectionManager::wait : Timed out waiting for threads to exit\n";
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionManager::start()
{
	m_thread.reset(new std::thread(&ConnectionManager::threadFunction,this));
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionManager::threadFunction()
{
	ConnectionData * q;
	IpPort * r;
	int ret;
	bool qempty = false;

	while (!m_stopped) {

		q = m_queue.get();
		r = m_rqueue.get();
		if ((q==0)&&(r==0)) qempty = true; else qempty = false;

		ret = 0;
		if (qempty && (m_fdset.size() > 0)) {
			ret = ::poll(&m_fdset[0], m_fdset.size(), 1);
			if (ret == -1) {
				std::cout << "ConnectionManager::threadFunction : Error from poll()\n";
			}
		}

		if (qempty && (ret<=0)) {
			if (m_fdset.size() > 0) continue;
			::usleep(1000);
			continue;
		}

		threadEventFunction(q, r, ret);

		if (q) m_queue.release();
		if (r) m_rqueue.release();
	}

	for (auto i : m_serverMap) {
		::close(i.first);
	}

	std::scoped_lock<std::mutex> lock(m_mutex);
	for (auto i : m_serverThreadList) i.second->stop();
	for (auto i : m_clientThreadList) i.second->stop();
	if ((m_clientThreadList.size()==0) && (m_serverThreadList.size()==0))
		m_stoppedConfirmed = true;
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionManager::onTerminate(pid_t tid)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	m_serverThreadList.erase(tid);
	m_clientThreadList.erase(tid);
	if (m_stopped && (m_clientThreadList.size()==0) && (m_serverThreadList.size()==0))
		m_stoppedConfirmed = true;
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionManager::shutdown()
{
	stop();
}

//////////////////////////////////////////////////////////////////////////////

void
msglib::erase_fd(std::vector<pollfd> & fdset, const int fd)
{
	for (auto i=fdset.begin();i!=fdset.end();i++) {
		if (i->fd==fd) {
			fdset.erase(i);
			return;
		}
	}
	std::cout << "erase_fd : Entry not found\n";
}

////////////////////////////////////////////////////////////////////////////// 
