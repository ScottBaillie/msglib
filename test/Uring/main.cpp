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
	int fd;
	void * array;

	ret = ::posix_memalign(&array, 4096, BUFF1_SIZE);
	if (ret!=0) {std::cout << "main : Error from posix_memalign\n";return 0;}

	{
		Uring u1(8);

		struct iovec iov;
		iov.iov_base = array;
		iov.iov_len = BUFF1_SIZE;

		bool ok = u1.registerBuffers(&iov, 1);
		if (!ok) {std::cout << "main : Error from registerBuffers\n";return 0;}

		ok = u1.get_fd("/home/sbaillie/tags", fd, true);
		if (!ok) {std::cout << "main : Error from get_fd\n";return 0;}

		ok = u1.registerFiles(&fd,1);
		if (!ok) {std::cout << "main : Error from registerFiles\n";return 0;}

		for (uint32_t u0=0; u0<1; u0++) {
//			bool ok = u1.read(fd, array, BUFF1_SIZE, 0, UringHandlerPtr(new Test1UringHandler));
			bool ok = u1.read(fd, (unsigned)0, BUFF1_SIZE, 0, UringHandlerPtr(new Test1UringHandler));
			if (!ok) {std::cout << "main : Error from read\n";return 0;}
		}

		::signal(SIGTERM, signal_handler);
		::signal(SIGINT, signal_handler);

		while (!g_stopped) {
			::usleep(1000);
		}
	}

	::free(array);

	std::cout << "main : exit OK\n";

	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
