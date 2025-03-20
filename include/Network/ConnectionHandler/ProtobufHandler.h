//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/MessageHandler.h>

#include <google/protobuf/message.h>

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

template <class T>
class ProtobufHandler : public MessageHandler
{
public:
	virtual ~ProtobufHandler() {}

	virtual void onConnectionAccepted() {}

	virtual void onConnectionTerminated() {}

	virtual void onProtobufMessageReceived() = 0;

	virtual void onError(int error) {}

	virtual void onTimer(uint64_t time) {}

	virtual std::shared_ptr<ConnectionHandler> clone() {return std::shared_ptr<ConnectionHandler>();}

	bool sendMessage(google::protobuf::Message & msg)
	{
		std::string data;
		bool ok = msg.SerializeToString(&data);
		if (!ok) return false;
		ok = MessageHandler::sendMessage((uint8_t*)data.data(),data.size());
		return ok;
	}

private:
	virtual void onMessageReceived(uint8_t * p, const size_t len)
	{
		char * begin = (char*)p + sizeof(uint64_t);
		char * end = begin + len;
		std::string data(begin,end);
		bool ok = m_msg.ParseFromString(data);
		if (!ok) {
			std::cout << "ProtobufHandler::onMessageReceived : Error from ParseFromString()\n";
		} else {
			onProtobufMessageReceived();
		}
	}

public:
	T	m_msg;
};

//////////////////////////////////////////////////////////////////////////////

