
#include <Memory/IpcMem.h>
#include <Memory/IpcMutex.h>

using namespace msglib;

#include <iostream>
#include <stdexcept>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

IpcMem::IpcMem()
{
}

IpcMem::IpcMem(	const size_t iSize,
	const std::string & iName,
	const bool iReadOnly,
	const bool iCreate,
	const bool iExclusive)
{
	bool ok = this->init(iSize,iName,iReadOnly,iCreate,iExclusive);
	if (!ok) {
		throw std::runtime_error("IpcMem::IpcMem : init() failed");
	}
}

IpcMem::~IpcMem()
{
	bool ok = this->unmap();
	if (!ok) {
		std::cout << "IpcMem::~IpcMem : Error from unmap";
	}
}

bool
IpcMem::init(
	const size_t iSize,
	const std::string & iName,
	const bool iReadOnly,
	const bool iCreate,
	const bool iExclusive)
{
	if ((iSize==0) || (iName.size()==0)) {
		std::cout << "IpcMem::init : Invalid args\n";
		return false;
	}

	this->m_name = iName;
	this->m_readOnly = iReadOnly;
	this->m_size = iSize;

	int oflag = 0;
	if (iReadOnly) {
		oflag = O_RDONLY;
	} else {
		oflag = O_RDWR;
	}
	if (iCreate) {
		oflag |= O_CREAT;
	}
	if (iExclusive) {
		oflag |= O_EXCL;
	}

	mode_t mode = S_IRUSR | S_IWUSR;

	int fd = shm_open(iName.c_str(), oflag, mode);
	if (fd == -1) {
		std::cout << "IpcMem::init : Error from shm_open()\n";
		return false;
	}

	int ret = ftruncate(fd, iSize);
	if (ret == -1) {
		::close(fd);
		std::cout << "IpcMem::init : Error from ftruncate()\n";
		return false;
	}

	int prot = 0;
	if (iReadOnly) {
		prot = PROT_READ;
	} else {
		prot = PROT_READ | PROT_WRITE;
	}

	int flags = MAP_SHARED;

	void * ptr = mmap(0, iSize, prot, flags, fd, 0);

	if (ptr == MAP_FAILED) {
		::close(fd);
		std::cout << "IpcMem::init : Error from mmap()\n";
		return false;
	}

	ret = ::close(fd);
	if (ret == -1) {
		std::cout << "IpcMem::init : Error from close()\n";
		return false;
	}

	this->m_memPtr = ptr;

	return true;
}

bool
IpcMem::init(const size_t iSize, const std::string & iName, bool & oCreated)
{
	IpcMutex mutex(iName);

	bool ok = init(iSize,iName,false,true,true);
	if (ok) {oCreated=true; return true;}

	ok = init(iSize,iName,false,false,false);

	oCreated=false;

	return ok;
}


bool
IpcMem::close()
{
	int ret;

	if (this->m_memPtr) {
		ret = munmap(this->m_memPtr, this->m_size);
		if (ret == -1) {
			return false;
		}
		this->m_memPtr = 0;
	}

	if (this->m_name.size()) {
		ret = shm_unlink(this->m_name.c_str());
		if (ret == -1) {
			return false;
		}
		this->m_name = "";
	}

	return true;
}

bool
IpcMem::unmap()
{
	if (this->m_memPtr) {
		int ret = munmap(this->m_memPtr, this->m_size);
		if (ret == -1) {
			return false;
		}
		this->m_memPtr = 0;
	}

	return true;
}

bool
IpcMem::unlink()
{
	if (this->m_name.size()) {
		int ret = shm_unlink(this->m_name.c_str());
		if (ret == -1) {
			return false;
		}
		this->m_name = "";
	}

	return true;
}


void *
IpcMem::getMemPtr()
{
	return this->m_memPtr;
}
