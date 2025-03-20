//////////////////////////////////////////////////////////////////////////////

#include <Time/Time.h>

#include <random>
#include <atomic>
#include <iostream>
#include <map>
#include <unordered_map>

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret;

	std::map<Time,int> m1;
	std::unordered_map<Time,int> m2;

	Time t1;
	Time t2;
	t1.now();

	m1[t1] = 1;
	m2[t1] = 1;

	if (t1==t2) ret=0;
	if (t1!=t2) ret=0;
	if (t1>t2) ret=0;
	if (t1>=t2) ret=0;
	if (t1<t2) ret=0;
	if (t1<=t2) ret=0;

	t1 = t2;

	if (t1) t2 = t1;

	std::cout << "time=" << t1.getString() << "\n";

	bool ok = t1.set("Mon Mar 17 00:00:25 2025");
	if (!ok) std::cout << "Error from set\n";

	std::cout << "time=" << t1.getString() << "\n";

	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
