//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>

using namespace msglib;

//////////////////////////////////////////////////////////////////////////////

UdpConnectionManager::UdpConnectionManager(const uint32_t maxThreads)
	: m_maxThreads(maxThreads)
{
}

//////////////////////////////////////////////////////////////////////////////

UdpConnectionManager::~UdpConnectionManager()
{
	stop();
	wait();
}

////////////////////////////////////////////////////////////////////////////// 

bool
UdpConnectionManager::add(UdpConnectionData & data)
{
	if (m_stopped) return false;

	int fd = socket(SOCK_DGRAM | SOCK_NONBLOCK, data.ipPort.addr.isv6());
	if (fd==-1) return false;

	int ret = bind(fd, data.ipPort);
	if (ret==-1) {::close(fd);return false;}

	data.fd = fd;

	bool ok;

	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		if (m_stopped) {::close(fd);return false;}

		if ((m_maxThreads==0) || (m_threadList.size() < m_maxThreads)) {

			UdpConnectionThreadPtr ptr;
			ptr.reset(new UdpConnectionThread);
			pid_t tid = ptr->gettid();
			ptr->setTerminateHandler(this);

			m_threadList[tid] = ptr;

			ok = ptr->addConnection(data);
		} else {
			auto i = m_threadList.find(m_nextTid);
			if (i == m_threadList.end()) {
				i = m_threadList.begin();
			}

			UdpConnectionThreadPtr ptr = i->second;

			i++;
			if (i == m_threadList.end()) {
				m_nextTid = m_threadList.begin()->first;
			} else {
				m_nextTid = i->first;
			}

			ok = ptr->addConnection(data);
		}
	}

	return ok;
}

//////////////////////////////////////////////////////////////////////////////

bool
UdpConnectionManager::remove(const int fd)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	if (m_stopped) return false;
	for (auto t : m_threadList) t.second->close(fd);
	return true;
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionManager::stop()
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	for (auto t : m_threadList) t.second->stop();
	m_stopped = true;
	if (m_threadList.size()==0) m_stoppedConfirmed = true;
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionManager::wait()
{
	unsigned count = 0;
	while (true) {
		if (m_stoppedConfirmed) return;
		::usleep(1000);
		count++;
		if (m_stopped && (count > 10000)) break;
	}
	std::cout << "UdpConnectionManager::wait : Timed out waiting for threads to exit\n";
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionManager::onTerminate(const pid_t tid)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	m_threadList.erase(tid);
	if (m_stopped && (m_threadList.size()==0))
		m_stoppedConfirmed = true;
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionManager::shutdown()
{
	stop();
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionHandler::close()
{
	if (m_connectionThread) m_connectionThread->close(m_fd);
}

void
UdpConnectionHandler::shutdown()
{
	if (m_connectionThread) m_connectionThread->shutdown();
}

bool
UdpConnectionHandler::sendMessage(const IpPort & ipPort, uint8_t * p, const size_t len)
{
	int flags = 0;
	ssize_t ret = sendto(m_fd, p, len, flags, ipPort);
	if (ret==-1) return false;
	return true;
}

bool
UdpConnectionHandler::sendMessage(const int fd, const IpPort & ipPort, uint8_t * p, const size_t len)
{
	int flags = 0;
	ssize_t ret = sendto(fd, p, len, flags, ipPort);
	if (ret==-1) return false;
	return true;
}

bool
UdpConnectionHandler::sendMessageAnon(const IpPort & ipPort, uint8_t * p, const size_t len)
{
	int flags = 0;
	int fd = socket(SOCK_DGRAM, ipPort.addr.isv6());
	if (fd==-1) return false;
	ssize_t ret = sendto(fd, p, len, flags, ipPort);
	if (ret==-1) {::close(fd);return false;}
	::close(fd);
	return true;
}

//////////////////////////////////////////////////////////////////////////////

