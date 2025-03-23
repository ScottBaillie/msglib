//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_MESSAGEQUEUE_H
#define MSGLIB_MESSAGEQUEUE_H

//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include <mutex>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

struct MessageQueueHeader
{
	uint64_t	m_magic = 0;
	uint64_t	m_size = 0;	// length of m_queue[] array of bytes.
	uint64_t	m_rd = 0;
	uint64_t	m_wr = 0;
	uint8_t		m_queue[0];
};

//////////////////////////////////////////////////////////////////////////////

typedef struct MessageQueueHeader * MessageQueueHeaderPtr;

typedef uint8_t * BytePtr;

///////////////////////////////////////////////////////////////

class MessageQueue
{
public:
	MessageQueue() {}
	MessageQueue(uint8_t * buffer);

	bool init(uint8_t * buffer, const uint64_t size);

	void init(uint8_t * buffer);

	uint64_t avail();

	uint64_t free();

	uint64_t size();

	bool add(const BytePtr p, const uint64_t size, const bool useLock);  // copies from p to queue.

	bool addBegin(BytePtr & p, const uint64_t size);	// no copying (unless message would wrap around buffer). Not reentrant.

	bool addEnd(const uint64_t size);	// Call after addBegin(). Not reentrant.

	bool get(BytePtr & p, uint64_t & size);

	bool release(const uint64_t size);	// If get() returns true, this function must be called when finished reading data.

private:
	std::mutex			m_mutex;
	MessageQueueHeaderPtr		m_mq = 0;
	std::vector<uint8_t>		m_getBuffer;
	std::vector<uint8_t>		m_addBuffer;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<MessageQueue> MessageQueuePtr;

//////////////////////////////////////////////////////////////////////////////

}

#endif
