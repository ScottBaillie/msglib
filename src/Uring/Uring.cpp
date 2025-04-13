//////////////////////////////////////////////////////////////////////////////

#include <Uring/Uring.h>

using namespace msglib;

#include <iostream>

//////////////////////////////////////////////////////////////////////////////

Uring::Uring(	const unsigned sqentries,
		const unsigned cqentries,
		const bool useIoPoll,
		const bool useTaskRun,
		const bool useSingleIssuer,
		const bool useDirect)
	: m_queue(4096)
	, m_sqentries(sqentries)
	, m_cqentries(cqentries)
	, m_useIoPoll(useIoPoll)
	, m_useTaskRun(useTaskRun)
	, m_useSingleIssuer(useSingleIssuer)
	, m_useDirect(useDirect)
{
	start();

	const uint32_t WAIT_LOOPS = 1000*10;
	uint32_t u0;

	for (u0=0; u0<WAIT_LOOPS; u0++) {
		if (m_threadStarted) break;
		::usleep(1000);
	}
	if (u0==WAIT_LOOPS) std::cout << "Uring::Uring : Timed out waiting for thread to start\n";
}

//////////////////////////////////////////////////////////////////////////////

Uring::~Uring()
{
	stop();
}

//////////////////////////////////////////////////////////////////////////////

bool
Uring::read (const int fd, unsigned index, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr)
{
	UringQueueEntry * q = m_queue.next();

	if (q==0) return false;

	q->op = OP_READ;
	q->fd = fd;
	q->index = index;
	q->buf = buf;
	q->nbytes = nbytes;
	q->offset = offset;
	q->hlr = hlr;

	bool ok = m_queue.add();

	return ok;
}

//////////////////////////////////////////////////////////////////////////////

bool
Uring::write(const int fd, unsigned index, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr)
{
	UringQueueEntry * q = m_queue.next();

	if (q==0) return false;

	q->op = OP_WRITE;
	q->fd = fd;
	q->index = index;
	q->buf = buf;
	q->nbytes = nbytes;
	q->offset = offset;
	q->hlr = hlr;

	bool ok = m_queue.add();

	return ok;
}

//////////////////////////////////////////////////////////////////////////////

void *
Uring::allocateBuffers(const unsigned nbytes)
{
	void * buffer = 0;
	int ret = ::posix_memalign(&buffer, 4096, nbytes);
	if (ret!=0) return 0;
	return buffer;
}

//////////////////////////////////////////////////////////////////////////////

bool
Uring::registerBuffers(struct iovec * piovec, unsigned n)
{
	UringQueueEntry * q = m_queue.next();

	if (q==0) return false;

	q->op = OP_REGISTERBUFFERS;
	q->iov.resize(n);
	for (uint32_t u0=0; u0<n; u0++) q->iov[u0] = piovec[u0];

	bool ok = m_queue.add();

	return ok;
}

bool
Uring::unregisterBuffers()
{
	if (m_registeredBuffers.size()==0) return true;
	int ret = ::io_uring_unregister_buffers(&m_ring);
	m_registeredBuffers.clear();
	if(ret != 0) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
Uring::registerFiles(int * pfd, unsigned n)
{
	UringQueueEntry * q = m_queue.next();

	if (q==0) return false;

	q->op = OP_REGISTERFILES;
	q->fdv.resize(n);
	for (uint32_t u0=0; u0<n; u0++) q->fdv[u0] = pfd[u0];

	bool ok = m_queue.add();

	return ok;
}

bool
Uring::unregisterFiles()
{
	if (!m_filesRegistered) return true;
	int ret = ::io_uring_unregister_files(&m_ring);
	m_filesRegistered = false;
	if(ret != 0) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
Uring::get_fd(const std::string & filename, int & fd, const bool read, const bool useDirect)
{
	int flags;
	mode_t mode;

	if (read) {
		flags = 0;
		if (useDirect) flags |= O_DIRECT;
		mode = O_RDONLY;
	} else {
		flags = O_WRONLY | O_CREAT | O_TRUNC;
		if (useDirect) flags |= O_DIRECT;
		mode = 0644;
	}

	int fdesc = ::open(filename.c_str(), flags, mode);

	if (fdesc == -1) return false;

	fd = fdesc;

	return true;
}

bool
Uring::get_fd(const std::string & filename, int & fd, const bool read)
{
	return get_fd(filename, fd, read, m_useDirect);
}

//////////////////////////////////////////////////////////////////////////////

bool
Uring::read (const int fd, const unsigned index, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr)
{
	return read(fd, index, 0, nbytes, offset, hlr);
}

bool
Uring::write(const int fd, const unsigned index, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr)
{
	return write(fd, index, 0, nbytes, offset, hlr);
}

//////////////////////////////////////////////////////////////////////////////

bool
Uring::read(const int fd, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr)
{
	return read(fd, 0, buf, nbytes, offset, hlr);
}

bool
Uring::write(const int fd, void * buf, const unsigned nbytes, const uint64_t offset, UringHandlerPtr hlr)
{
	return write(fd, 0, buf, nbytes, offset, hlr);
}

//////////////////////////////////////////////////////////////////////////////

void
Uring::start()
{
	m_thread.reset(new std::thread(&Uring::threadFunction,this));
}

//////////////////////////////////////////////////////////////////////////////

void
Uring::stop()
{
	m_stopped = true;
	if (m_thread) {
		m_thread->join();
		m_thread.reset();
	}
}

//////////////////////////////////////////////////////////////////////////////

//
//	mapped = mmap(NULL, ctx->buf_ring_size, PROT_READ | PROT_WRITE,
//		      MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
//	if (mapped == MAP_FAILED) {
//		fprintf(stderr, "buf_ring mmap: %s\n", strerror(errno));
//		return -1;
//	}
//

//////////////////////////////////////////////////////////////////////////////

void
Uring::threadFunction()
{
	unsigned n;
	int ret;
	struct io_uring_params params;
	struct io_uring_cqe *cqe;
	struct io_uring_cqe cqe1;
	UringQueueEntry * q;

	memset(&params, 0, sizeof(params));
	params.cq_entries = m_cqentries;
	params.flags = IORING_SETUP_SUBMIT_ALL | IORING_SETUP_SQPOLL | IORING_SETUP_CQSIZE;
	if (m_useIoPoll) params.flags |= IORING_SETUP_IOPOLL;
	if (m_useTaskRun) params.flags |= IORING_SETUP_COOP_TASKRUN;
	if (m_useSingleIssuer) params.flags |= IORING_SETUP_SINGLE_ISSUER;

	ret = ::io_uring_queue_init_params(m_sqentries, &m_ring, &params);
	if (ret != 0) {m_threadStarted = true;std::cout << "Uring::threadFunction : Error from io_uring_queue_init_params\n";return;}

	if (m_registerRingFd) {
		ret = ::io_uring_register_ring_fd(&m_ring);
		if (ret != 1) std::cout << "Error from io_uring_register_ring_fd\n";
	}

	m_threadStarted = true;

	while (!m_stopped) {

		q = m_queue.get();
		if (q) {
			threadEventFunction(q);
			m_queue.release();
		}

		n = ::io_uring_cq_ready(&m_ring);

		if (n==0) {
			::usleep(1000);
			continue;
		}

		ret = ::io_uring_wait_cqe_nr(&m_ring ,&cqe, 1);

		if (ret != 0) {
			std::cout << "Uring::threadFunction : Error from io_uring_wait_cqe_nr()\n";
			continue;
		}

		cqe1 = *cqe;

		::io_uring_cqe_seen(&m_ring ,cqe);

		auto hlr = m_hlrMap[cqe1.user_data];
		m_hlrMap.erase(cqe1.user_data);
		if (hlr) hlr->onComplete(cqe1.res);
	}

	close();
}

//////////////////////////////////////////////////////////////////////////////

void
Uring::threadEventFunction(UringQueueEntry * q)
{
	switch (q->op) {
		case OP_READ:
			readEvent(*q);
			break;
		case OP_WRITE:
			writeEvent(*q);
			break;
		case OP_REGISTERBUFFERS:
			registerBuffersEvent(*q);
			break;
		case OP_REGISTERFILES:
			registerFilesEvent(*q);
			break;
		default:
			std::cout << "Uring::threadEventFunction : Unknown operation\n";
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////

void
Uring::readEvent(const UringQueueEntry & e)
{
	if ((e.buf == 0) && (e.index >= m_registeredBuffers.size())) {std::cout << "Uring::readEvent : Index out of range\n";return;}

	struct io_uring_sqe * sqe = ::io_uring_get_sqe(&m_ring);
	if (sqe==0) {
		std::cout << "Uring::readEvent : Unable to get a submission queue entry\n";
		return;
	}

	if (e.buf == 0) {
		void * buf = m_registeredBuffers[e.index].iov_base;
		::io_uring_prep_read_fixed(sqe, e.fd, buf, e.nbytes, e.offset, e.index);
	} else {
		::io_uring_prep_read(sqe, e.fd, e.buf, e.nbytes, e.offset);
	}

	sqe->user_data = m_nextid;
	m_hlrMap[m_nextid] = e.hlr;
	m_nextid++;

	::io_uring_submit(&m_ring);
}

//////////////////////////////////////////////////////////////////////////////

void
Uring::writeEvent(const UringQueueEntry & e)
{
	if ((e.buf == 0) && (e.index >= m_registeredBuffers.size())) {std::cout << "Uring::readEvent : Index out of range\n";return;}

	struct io_uring_sqe * sqe = ::io_uring_get_sqe(&m_ring);
	if (sqe==0) {
		std::cout << "Uring::writeEvent : Unable to get a submission queue entry\n";
		return;
	}

	if (e.buf == 0) {
		void * buf = m_registeredBuffers[e.index].iov_base;
		::io_uring_prep_write_fixed(sqe, e.fd, buf, e.nbytes, e.offset, e.index);
	} else {
		::io_uring_prep_write(sqe, e.fd, e.buf, e.nbytes, e.offset);
	}

	sqe->user_data = m_nextid;
	m_hlrMap[m_nextid] = e.hlr;
	m_nextid++;

	::io_uring_submit(&m_ring);
}

//////////////////////////////////////////////////////////////////////////////

void
Uring::registerBuffersEvent(const UringQueueEntry & e)
{
	m_registeredBuffers = e.iov;
	int ret = ::io_uring_register_buffers(&m_ring, m_registeredBuffers.data(), m_registeredBuffers.size());
	if(ret != 0) std::cout << "Uring::registerBuffersEvent : Error from io_uring_register_buffers\n";
}

void
Uring::registerFilesEvent(const UringQueueEntry & e)
{
	int ret = ::io_uring_register_files(&m_ring, e.fdv.data(), e.fdv.size());
	if(ret != 0) std::cout << "Uring::registerFilesEvent : Error from io_uring_register_files\n";
	m_filesRegistered = true;
}

//////////////////////////////////////////////////////////////////////////////

void
Uring::close()
{
	bool ok = unregisterBuffers();
	if (!ok) std::cout << "Uring::close : Error from unregisterBuffers()\n";
	ok = unregisterFiles();
	if (!ok) std::cout << "Uring::close : Error from unregisterFiles()\n";
	if (m_registerRingFd) {
		int ret = ::io_uring_unregister_ring_fd(&m_ring);
		if (ret != 1) std::cout << "Uring::close : Error from io_uring_unregister_ring_fd()\n";
		m_registerRingFd = false;
	}
	::io_uring_queue_exit(&m_ring);
}

//////////////////////////////////////////////////////////////////////////////
