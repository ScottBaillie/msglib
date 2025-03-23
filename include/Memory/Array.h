//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_ARRAY_H
#define MSGLIB_ARRAY_H

//////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class Array
{
public:
	Array(const uint64_t size=0);
	Array(void * p, const uint64_t size, const bool freeOnDestroy=false);

	~Array();

	// All of these functions will free the array first if it is allocated.
	void init(const uint64_t size);
	void init(void * p, const uint64_t size, const bool freeOnDestroy=false);
	void init(char * str);
	void init(const std::string_view & str);
	void init(const std::vector<uint8_t> & vec);

	// If array has nonzero size : copy contents, dont free and allocate a new array or change the size.
	//                           : copy as much data as possible if arrays have different sizes.
	// If array has zero size    : allocate a new array having same size as argument.
	void assign(const Array & array);

	uint64_t size() const;
	uint8_t * data();

	void fill(const uint8_t value);

	// A length of zero indicates the end index is the end of the array.
	Array subArray(const uint64_t startIndex=0, const uint64_t length=0);

	const uint8_t & operator [] ( uint64_t index ) const;
	uint8_t & operator [] ( uint64_t index );

	operator bool () const;

private:
	uint8_t *	m_array = 0;
	uint64_t	m_size = 0;
	bool		m_freeOnDestroy = false;
};

//////////////////////////////////////////////////////////////////////////////

}

#endif
