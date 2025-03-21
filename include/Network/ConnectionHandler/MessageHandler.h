//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionManager/ConnectionManager.h>

#include <memory>
#include <string>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////
//
// Datagrams are formatted in memory as a header followed by a variable length data block.
// The format of a datagram in memory is as follows :
//
// uint64_t length; // Header 
// uint8_t  data[]; // block of data.
//
//
//////////////////////////////////////////////////////////////////////////////

class MessageHandler : public ConnectionHandler
{
public:
	virtual ~MessageHandler() {}

	virtual void onConnectionAccepted() {}

	virtual void onConnectionTerminated() {}

	virtual void onMessageReceived(uint8_t * p, const size_t len) = 0;

	virtual void onError(int error) {}

	virtual void onTimer(uint64_t time) {}

	virtual std::shared_ptr<ConnectionHandler> clone() {return std::shared_ptr<ConnectionHandler>();}

	bool sendMessage(uint8_t * p, const size_t len);

private:
	// The method will consume all data in Buffer and call onMessageReceived() multiple times.
	// When onMessageReceived() returns, this method will advance the read ptr of the buffer.
	virtual void onDataReceived();
};

//////////////////////////////////////////////////////////////////////////////

typedef std::shared_ptr<MessageHandler> MessageHandlerPtr;

//////////////////////////////////////////////////////////////////////////////

