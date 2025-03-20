////////////////////////////////////////////////////////////////

#include <cstddef>

#include <stdint.h>

////////////////////////////////////////////////////////////////

class Alloc
{
public:
	Alloc();
	Alloc(void * base, const size_t blockSize, const size_t numblocks);
	Alloc(const size_t blockSize, const size_t numblocks);

	~Alloc();

	void init(void * base, const size_t blockSize, const size_t numblocks);
	void init(const size_t blockSize, const size_t numblocks);

	void * alloc(const size_t size);
	void free(void * p);

	bool isValid(void * p) const;

private:
	size_t			m_blockSize = 0;
	size_t			m_numblocks = 0;
	std::vector<void*>	m_freeList;
	size_t			m_freeListSize = 0;
	void *			m_base = 0;
	bool			m_freeOnDestroy = false;
};

////////////////////////////////////////////////////////////////

// key is blockSize : data is numblocks.
typedef std::map<size_t,size_t> BlockInfoMap;

////////////////////////////////////////////////////////////////

class AllocGroup
{
public:
	AllocGroup(const BlockInfoMap & blockInfoMap);

	void * alloc(const size_t size);
	void free(void * p);

private:
	void *			m_base = 0;
	std::vector<Alloc>	m_allocList;
	BlockInfoMap		m_blockInfoMap;
};

////////////////////////////////////////////////////////////////

