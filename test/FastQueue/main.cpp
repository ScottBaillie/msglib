//////////////////////////////////////////////////////////////////////////////

#include <FastQueue/FastQueue.h>

using namespace msglib;

#include <iostream>
#include <random>
#include <set>

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret = 0;

	unsigned int seed = 0;
	::srand(seed);

	struct QueueEntry
	{
		unsigned value = 0;
	};

	std::set<unsigned> entrySet;
	FastQueue<QueueEntry> q(128);
	bool ok;
	unsigned n;

	for (unsigned u=0; u<(65536*8); u++) {
		n = (::rand()%64) + 64;

		for (unsigned u0=0; u0<n; u0++) {
			QueueEntry* p = q.next();
			p->value = u0;
			ok = q.add();
			if (!ok) {
				std::cout << "Error from add()\n";
				return 0;
			}
		}

		entrySet.clear();

		for (unsigned u0=0; u0<n; u0++) {
			QueueEntry* p = q.get();
			entrySet.insert(p->value);
			q.release();
		}

		if (q.getNumAvail() != 0) {
			std::cout << "Queue not empty\n";
			return 0;
		}
		if (entrySet.size() != n) {
			std::cout << "entrySet has wrong size\n";
			return 0;
		}
		if (*entrySet.begin() != 0) {
			std::cout << "entrySet has wrong begin\n";
			return 0;
		}
		if (*entrySet.rbegin() != (n-1)) {
			std::cout << "entrySet has wrong end\n";
			return 0;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
