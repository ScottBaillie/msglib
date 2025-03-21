//////////////////////////////////////////////////////////////////////////////

#include <FastQueue/FastQueue.h>

#include <random>
#include <atomic>
#include <iostream>
#include <unordered_map>
#include <type_traits>
#include <memory>
#include <limits>
#include <map>
#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <linux/io_uring.h> 
#include <liburing.h> 

//////////////////////////////////////////////////////////////////////////////

class UringHandler
{
public:
	virtual ~UringHandler() {}

	virtual void onComplete(const int res) = 0;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<UringHandler> UringHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

const uint32_t OP_READ = 0;
const uint32_t OP_WRITE = 1;
const uint32_t OP_REGISTERBUFFERS = 2;
const uint32_t OP_REGISTERFILES = 3;

//////////////////////////////////////////////////////////////////////////////

struct UringQueueEntry {
	uint32_t		op; // 0=read , 1=write
	int			fd;
	unsigned		index;
	void *			buf;
	unsigned		nbytes;
	uint64_t		offset;
	UringHandlerPtr		hlr;
	std::vector<struct iovec>	iov;
	std::vector<int>	fdv;
};

//////////////////////////////////////////////////////////////////////////////

class Uring
{
public:
	Uring(const unsigned entries = 1);
	~Uring();

	void * allocateBuffers(const unsigned nbytes);

	bool registerBuffers(struct iovec * piovec, unsigned n);

	bool registerFiles(int * pfd, unsigned n);

	bool get_fd(const std::string & filename, int & fd, const bool read);

	// Use these for register buffers.
	// index : is the index in iovec structure at register time.
	bool read (const int fd, const unsigned index, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr);
	bool write(const int fd, const unsigned index, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr);

	// Use these for unregistered buffers.
	bool read (const int fd, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr);
	bool write(const int fd, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr);

	void stop();

private:
	void threadFunction();
	void threadEventFunction(UringQueueEntry * q);
	void readEvent(const UringQueueEntry & e);
	void writeEvent(const UringQueueEntry & e);
	void registerBuffersEvent(const UringQueueEntry & e);
	void registerFilesEvent(const UringQueueEntry & e);

	void start();
	bool unregisterBuffers();
	bool unregisterFiles();
	void close();

	bool read (const int fd, unsigned index, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr);
	bool write(const int fd, unsigned index, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr);

private:
	unsigned				m_entries = 0;
	bool					m_registerRingFd = true;
	bool					m_useIoPoll = false;
	bool					m_useTaskRun = false;
	bool					m_useDirect = true;

	std::unique_ptr<std::thread>		m_thread;
	bool					m_stopped = false;
	bool					m_filesRegistered = false;
	FastQueue<UringQueueEntry>		m_queue;
	std::map<uint64_t,UringHandlerPtr>	m_hlrMap;
	uint64_t				m_nextid = 0;

	std::vector<struct iovec>		m_registeredBuffers;

	struct io_uring 			m_ring;
};

////////////////////////////////////////////////////////////////////////////// 
