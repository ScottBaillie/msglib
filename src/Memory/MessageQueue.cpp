//////////////////////////////////////////////////////////////////////////////

#include <Memory/MessageQueue.h>

//////////////////////////////////////////////////////////////////////////////

MessageQueue::MessageQueue(uint8_t * buffer)
{
	init(buffer);
}

//////////////////////////////////////////////////////////////////////////////

bool
MessageQueue::init(uint8_t * buffer, const uint64_t size)
{
	m_mq = (MessageQueueHeaderPtr)buffer;
	m_mq->m_magic = 0x1122334455667788;
	m_mq->m_size = size - sizeof(struct MessageQueueHeader);
	m_mq->m_rd = 0;
	m_mq->m_wr = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////////

void
MessageQueue::init(uint8_t * buffer)
{
	m_mq = (MessageQueueHeaderPtr)buffer;
}

//////////////////////////////////////////////////////////////////////////////

uint64_t
MessageQueue::avail()
{
	auto rd = m_mq->m_rd;
	auto wr = m_mq->m_wr;
	if (rd == wr) return(0);
	if (wr > rd) return(wr - rd);
	return(m_mq->m_size + wr - rd);
}

//////////////////////////////////////////////////////////////////////////////

uint64_t
MessageQueue::free()
{
	return m_mq->m_size - 1 - avail();
}

//////////////////////////////////////////////////////////////////////////////

uint64_t
MessageQueue::size()
{
	return(m_mq->m_size);
}

//////////////////////////////////////////////////////////////////////////////

bool
MessageQueue::add(const BytePtr p, const uint64_t size, const bool useLock)
{
	uint64_t messageSize = size + sizeof(uint64_t);

	if (messageSize > free()) {
		return false;
	}

	auto & qsize = m_mq->m_size;
	auto rd = m_mq->m_rd;
	auto wr = m_mq->m_wr;

	uint64_t len = size;
	uint8_t * plen = (uint8_t *)&len;

	for (uint32_t u0=0; u0<sizeof(uint64_t); u0++) {
		m_mq->m_queue[wr] = plen[u0];
		wr = (wr + 1) % qsize;
	}

	uint64_t head = qsize - wr;

	BytePtr dst = &m_mq->m_queue[wr];

	if (size > head) {
		BytePtr base = &m_mq->m_queue[0];
		memcpy(dst,  p, head);
		memcpy(base, &p[head], size-head);
	} else {
		memcpy(dst, p, size);
	}

	wr = (wr + size) % qsize;
	m_mq->m_wr = wr;

	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
MessageQueue::addBegin(BytePtr & p, const uint64_t size)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
MessageQueue::addEnd(const uint64_t size)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
MessageQueue::get(BytePtr & p, uint64_t & size)
{
	auto & qsize = m_mq->m_size;
	auto rd = m_mq->m_rd;
	auto wr = m_mq->m_wr;

	if (rd == wr) {
		return false;
	}

	uint64_t avail = this->avail();
	if (avail<sizeof(uint64_t)) return false;
	uint64_t len = 0;
	uint8_t * plen = (uint8_t *)&len;

	for (uint32_t u0=0; u0<sizeof(uint64_t); u0++) {
		plen[u0] = m_mq->m_queue[rd];
		rd = (rd + 1) % qsize;
	}

	uint64_t messageSize = len + sizeof(uint64_t);
	if (avail<messageSize) return false;

	uint64_t head = qsize - rd;

	if (len > head) {
		m_getBuffer.resize(len);
		BytePtr src = &m_mq->m_queue[rd];
		BytePtr dst = m_getBuffer.data();
		memcpy(dst,  src, head);
		memcpy(dst+head, &m_mq->m_queue[0], len-head);
		p = m_getBuffer.data();
		size = len;
	} else {
		p = &m_mq->m_queue[rd];
		size = len;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
MessageQueue::release(const uint64_t size)
{
	auto & qsize = m_mq->m_size;
	auto rd = m_mq->m_rd;

	m_getBuffer.clear();

	uint64_t messageSize = size + sizeof(uint64_t);

	rd = (rd + messageSize) % qsize;
	m_mq->m_rd = rd;

	return true;
}

////////////////////////////////////////////////////////////////////////////// 
