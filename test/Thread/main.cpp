//////////////////////////////////////////////////////////////////////////////

#include <Thread/ThreadPool.h>

using namespace msglib;

#include <iostream>

#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////

class Test1UserData : public MsglibData
{
public:
	int		m_int = 1234;
	std::string	m_str = "Test1-test-string";
};

//////////////////////////////////////////////////////////////////////////////

class Test1aThreadConnectionHandler : public ThreadConnectionHandler
{
public:
	virtual ~Test1aThreadConnectionHandler() {}

	virtual void onUserData(MsglibDataPtr data)
	{
		std::cout << "Test1aThreadConnectionHandler::onUserData : Entered\n";
	}
};

//////////////////////////////////////////////////////////////////////////////

class Test1bThreadConnectionHandler : public ThreadConnectionHandler
{
public:
	virtual ~Test1bThreadConnectionHandler() {}

	virtual void onUserData(MsglibDataPtr data)
	{
		std::cout << "Test1bThreadConnectionHandler::onUserData : Entered\n";
	}
};

//////////////////////////////////////////////////////////////////////////////

int
test1(int argc, char * argv[])
{
	bool ok;
	ThreadPool threadPool(10);

	ThreadConnectionHandlerPtr hlra(new Test1aThreadConnectionHandler);
	ThreadConnectionHandlerPtr hlrb(new Test1bThreadConnectionHandler);
	MsglibDataPtr data;

	data.reset(new Test1UserData);
	ok = threadPool.postUserData(hlra, data, false);
	if (!ok) std::cout << "test1 : Error from postUserData()\n";


	data.reset(new Test1UserData);
	ok = threadPool.postUserData(hlrb, data, false);
	if (!ok) std::cout << "test1 : Error from postUserData()\n";

	::usleep(1000000*4);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret = 0;

	ret = test1(argc, argv);

	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
