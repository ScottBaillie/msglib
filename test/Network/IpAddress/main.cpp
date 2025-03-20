//////////////////////////////////////////////////////////////////////////////

#include <Network/IpAddress/IpAddress.h>

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

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret = 0;

	IpAddress addr1;
	IpAddress addr2("12.34.56.78");
	IpAddress addr3("1.2.3.4");
	IpAddress addr4("255.255.255.255");
	IpAddress addr5("2001:db8:3333:4444:5555:6666:7777:8888");
	IpAddress addr6("fe80::1");

	std::cout << "addr2=" << addr2.getString() << "\n";
	std::cout << "addr3=" << addr3.getString() << "\n";
	std::cout << "addr4=" << addr4.getString() << "\n";
	std::cout << "addr5=" << addr5.getString() << "\n";
	std::cout << "addr6=" << addr6.getString() << "\n";

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
