//////////////////////////////////////////////////////////////////////////////

#include <Memory/IpcMutex.h>

using namespace msglib;

#include <iostream>
#include <stdexcept>

//////////////////////////////////////////////////////////////////////////////

IpcMutex::IpcMutex()
{
}

IpcMutex::IpcMutex(const std::string & iName, const bool iInitialLock)
{
	bool ok = this->init(iName);
	if (!ok) {
		throw std::runtime_error("IpcMutex::IpcMutex : init() failed");
	}
	if (iInitialLock) {
		ok = this->lock();
		if (!ok) {
			throw std::runtime_error("IpcMutex::IpcMutex : lock() failed");
		}
	}
}

IpcMutex::~IpcMutex()
{
	if (this->m_locked) {
		bool ok = this->unlock();
		if (!ok) {
			// Log error
		}
	}
}

bool
IpcMutex::init(const std::string & iName)
{
	if (this->m_sem) {
		return false;
	}
	if (iName.size() < 2) {
		return false;
	}
	if (iName[0] != '/') {
		return false;
	}

	int oflag = O_CREAT;
	mode_t mode = S_IRWXU;
	unsigned int value = 1;

	sem_t * sem = sem_open(iName.c_str(), oflag, mode, value);

	if (sem == SEM_FAILED) {
		return false;
	}

	this->m_sem = sem;

	return true;
}

bool
IpcMutex::lock()
{
	if (!this->m_sem) {
		return false;
	}
	if (this->m_locked) {
		return false;
	}

	int ret = sem_wait(this->m_sem);

	if (ret != 0) {
		return false;
	}

	this->m_locked = true;

	return true;
}

bool
IpcMutex::unlock()
{
	if (!this->m_sem) {
		return false;
	}
	if (!this->m_locked) {
		return false;
	}

	int ret = sem_post(this->m_sem);

	if (ret != 0) {
		return false;
	}

	this->m_locked = false;

	return true;
}
