//////////////////////////////////////////////////////////////////////////////

#include <Uring/Uring.h>

using namespace msglib;

#include <atomic>
#include <iostream>
#include <random>

#include <signal.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

class TestUringHandler : public UringHandler
{
public:
	TestUringHandler(const int fd, const uint32_t buffersize)
		: m_fd(fd)
		, m_buffersize(buffersize)
	{
	}

	virtual ~TestUringHandler() {}

	virtual void onComplete(const int res)
	{
		std::cout << "TestUringHandler::onComplete : res=" << res << " : fd=" << m_fd << "\n";
		if (res<0) {std::cout << "TestUringHandler::onComplete : error=" << ::strerror(res) << " : fd=" << m_fd << "\n";return;}

		m_length += res;
		if (m_length == m_buffersize) {
			m_complete = true;
		}
	}

public:
	int		m_fd = 0;
	int		m_length = 0;
	uint32_t	m_buffersize = 0;
	bool		m_complete = false;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<TestUringHandler> TestUringHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	bool ok;
	int fd;
	std::string filename;

	std::string dirname = argv[1];

	const uint32_t NUM_READ_BUFFER = 32;
	const uint32_t NUM_WRITE_BUFFER = 32;

	const uint32_t READ_BUFFER_SIZE = 8*1024*1024;
	const uint32_t WRITE_BUFFER_SIZE = 8*1024*1024;

	std::vector<void*> readBuffers(NUM_READ_BUFFER);
	std::vector<void*> writeBuffers(NUM_WRITE_BUFFER);

	struct iovec iov[NUM_READ_BUFFER+NUM_WRITE_BUFFER];

	std::vector<int> readFd(NUM_READ_BUFFER);
	std::vector<int> writeFd(NUM_WRITE_BUFFER);

	std::vector<TestUringHandlerPtr> readhlr(NUM_READ_BUFFER);
	std::vector<TestUringHandlerPtr> writehlr(NUM_WRITE_BUFFER);

	bool useIoPoll = false;
	bool useTaskRun = false;
	bool useSingleIssuer = false;
	bool useDirect = false;

	uint32_t sqdepth = 64;
	uint32_t cqdepth = 64;

	Uring u1(sqdepth,cqdepth,useIoPoll,useTaskRun,useSingleIssuer,useDirect);

	//// Allocate buffers

	for (uint32_t u0=0; u0<NUM_READ_BUFFER; u0++) {
		readBuffers[u0] = u1.allocateBuffers(READ_BUFFER_SIZE);
		if (readBuffers[u0]==0) {std::cout << "main : Error from allocateBuffers\n";return 0;}

	}
	for (uint32_t u0=0; u0<NUM_WRITE_BUFFER; u0++) {
		writeBuffers[u0] = u1.allocateBuffers(WRITE_BUFFER_SIZE);
		if (writeBuffers[u0]==0) {std::cout << "main : Error from allocateBuffers\n";return 0;}

	}

	//// Register buffers

	for (uint32_t u0=0; u0<NUM_READ_BUFFER; u0++) {
		iov[u0].iov_base = readBuffers[u0];
		iov[u0].iov_len = READ_BUFFER_SIZE;
	}
	for (uint32_t u0=0; u0<NUM_WRITE_BUFFER; u0++) {
		iov[u0+NUM_READ_BUFFER].iov_base = writeBuffers[u0];
		iov[u0+NUM_READ_BUFFER].iov_len = WRITE_BUFFER_SIZE;
	}
	ok = u1.registerBuffers(iov, NUM_READ_BUFFER+NUM_WRITE_BUFFER);
	if (!ok) {std::cout << "main : Error from registerBuffers\n";return 0;}

	//// Open and register write files.

	for (uint32_t u0=0; u0<NUM_WRITE_BUFFER; u0++) {
		filename = dirname + "/testfile_" + std::to_string(u0);
		ok = u1.get_fd(filename, fd, false);
		if (!ok) {std::cout << "main : Error from get_fd\n";return 0;}
		writeFd[u0] = fd;
	}
	ok = u1.registerFiles(writeFd.data(), writeFd.size());
	if (!ok) {std::cout << "main : Error from registerFiles\n";return 0;}

	//// Write to the files.

	for (uint32_t u0=0; u0<NUM_WRITE_BUFFER; u0++) {
		writehlr[u0].reset(new TestUringHandler(writeFd[u0],WRITE_BUFFER_SIZE));
		ok = u1.write(writeFd[u0], (unsigned)(u0+NUM_READ_BUFFER), WRITE_BUFFER_SIZE, 0, writehlr[u0]);
		if (!ok) {std::cout << "main : Error from write\n";return 0;}
	}

	//// Wait for the writes to complete.

	uint32_t u;
	while (true) {
		for (u=0; u<NUM_WRITE_BUFFER; u++) {
			if (!writehlr[u]->m_complete) break;
		}
		if (u==NUM_WRITE_BUFFER) break;
		::usleep(1000);
	}

	//// Open read files. A second call to registerFiles fails.

	for (uint32_t u0=0; u0<NUM_READ_BUFFER; u0++) {
		filename = dirname + "/testfile_" + std::to_string(u0%NUM_WRITE_BUFFER);
		ok = u1.get_fd(filename, fd, true);
		if (!ok) {std::cout << "main : Error from get_fd\n";return 0;}
		readFd[u0] = fd;
	}
//	ok = u1.registerFiles(readFd.data(), readFd.size());
//	if (!ok) {std::cout << "main : Error from registerFiles\n";return 0;}

	//// Read the files.

	for (uint32_t u0=0; u0<NUM_READ_BUFFER; u0++) {
		readhlr[u0].reset(new TestUringHandler(readFd[u0],READ_BUFFER_SIZE));
		ok = u1.read(readFd[u0], (unsigned)u0, READ_BUFFER_SIZE, 0, readhlr[u0]);
		if (!ok) {std::cout << "main : Error from write\n";return 0;}
	}

	//// Wait for the reads to complete.

	while (true) {
		for (u=0; u<NUM_READ_BUFFER; u++) {
			if (!readhlr[u]->m_complete) break;
		}
		if (u==NUM_READ_BUFFER) break;
		::usleep(1000);
	}

	////

	std::cout << "main : Complete\n";

	////

	::signal(SIGTERM, signal_handler);
	::signal(SIGINT, signal_handler);

	while (!g_stopped) {
		::usleep(1000);
	}

///////

	for (auto & i : readFd) ::close(i);
	for (auto & i : writeFd) ::close(i);
	for (auto & i : readBuffers) ::free(i);
	for (auto & i : writeBuffers) ::free(i);

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
