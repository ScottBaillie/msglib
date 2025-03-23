//////////////////////////////////////////////////////////////////////////////

#include <Memory/MemConnectionManager.h>

using namespace msglib;

#include <iostream>

#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

MemConnectionThread::MemConnectionThread()
	: m_queue(1024)
	, m_controlq(1024)
{
	start();
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::setTerminateHandler(MemThreadTerminateHandler * terminateHandler)
{
	m_terminateHandler = terminateHandler;
}

//////////////////////////////////////////////////////////////////////////////

bool
MemConnectionThread::addConnection(const MemConnectionData & data)
{
	if (m_stopped) return false;
	MemConnectionData * q = m_queue.next();
	*q = data;
	bool ok = m_queue.add();
	return ok;
}

//////////////////////////////////////////////////////////////////////////////

pid_t
MemConnectionThread::gettid() const
{
	return m_tid;
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::stop()
{
	m_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

size_t
MemConnectionThread::size() const
{
	return m_size;
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::close(const std::string & name)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	MemConnectionControlData * q = m_controlq.next();
	q->m_name = name;
	bool ok = m_controlq.add();
	if (!ok) std::cout << "MemConnectionThread::close : Error from add()\n";
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::shutdown()
{
	if (m_terminateHandler) m_terminateHandler->shutdown();
}

//////////////////////////////////////////////////////////////////////////////

bool
MemConnectionThread::sendMessage(const std::string & bufferName, uint8_t * p, const uint64_t size)
{
	if (m_terminateHandler) return m_terminateHandler->sendMessage(bufferName, p, size);
	return false;
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::start()
{
	std::thread t(&MemConnectionThread::threadFunction,this);
	t.detach();

	for (unsigned u0=0;u0<4000;u0++) {
		if (m_tid != 0) return;
		::usleep(1000);
	}
	std::cout << "MemConnectionThread::start : Timed out waiting for thread to start\n";
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::threadFunction()
{
	m_tid = ::gettid();

	MemConnectionData * q;
	MemConnectionControlData * c;
	int ret;
	unsigned sleep_count = 0;
	bool qempty = false;
	bool ok;
	BytePtr p;
	uint64_t size;

	while (!m_stopped) {

		q = m_queue.get();
		c = m_controlq.get();
		if ((q==0)&&(c==0)) qempty = true; else qempty = false;

		for (auto c : m_connectionMap) {
			MessageQueue msgq(c.second.m_buffer.m_buffer);
			while (true) {
				ok = msgq.get(p, size);
				if (!ok) break;
				c.second.m_hlr->onMessageReceived(p, size);
				msgq.release(size);
			}
		}

		if (qempty) {
			::usleep(1000);
			sleep_count++;
			const unsigned SLEEP_COUNT_MAX = 1000;
			if (sleep_count < SLEEP_COUNT_MAX) continue;
			sleep_count = 0;
			if (m_connectionMap.size() == 0) continue;

			auto i = m_connectionMap.find(m_nextName);
			if (i == m_connectionMap.end()) {
				i = m_connectionMap.begin();
			}

			i->second.m_hlr->onTimer(0);

			i++;
			if (i == m_connectionMap.end()) {
				m_nextName = m_connectionMap.begin()->first;
			} else {
				m_nextName = i->first;
			}

			continue;
		}

		threadEventFunction(q, c);

		if (q) m_queue.release();
		if (c) m_controlq.release();
	}

	for (auto i : m_connectionMap) {
		i.second.m_hlr->onConnectionTerminated();
		if (m_terminateHandler) m_terminateHandler->close(i.first);
	}

	if (m_terminateHandler) m_terminateHandler->onTerminate(m_tid);
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::threadEventFunction(MemConnectionData * q, MemConnectionControlData * c)
{
	if (q) {addConnectionEvent(*q);}
	if (c) {controlEvent(*c);}
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::addConnectionEvent(MemConnectionData & indata)
{
	auto & name = indata.m_buffer.m_name;

	auto i = m_connectionMap.find(name);
	if (i != m_connectionMap.end()) {
		std::cout << "MemConnectionThread::controlEvent : Entry already exists\n";
		return;
	}

	indata.m_hlr->setConnectionThread(this);
	indata.m_hlr->m_name = name;
	indata.m_hlr->onConnectionAccepted();

	m_connectionMap[name] = indata;
	m_size = m_connectionMap.size();
}

//////////////////////////////////////////////////////////////////////////////

void
MemConnectionThread::controlEvent(MemConnectionControlData & indata)
{
	auto & name = indata.m_name;

	auto i = m_connectionMap.find(name);
	if (i == m_connectionMap.end()) {
		std::cout << "MemConnectionThread::controlEvent : Entry not found\n";
		return;
	}
	MemConnectionData & data = i->second;
	data.m_hlr->onConnectionTerminated();
	m_connectionMap.erase(name);
	m_size = m_connectionMap.size();
	if (m_terminateHandler) m_terminateHandler->close(name);
}

////////////////////////////////////////////////////////////////////////////// 
