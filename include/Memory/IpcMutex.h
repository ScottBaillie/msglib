#ifndef IPCMUTEX_H
#define IPCMUTEX_H 1

#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

class IpcMutex
{
public:
	IpcMutex();
	IpcMutex(const std::string & iName, const bool iInitialLock = true);
	~IpcMutex();

	bool init(const std::string & iName);
	bool lock();
	bool unlock();

private:
	sem_t *	m_sem = 0;
	bool	m_locked = false;
};

#endif
