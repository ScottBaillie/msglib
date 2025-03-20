//////////////////////////////////////////////////////////////////////////////

#include <LinkedList/LinkedList.h>

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
test1(int argc, char * argv[])
{
	class ListData
	{
	public:
		bool operator==(const ListData & ldata) const
		{
			if (start != ldata.start) return false;
			if (end != ldata.end) return false;
			if (alloc != ldata.alloc ) return false;
			return true;
		}
		bool operator!=(const ListData & ldata) const {return(!((*this) == ldata));}

	public:
		uint64_t start = 0;
		uint64_t end = 0;
		bool alloc = false;
	};

	class FindAllocFunction
	{
	public:
		bool find(ListData & data) const {return data.alloc;}
	};

	using namespace Primitives;

	List<ListData> l0;
	List<ListData> l1 = l0;
	List<ListData> l2;
	List<ListData> l3(l0);
	l2 = l0;
	l3 += l2;
	if (l2==l3) l0=l2;

	auto ptr = l0.insertAfter(l0.getTail());
	ptr->getData().start = 1;
	ptr->getData().end = 11;
	ptr->getData().alloc = true;

	ptr = l0.insertAfter(l0.getTail());
	ptr->getData().start = 2;
	ptr->getData().end = 12;
	ptr->getData().alloc = true;

	if (l0) l0.clear();

	ptr = l0.insertAfter(l0.getTail());
	ptr->getData().start = 3;
	ptr->getData().end = 13;
	ptr->getData().alloc = true;

	ptr = l0.insertBefore(l0.getHead());
	ptr->getData().start = 4;
	ptr->getData().end = 14;
	ptr->getData().alloc = false;

	ptr = l0.insertAfter(l0.getTail());
	ptr->getData().start = 5;
	ptr->getData().end = 15;
	ptr->getData().alloc = true;

	ptr = l0.insertAfter(l0.getHead());
	ptr->getData().start = 6;
	ptr->getData().end = 16;
	ptr->getData().alloc = false;

	ptr = l0.insertBefore(l0.getTail());
	ptr->getData().start = 7;
	ptr->getData().end = 17;
	ptr->getData().alloc = true;

	List<ListData>::VecPtr foundList;

	l0.findFwdAll(FindAllocFunction(),foundList);

	for (auto p : foundList) {
		auto & d = p->getData();
//		std::cout << "start=" << d.start << "\n";
	}

	l0.remove(foundList);

	foundList.clear();
	l0.findFwdAll(FindAllFunction<ListData>(),foundList);

	for (auto p : foundList) {
		auto & d = p->getData();
//		std::cout << "start=" << d.start << "\n";
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int
test2(int argc, char * argv[])
{
	struct ListData
	{
		uint32_t value = 0;
	};

	unsigned int seed = 0;
	::srand(seed);

	using namespace Primitives;

	int count = 0;
	int rnd1 = 0;
	int numEntries = 32;
	List<ListData> l;
	List<ListData>::Ptr ptr=0;

	for (uint32_t u=0; u<(1024*128); u++) {

		ptr=0;

		for (uint32_t u0=0; u0<numEntries; u0++) {

			rnd1 = ::rand();
			count = 0;
			for(auto p=l.getHead();p!=0;p=p->getNext()) {
				ptr = p;
				if (count==(rnd1%l.size())) break;
				count++;
			}

			rnd1 = ::rand();
			if ((rnd1%2)==0) {
				ptr = l.insertAfter(ptr);
				ptr->getData().value = u0;
			} else {
				ptr = l.insertBefore(ptr);
				ptr->getData().value = u0;
			}
		}

		std::set<uint32_t> valueSet;
		for(auto p=l.getHead();p!=0;p=p->getNext()) {
			valueSet.insert(p->getData().value);
		}
		if (valueSet.size() != numEntries) std::cout << "Values do not match 1\n";
		if (*valueSet.begin()!=0) std::cout << "Values do not match 2\n";
		if (*valueSet.rbegin()!=(numEntries-1)) std::cout << "Values do not match 3\n";

		ptr=0;

		for (uint32_t u0=0; u0<numEntries; u0++) {

			rnd1 = ::rand();
			count = 0;
			for(auto p=l.getHead();p!=0;p=p->getNext()) {
				ptr = p;
				if (count==(rnd1%l.size())) break;
				count++;
			}
			l.remove(ptr);
		}

		if (l.size() != 0) std::cout << "Not Empty\n";
	}

	std::cout << "Complete\n";

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret;

	ret = test1(argc, argv);
	ret = test2(argc, argv);

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
