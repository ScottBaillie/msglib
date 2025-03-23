//////////////////////////////////////////////////////////////////////////////

#include <Network/Buffer/Buffer.h>

using namespace msglib;

#include <cstdlib>

//////////////////////////////////////////////////////////////////////////////

void
Buffer::setSize(const size_t size)
{
	if (size==0) {clear();return;}
	if (m_buffer) ::free(m_buffer);
	m_buffer = (uint8_t*)::malloc(size);
	m_size = size;
	m_rd = 0;
	m_wr = 0;
}

void
Buffer::clear()
{
	if (m_buffer) ::free(m_buffer);
	m_size = 0;
	m_rd = 0;
	m_wr = 0;
	m_buffer = 0;
}

uint8_t *
Buffer::getReadPtr() const
{
	if (m_buffer==0) {return 0;}
	if (m_rd>=m_size) {return 0;}
	uint8_t * p = m_buffer + m_rd;
	return p;
}

uint8_t *
Buffer::getWritePtr() const
{
	if (m_buffer==0) {return 0;}
	if (m_wr>=m_size) {return 0;}
	uint8_t * p = m_buffer + m_wr;
	return p;
}

bool
Buffer::advanceRead(const size_t nbytes)
{
	size_t rd = m_rd + nbytes;
	if (rd>m_size) return false;
	m_rd = rd;
	return true;
}

bool
Buffer::advanceWrite(const size_t nbytes)
{
	size_t wr = m_wr + nbytes;
	if (wr>m_size) return false;
	m_wr = wr;
	return true;
}

size_t
Buffer::getReadSize() const
{
	return m_wr-m_rd;
}

size_t
Buffer::getWriteSize() const
{
	if (m_wr > m_size) return 0;
	return m_size-m_wr;
}

// Have to call getReadPtr and getWritePtr after this to get up to date pointers.
void
Buffer::compact()
{
	if (m_buffer==0) return;
	if (m_rd==0) return;
	if (m_rd==m_wr) {
		m_rd = 0;
		m_wr = 0;
		return;
	}

	size_t num = m_wr - m_rd;

	::memmove(m_buffer, m_buffer+m_rd, num);

	m_rd = 0;
	m_wr = num;
}

//////////////////////////////////////////////////////////////////////////////

