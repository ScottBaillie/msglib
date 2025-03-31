//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_PROTOBUFHANDLER_H
#define MSGLIB_PROTOBUFHANDLER_H

//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/MessageHandler.h>

#include <google/protobuf/message.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

template <class T>
class ProtobufHandler : public MessageHandler
{
public:
	virtual ~ProtobufHandler() {}

	virtual void onConnectionAccepted() {}

	virtual void onConnectionTerminated() {}

	virtual void onProtobufMessageReceived() = 0;

	virtual void onUserData(MsglibDataPtr data) = 0;

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
		char * begin = (char*)p;
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

}

#endif
