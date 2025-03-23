//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_BUFFER_H
#define MSGLIB_BUFFER_H

//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>
#include <cstdint>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class Buffer
{
public:
	Buffer(const size_t size = 0) {setSize(size);}
	~Buffer() {clear();}

	void		setSize(const size_t size);
	size_t		getSize() const {return m_size;}

	void		clear();

	uint8_t *	getReadPtr() const;
	uint8_t *	getWritePtr() const;

	bool		advanceRead(const size_t nbytes);
	bool		advanceWrite(const size_t nbytes);

	size_t		getReadSize() const;
	size_t		getWriteSize() const;

	// Have to call getReadPtr and getWritePtr after this to get up to date pointers.
	void		compact();

private:
	size_t		m_size = 0;
	size_t		m_rd = 0;
	size_t		m_wr = 0;
	uint8_t *	m_buffer = 0;
};

//////////////////////////////////////////////////////////////////////////////

}

#endif 
