//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>

using namespace msglib;

#include <set>

//////////////////////////////////////////////////////////////////////////////

void
ConnectionManager::removeServerEvent(const IpPort & ipPort)
{
	for (auto i : m_serverMap) {
		if (i.second.ipPort == ipPort) {
			erase_fd(m_fdset, i.first);
			m_serverMap.erase(i.first);
			::close(i.first);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionManager::addServerEvent(ConnectionData & data)
{
	int fd = socket(SOCK_STREAM|SOCK_NONBLOCK, data.ipPort.addr.isv6());

	if (fd<0) {
		std::cout << "ConnectionManager::addServerEvent : Error from socket()\n";
		return;
	}

	int ret = bind(fd, data.ipPort);

	if (ret != 0) {
		::close(fd);
		std::cout << "ConnectionManager::addServerEvent : Error from bind()\n";
		return;
	}

	ret = ::listen(fd, 64);
	if (ret != 0) {
		::close(fd);
		std::cout << "ConnectionManager::addServerEvent : Error from listen()\n";
		return;
	}

	auto i = m_serverMap.find(fd);
	if (i != m_serverMap.end()) {
		::close(fd);
		std::cout << "ConnectionManager::addServerEvent : Entry already exists\n";
		return;
	}

	ServerData & sdata = m_serverMap[fd];
	sdata.ipPort = data.ipPort;
	sdata.hlr = data.hlr;

	m_fdset.push_back(pollfd());
	pollfd & entry = m_fdset.back();
	entry.fd = fd;
	entry.events = POLLIN|POLLOUT|POLLRDHUP;
	entry.revents = 0;
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionManager::addClientEvent(ConnectionData & data)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	size_t size = m_clientThreadList.size();
	ConnectionThreadPtr ptr;
	if ((m_clientThreads==0) || (size < m_clientThreads)) {
		ptr.reset(new ConnectionThread);
		pid_t tid = ptr->gettid();
		ptr->setTerminateHandler(this);
		m_clientThreadList[tid] = ptr;
	} else {
		auto i = m_clientThreadList.find(m_clientNextTid);
		if (i==m_clientThreadList.end()) {
			i = m_clientThreadList.begin();
		}

		ptr = i->second;

		i++;
		if (i==m_clientThreadList.end()) {
			m_clientNextTid = m_clientThreadList.begin()->first;
		} else {
			m_clientNextTid = i->first;
		}
	}

	bool ok = ptr->addConnection(data);
	if (!ok) std::cout << "ConnectionManager::addClientEvent : Error from addConnection()\n";
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionManager::fileDescChangedEvent(const int ret)
{
	if (ret == 0) {
		std::cout << "ConnectionManager::fileDescChangedEvent : No events\n";
		return;
	}

	bool notHandled;
	int status;
	std::vector<uint8_t> sadr(sizeof(struct sockaddr_in6));
	std::set<int> eraseSet;

	for (unsigned u0=0;u0<m_fdset.size();u0++) {
		pollfd & entry = m_fdset[u0];
		int & fd = entry.fd;
		if (entry.revents==0) continue;
		notHandled = true;
		if (entry.revents & (POLLRDHUP|POLLHUP|POLLERR|POLLNVAL)) {
			eraseSet.insert(fd);
			notHandled = false;
			continue;
		}
		if (entry.revents & POLLIN) {
			auto i = m_serverMap.find(fd);
			if (i == m_serverMap.end()) {
				std::cout << "ConnectionManager::fileDescChangedEvent : Entry not found\n";
				continue;
			}
			ServerData & sdata = i->second;

			IpPort ipPort;
			status = accept(fd, ipPort, sdata.ipPort.addr.isv6());

			if (status == -1) {
				std::cout << "ConnectionManager::fileDescChangedEvent : Error from accept\n";
			} else {
				std::scoped_lock<std::mutex> lock(m_mutex);

				size_t size = m_serverThreadList.size();
				ConnectionThreadPtr ptr;
				if ((m_serverThreads==0) || (size < m_serverThreads)) {
					ptr.reset(new ConnectionThread);
					pid_t tid = ptr->gettid();
					ptr->setTerminateHandler(this);
					m_serverThreadList[tid] = ptr;
				} else {
					auto i = m_serverThreadList.find(m_serverNextTid);
					if (i==m_serverThreadList.end()) {
						i = m_serverThreadList.begin();
					}

					ptr = i->second;

					i++;
					if (i==m_serverThreadList.end()) {
						m_serverNextTid = m_serverThreadList.begin()->first;
					} else {
						m_serverNextTid = i->first;
					}
				}
				ConnectionData data;
				data.ipPort = sdata.ipPort;
				data.hlr = sdata.hlr;
				data.server = true;
				data.clientConnected = false;
				data.fd = status;
				bool ok = ptr->addConnection(data);
				if (!ok) std::cout << "ConnectionManager::fileDescChangedEvent : Error from addConnection()\n";
			}
			notHandled = false;
		}
		if (entry.revents & POLLOUT) {
			std::cout << "ConnectionManager::fileDescChangedEvent : TEST CODE : POLLOUT : fd=" << fd << "\n";
			notHandled = false;
		}
		if (notHandled) {
			std::cout << "ConnectionManager::fileDescChangedEvent : Event not handled : " << entry.revents << "\n";
		}
	}

	for (auto i : eraseSet) {
		::close(i);
		erase_fd(m_fdset, i);
		m_serverMap.erase(i);
	}
}

//////////////////////////////////////////////////////////////////////////////

void
ConnectionManager::threadEventFunction(ConnectionData * q, IpPort * r, const int ret)
{
	if (q || r) {
		if (q) if (q->server) addServerEvent(*q); else addClientEvent(*q);
		if (r) removeServerEvent(*r);
		return;
	}
	fileDescChangedEvent(ret);
}

///////////////////////////////////////////////////////////////////////////// 
