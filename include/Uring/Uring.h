//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_URING_H
#define MSGLIB_URING_H

//////////////////////////////////////////////////////////////////////////////

#include <FastQueue/FastQueue.h>

#include <memory>
#include <map>
#include <thread>

#include <linux/io_uring.h> 
#include <liburing.h> 

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

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
	Uring(	const unsigned sqentries,
		const unsigned cqentries,
		const bool useIoPoll,
		const bool useTaskRun,
		const bool useSingleIssuer,
		const bool useDirect);

	~Uring();

	void * allocateBuffers(const unsigned nbytes);

	bool registerBuffers(struct iovec * piovec, unsigned n);

	bool registerFiles(int * pfd, unsigned n);

	bool get_fd(const std::string & filename, int & fd, const bool read);
	bool get_fd(const std::string & filename, int & fd, const bool read, const bool useDirect);

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
	unsigned				m_sqentries;
	unsigned				m_cqentries;

	bool					m_registerRingFd = true;

	// The file system (if any) and block device must support polling in order for this to work.
	// Feature is usable only on a file descriptor opened using O_DIRECT.
	bool					m_useIoPoll; 

	// By default, io_uring will interrupt a task running in userspace when a completion event comes in.
	// This flag will prevent task interuption, hence must poll for completion events.
	bool					m_useTaskRun; // This requires a kernel version >= 6.0

	// A hint to the kernel that only a single task (or thread) will submit requests,
	// which is used for internal optimisations. 
	bool					m_useSingleIssuer; // Available since 6.0.

	// Use O_DIRECT flag when opening files.
	bool					m_useDirect;

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

}

#endif
