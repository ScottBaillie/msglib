//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_MEMPROTOBUFHANDLER_H
#define MSGLIB_MEMPROTOBUFHANDLER_H

//////////////////////////////////////////////////////////////////////////////

#include <Memory/MemConnectionManager.h>

#include <google/protobuf/message.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

template <class T>
class MemProtobufHandler : public MemConnectionHandler
{
public:
	virtual ~MemProtobufHandler() {}

	virtual void onConnectionAccepted() {}

	virtual void onConnectionTerminated() {}

	virtual void onProtobufMessageReceived() = 0;

	virtual void onUserData(MsglibDataPtr data) = 0;

	virtual void onTimer(uint64_t time) {}

	bool sendMessage(const std::string & bufferName, google::protobuf::Message & msg)
	{
		std::string data;
		bool ok = msg.SerializeToString(&data);
		if (!ok) return false;
		ok = MemConnectionHandler::sendMessage(bufferName, (uint8_t*)data.data(), data.size());
		return ok;
	}

	bool sendMessage(uint8_t * buffer, google::protobuf::Message & msg)
	{
		std::string data;
		bool ok = msg.SerializeToString(&data);
		if (!ok) return false;
		ok = MemConnectionHandler::sendMessage(buffer, (uint8_t*)data.data(), data.size());
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
			std::cout << "MemProtobufHandler::onMessageReceived : Error from ParseFromString()\n";
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
