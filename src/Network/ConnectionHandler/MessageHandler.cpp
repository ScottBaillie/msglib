//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/MessageHandler.h>

using namespace msglib;

//////////////////////////////////////////////////////////////////////////////

void 
MessageHandler::onDataReceived()
{
	size_t size = m_buffer.getReadSize();
	if (size<sizeof(uint64_t)) return;
	uint8_t * p = m_buffer.getReadPtr();
	uint64_t len = be64toh(*(uint64_t*)p);
	uint64_t tlen = sizeof(uint64_t)+len;
	if (size < tlen) return;
	uint8_t * pend = p + size;

	while (p < pend) {

		onMessageReceived(p, len);

		m_buffer.advanceRead(tlen);
		p = p + tlen;

		size = m_buffer.getReadSize();
		if (size<sizeof(uint64_t)) break;
		len = be64toh(*(uint64_t*)p);
		tlen = sizeof(uint64_t)+len;
		if (size < tlen) break;
	}
}

////////////////////////////////////////////////////////////////////////////// 

bool
MessageHandler::sendMessage(uint8_t * p, const size_t len)
{
	uint64_t l = htobe64(len);
	write((uint8_t*)&l, sizeof(uint64_t));
	bool ok = write(p, len);
	return ok;
}

////////////////////////////////////////////////////////////////////////////// 
