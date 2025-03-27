//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>

using namespace msglib;

#include <set>

//////////////////////////////////////////////////////////////////////////////

// struct pollfd {
// 	int   fd;         /* file descriptor */
// 	short events;     /* requested events */
// 	short revents;    /* returned events */
// };

//////////////////////////////////////////////////////////////////////////////

void
ConnectionThread::controlEvent(ConnectionControlData & cdata)
{
	int & fd = cdata.fd;

	auto i = m_connectionMap.find(fd);
	if (i == m_connectionMap.end()) {
		::close(fd);
		std::cout << "ConnectionThread::controlEvent : Entry not found\n";
		return;
	}
	ConnectionData & data = i->second;
	data.hlr->onConnectionTerminated();
	::close(fd);
	erase_fd(m_fdset, fd);
	m_connectionMap.erase(fd);
	m_size = m_connectionMap.size();
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionThread::addServerEvent(ConnectionData & dataIn)
{
	ConnectionData data = dataIn;

	int & fd = data.fd;

	ConnectionHandlerPtr hlr = data.hlr;
	hlr = hlr->clone();
	if (!hlr) {
		std::cout << "ConnectionThread::addServerEvent : Derived class nust define a clone() method.\n";
		return;
	}
	data.hlr = hlr;

	data.hlr->m_ipPort = data.ipPort;
	data.hlr->m_server = data.server;
	data.hlr->m_fd = data.fd;

	data.hlr->setConnectionThread(this);

	auto i = m_connectionMap.find(fd);
	if (i != m_connectionMap.end()) {
		std::cout << "ConnectionThread::addServerEvent : Entry already exists\n";
		return;
	}

	m_connectionMap[fd] = data;
	m_size = m_connectionMap.size();

	m_fdset.push_back(pollfd());
	pollfd & entry = m_fdset.back();
	entry.fd = fd;
	entry.events = POLLIN|POLLOUT|POLLRDHUP;
	entry.revents = 0;
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionThread::addClientEvent(ConnectionData & data)
{
	int fd = socket(SOCK_STREAM|SOCK_NONBLOCK, data.ipPort.addr.isv6());

	if (fd<0) {
		std::cout << "ConnectionThread::addClientEvent : Error from socket()\n";
		data.hlr->onError(1);
		return;
	}

	int ret = connect(fd, data.ipPort);

	if (ret != 0) {
		if (errno!=EINPROGRESS) {
			data.hlr->onError(2);
			::close(fd);
			std::cout << "ConnectionThread::addClientEvent : Error from connect() : " << strerror(errno) << "\n";
			return;
		}
	}

	data.fd = fd;

	data.hlr->m_ipPort = data.ipPort;
	data.hlr->m_server = data.server;
	data.hlr->m_fd = data.fd;

	data.hlr->setConnectionThread(this);

	auto i = m_connectionMap.find(fd);
	if (i != m_connectionMap.end()) {
		::close(fd);
		std::cout << "ConnectionThread::addClientEvent : Entry already exists\n";
		return;
	}

	m_connectionMap[fd] = data;
	m_size = m_connectionMap.size();

	m_fdset.push_back(pollfd());
	pollfd & entry = m_fdset.back();
	entry.fd = fd;
	entry.events = POLLIN|POLLOUT|POLLRDHUP;
	entry.revents = 0;
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionThread::fileDescChangedEvent(const int ret)
{
	if (ret == 0) {
		std::cout << "ConnectionThread::fileDescChangedEvent : No events\n";
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
					std::cout << "ConnectionThread::fileDescChangedEvent : Entry not found\n";
					continue;
				}
				ConnectionData & data = i->second;
				data.hlr->onError(4);
			}
			eraseSet.insert(fd);
			notHandled = false;
			continue;
		}
		if (entry.revents & POLLOUT) {
			auto i = m_connectionMap.find(fd);
			if (i == m_connectionMap.end()) {
				std::cout << "ConnectionThread::fileDescChangedEvent : Entry not found\n";
				continue;
			}
			ConnectionData & data = i->second;
			if (!data.server) data.clientConnected = true;
			data.hlr->onConnectionAccepted();
			entry.events = entry.events & ~POLLOUT;
			notHandled = false;
		}
		if (entry.revents & POLLIN) {
			auto i = m_connectionMap.find(fd);
			if (i == m_connectionMap.end()) {
				std::cout << "ConnectionThread::fileDescChangedEvent : Entry not found\n";
				continue;
			}
			ConnectionData & data = i->second;
			size = data.hlr->m_buffer.getWriteSize();
			if (size < (data.hlr->m_buffer.getSize()/4)) {
				data.hlr->m_buffer.compact();
				size = data.hlr->m_buffer.getWriteSize();
			}
			if (size==0) {
				std::cout << "ConnectionThread::fileDescChangedEvent : Buffer full\n";
			} else {
				status = ::read(fd, data.hlr->m_buffer.getWritePtr(), size);
				if (status==-1) {
					std::cout << "ConnectionThread::fileDescChangedEvent : Error from read\n";
					data.hlr->onError(3);
				} else {
					data.hlr->m_buffer.advanceWrite(status);
					if (status) data.hlr->onDataReceived();
				}
			}
			notHandled = false;
		}
		if (notHandled) {
			std::cout << "ConnectionThread::fileDescChangedEvent : Event not handled : " << entry.revents << "\n";
		}
	}

	for (auto fd : eraseSet) {
		auto i = m_connectionMap.find(fd);
		if (i == m_connectionMap.end()) {
			std::cout << "ConnectionThread::fileDescChangedEvent : Entry not found\n";
			continue;
		}
		ConnectionData & data = i->second;
		data.hlr->onConnectionTerminated();
		::close(fd);
		erase_fd(m_fdset, fd);
		m_connectionMap.erase(fd);
		m_size = m_connectionMap.size();
	}
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionThread::userdataEvent(UserDataQueueEntry & data)
{
	data.hlr->onUserData(data.data);
	data.hlr.reset();
	data.data.reset();
}

////////////////////////////////////////////////////////////////////////////// 

void
ConnectionThread::threadEventFunction(ConnectionData * q, ConnectionControlData * c, UserDataQueueEntry * u, const int ret)
{
	if (q || c || u) {
		if (q) {if (q->server) addServerEvent(*q); else addClientEvent(*q);}
		if (c) {controlEvent(*c);}
		if (u) {userdataEvent(*u);}
		return;
	}
	fileDescChangedEvent(ret);
}

////////////////////////////////////////////////////////////////////////////// 
