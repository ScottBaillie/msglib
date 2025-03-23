//////////////////////////////////////////////////////////////////////////////

#include <Memory/MemConnectionManager.h>

using namespace msglib;

#include <iostream>

#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

MemConnectionManager::MemConnectionManager(const uint32_t maxThreads)
	: m_maxThreads(maxThreads)
{
}

//////////////////////////////////////////////////////////////////////////////

MemConnectionManager::~MemConnectionManager()
{
	stop();
	wait();
}

//////////////////////////////////////////////////////////////////////////////

bool
MemConnectionManager::add(const MemConnectionData & data)
{
	if (m_stopped) return false;

	bool ok;
	const auto & name = data.m_buffer.m_name;

	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		if (m_nameToBufferMap.find(name) != m_nameToBufferMap.end()) {
			std::cout << "MemConnectionManager::add : Name already exists\n";
			return false;
		}

		if ((m_maxThreads==0) || (m_threadList.size() < m_maxThreads)) {

			MemConnectionThreadPtr ptr;
			ptr.reset(new MemConnectionThread);
			pid_t tid = ptr->gettid();
			ptr->setTerminateHandler(this);

			m_threadList[tid] = ptr;

			m_nameToBufferMap[name] = data.m_buffer;

			ok = ptr->addConnection(data);

			return ok;
		} else {
			auto i = m_threadList.find(m_nextTid);
			if (i == m_threadList.end()) {
				i = m_threadList.begin();
			}

			MemConnectionThreadPtr ptr = i->second;

			i++;
			if (i == m_threadList.end()) {
				m_nextTid = m_threadList.begin()->first;
			} else {
				m_nextTid = i->first;
			}

			m_nameToBufferMap[name] = data.m_buffer;
                        
			ok = ptr->addConnection(data);
		}
	}

	return ok;
}

//////////////////////////////////////////////////////////////////////////////

bool
MemConnectionManager::remove(const std::string & name)
{
	if (m_stopped) return false;
	std::scoped_lock<std::mutex> lock(m_mutex);
	for (auto t : m_threadList) t.second->close(name);
	return true;
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionManager::stop()
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	for (auto t : m_threadList) t.second->stop();
	m_stopped = true;
	if (m_threadList.size()==0) m_stoppedConfirmed = true;
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionManager::wait()
{
	unsigned count = 0;
	while (true) {
		if (m_stoppedConfirmed) return;
		::usleep(1000);
		count++;
		if (m_stopped && (count > 10000)) break;
	}
	std::cout << "MemConnectionManager::wait : Timed out waiting for threads to exit\n";
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionManager::onTerminate(const pid_t tid)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	m_threadList.erase(tid);
	if (m_stopped && (m_threadList.size()==0))
		m_stoppedConfirmed = true;
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionManager::shutdown()
{
	stop();
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionManager::close(const std::string & name)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	m_nameToBufferMap.erase(name);
}

//////////////////////////////////////////////////////////////////////////////

bool
MemConnectionManager::sendMessage(const std::string & bufferName, uint8_t * p, const uint64_t size)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto i = m_nameToBufferMap.find(bufferName);
	if (i == m_nameToBufferMap.end()) {
		std::cout << "MemConnectionManager::sendMessage : name not found : " << bufferName << "\n";;
		return false;
	}

	MessageQueue msgq(i->second.m_buffer);

	bool ok = msgq.add(p, size, false);

	return ok;
}

//////////////////////////////////////////////////////////////////////////////

bool
MemBufferData::init(uint8_t * buffer, const uint64_t size, const std::string & name)
{
	m_name = name;
	m_buffer = buffer;

	MessageQueue msgq;
	msgq.init(m_buffer,size);

	return true;
}

bool
MemBufferData::init(uint8_t * buffer, const std::string & name)
{
	m_name = name;
	m_buffer = buffer;
	m_writeOnly = true;
	return true;
}

////////////////////////////////////////////////////////////////////////////// 

bool
MemConnectionHandler::sendMessage(const std::string & bufferName, uint8_t * p, const uint64_t size)
{
	if (m_connectionThread) return m_connectionThread->sendMessage(bufferName, p, size);
	return false;
}

void
MemConnectionHandler::close()
{
	if (m_connectionThread) m_connectionThread->close(m_name);
}

void
MemConnectionHandler::shutdown()
{
	if (m_connectionThread) m_connectionThread->shutdown();
}

////////////////////////////////////////////////////////////////////////////// 
