//////////////////////////////////////////////////////////////////////////////

#include <Uring/Uring.h>

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <random>
#include <vector>
#include <set>

#include <cstdlib>

#include <signal.h>

//////////////////////////////////////////////////////////////////////////////

bool g_stopped = false;

//////////////////////////////////////////////////////////////////////////////

void signal_handler(int signal)
{
	g_stopped = true;
}

//////////////////////////////////////////////////////////////////////////////

class Test1UringHandler : public UringHandler
{
public:
	virtual ~Test1UringHandler() {}

	virtual void onComplete(const int res)
	{
		std::cout << "Test1UringHandler::onComplete : res=" << res << "\n";
		if (res<0) std::cout << "Test1UringHandler::onComplete : error=" << ::strerror(res) << "\n";
	}
};

//////////////////////////////////////////////////////////////////////////////

const unsigned BUFF1_SIZE = 65536*16;

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret = 0;
	bool ok;
	int fd[2];
	void * buffer_read;
	void * buffer_write;

	{
		bool useIoPoll = false;
		bool useTaskRun = false;
		bool useSingleIssuer = false;
		bool useDirect = false;

		Uring u1(8,8,useIoPoll,useTaskRun,useSingleIssuer,useDirect);

		buffer_read = u1.allocateBuffers(BUFF1_SIZE);
		if (buffer_read==0) {std::cout << "main : Error from allocateBuffers\n";return 0;}
		buffer_write = u1.allocateBuffers(BUFF1_SIZE);
		if (buffer_write==0) {std::cout << "main : Error from allocateBuffers\n";return 0;}

		struct iovec iov[2];
		iov[0].iov_base = buffer_read;
		iov[0].iov_len = BUFF1_SIZE;
		iov[1].iov_base = buffer_write;
		iov[1].iov_len = BUFF1_SIZE;

		ok = u1.registerBuffers(iov, 2);
		if (!ok) {std::cout << "main : Error from registerBuffers\n";return 0;}

		ok = u1.get_fd("/home/sbaillie/tags", fd[0], true);
		if (!ok) {std::cout << "main : Error from get_fd\n";return 0;}

		ok = u1.get_fd("/home/sbaillie/tags2", fd[1], false);
		if (!ok) {std::cout << "main : Error from get_fd\n";return 0;}

		ok = u1.registerFiles(fd, 2);
		if (!ok) {std::cout << "main : Error from registerFiles\n";return 0;}

		for (uint32_t u0=0; u0<1; u0++) {
//			ok = u1.read(fd[0], buffer_read, BUFF1_SIZE, 0, UringHandlerPtr(new Test1UringHandler));
			ok = u1.read(fd[0], (unsigned)0, BUFF1_SIZE, 0, UringHandlerPtr(new Test1UringHandler));
			if (!ok) {std::cout << "main : Error from read\n";return 0;}
			ok = u1.write(fd[1], (unsigned)1, BUFF1_SIZE, 0, UringHandlerPtr(new Test1UringHandler));
//			ok = u1.write(fd[1], buffer_write, BUFF1_SIZE, 0, UringHandlerPtr(new Test1UringHandler));
			if (!ok) {std::cout << "main : Error from write\n";return 0;}
		}

		::signal(SIGTERM, signal_handler);
		::signal(SIGINT, signal_handler);

		while (!g_stopped) {
			::usleep(1000);
		}
	}

	::close(fd[0]);
	::close(fd[1]);

	::free(buffer_read);
	::free(buffer_write);

	std::cout << "main : exit OK\n";

	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
