//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>

using namespace msglib;

#include <set>

//////////////////////////////////////////////////////////////////////////////

UdpConnectionThread::UdpConnectionThread()
	: m_queue(1024)
	, m_controlq(1024)
{
	start();
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::setTerminateHandler(UdpThreadTerminateHandler * terminateHandler)
{
	m_terminateHandler = terminateHandler;
}

//////////////////////////////////////////////////////////////////////////////

bool
UdpConnectionThread::addConnection(const UdpConnectionData & data)
{
	if (m_stopped) return false;
	UdpConnectionData * q = m_queue.next();
	*q = data;
	bool ok = m_queue.add();
	return ok;
}

////////////////////////////////////////////////////////////////////////////// 

pid_t
UdpConnectionThread::gettid() const
{
	return m_tid;
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::stop()
{
	m_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

size_t
UdpConnectionThread::size() const
{
	return m_size;
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::close(const int fd)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	UdpConnectionControlData * q = m_controlq.next();
	q->fd = fd;
	bool ok = m_controlq.add();
	if (!ok) std::cout << "UdpConnectionThread::close : Error from add()\n";
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::shutdown()
{
	if (m_terminateHandler) m_terminateHandler->shutdown();
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::start()
{
	std::thread t(&UdpConnectionThread::threadFunction,this);
	t.detach();

	for (unsigned u0=0;u0<4000;u0++) {
		if (m_tid != 0) return;
		::usleep(1000);
	}
	std::cout << "UdpConnectionThread::start : Timed out waiting for thread to start\n";
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::threadFunction()
{
	m_tid = ::gettid();

	UdpConnectionData * q;
	UdpConnectionControlData * c;
	int ret;
	unsigned sleep_count = 0;
	bool qempty = false;

	while (!m_stopped) {

		q = m_queue.get();
		c = m_controlq.get();
		if ((q==0)&&(c==0)) qempty = true; else qempty = false;

		ret = 0;
		if (qempty && (m_fdset.size() > 0)) {
			ret = ::poll(&m_fdset[0], m_fdset.size(), 1);
			if (ret == -1) {
				std::cout << "UdpConnectionThread::threadFunction : Error from poll()\n";
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

		threadEventFunction(q, c, ret);

		if (q) m_queue.release();
		if (c) m_controlq.release();
	}

	for (auto i : m_connectionMap) {
		i.second.hlr->onConnectionTerminated();
		::close(i.first);
	}

	if (m_terminateHandler) m_terminateHandler->onTerminate(m_tid);
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::threadEventFunction(UdpConnectionData * q, UdpConnectionControlData * c, const int ret)
{
	if (q || c) {
		if (q) {addConnectionEvent(*q);}
		if (c) {controlEvent(*c);}
		return;
	}
	fileDescChangedEvent(ret);
}

////////////////////////////////////////////////////////////////////////////// 

void
UdpConnectionThread::controlEvent(UdpConnectionControlData & cdata)
{
	int & fd = cdata.fd;

	auto i = m_connectionMap.find(fd);
	if (i == m_connectionMap.end()) {
		std::cout << "UdpConnectionThread::controlEvent : Entry not found\n";
		return;
	}
	UdpConnectionData & data = i->second;
	data.hlr->onConnectionTerminated();
	::close(fd);
	erase_fd(m_fdset, fd);
	m_connectionMap.erase(fd);
	m_size = m_connectionMap.size();
}

////////////////////////////////////////////////////////////////////////////// 

void
UdpConnectionThread::addConnectionEvent(UdpConnectionData & data)
{
	int & fd = data.fd;

	auto i = m_connectionMap.find(fd);
	if (i != m_connectionMap.end()) {
		std::cout << "UdpConnectionThread::addConnectionEvent : Entry already exists\n";
		return;
	}

	data.hlr->m_ipPort = data.ipPort;
	data.hlr->m_fd = data.fd;
	data.hlr->setConnectionThread(this);

	m_connectionMap[fd] = data;
	m_size = m_connectionMap.size();

	m_fdset.push_back(pollfd());
	pollfd & entry = m_fdset.back();
	entry.fd = fd;
	entry.events = POLLIN|POLLRDHUP;
	entry.revents = 0;

	data.hlr->onConnectionAccepted();
}

//////////////////////////////////////////////////////////////////////////////

void
UdpConnectionThread::fileDescChangedEvent(const int ret)
{
	if (ret == 0) {
		std::cout << "UdpConnectionThread::fileDescChangedEvent : No events\n";
		return;
	}

	bool notHandled;
	size_t size;
	ssize_t status;
	std::set<int> eraseSet;

	for (unsigned u0=0;u0<m_fdset.size();u0++) {
		pollfd & entry = m_fdset[u0];
		int & fd = entry.fd;
		if (entry.revents==0) continue;
		notHandled = true;
		if (entry.revents & (POLLRDHUP|POLLHUP|POLLERR|POLLNVAL)) {
			if (entry.revents & POLLERR) {
				auto i = m_connectionMap.find(fd);
				if (i == m_connectionMap.end()) {
					std::cout << "UdpConnectionThread::fileDescChangedEvent : Entry not found\n";
					continue;
				}
				UdpConnectionData & data = i->second;
				data.hlr->onError(4);
			}
			eraseSet.insert(fd);
			continue;
		}
		if (entry.revents & POLLIN) {
			auto i = m_connectionMap.find(fd);
			if (i == m_connectionMap.end()) {
				std::cout << "UdpConnectionThread::fileDescChangedEvent : Entry not found\n";
				continue;
			}
			UdpConnectionData & data = i->second;
			auto & buffer = data.hlr->m_buffer;
			if (buffer.size() == 0) buffer.resize(4096);
			IpPort ipPort;
			ssize_t sz = recvfrom(fd, buffer.data(), buffer.size(), 0, ipPort, data.ipPort.addr.isv6());
			if (sz == -1) {
				data.hlr->onError(5);
			} else {
				data.hlr->onMessageReceived(ipPort, buffer.data(), sz);
			}
			notHandled = false;
		}
		if (notHandled) {
			std::cout << "UdpConnectionThread::fileDescChangedEvent : Event not handled : " << entry.revents << "\n";
		}
	}

	for (auto fd : eraseSet) {
		auto i = m_connectionMap.find(fd);
		if (i == m_connectionMap.end()) {
			std::cout << "UdpConnectionThread::fileDescChangedEvent : Entry not found\n";
			continue;
		}
		UdpConnectionData & data = i->second;
		data.hlr->onConnectionTerminated();
		::close(fd);
		erase_fd(m_fdset, fd);
		m_connectionMap.erase(fd);
		m_size = m_connectionMap.size();
	}
}

//////////////////////////////////////////////////////////////////////////////

