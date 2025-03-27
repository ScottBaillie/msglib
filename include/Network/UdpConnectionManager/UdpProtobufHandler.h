//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_UDPPROTOBUFHANDLER_H
#define MSGLIB_UDPPROTOBUFHANDLER_H

//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>

#include <google/protobuf/message.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

template <class T>
class UdpProtobufHandler : public UdpConnectionHandler
{
public:
	virtual void onConnectionAccepted() {}

	virtual void onConnectionTerminated() {}

	virtual void onProtobufMessageReceived(const IpPort & peer) = 0;

	virtual void onUserData(MsglibDataPtr data) = 0;

	virtual void onError(int error) {}

	virtual void onTimer(uint64_t time) {}

	bool sendMessage(const IpPort & ipPort, google::protobuf::Message & msg, const bool anon=false)
	{
		std::string data;
		bool ok = msg.SerializeToString(&data);
		if (!ok) return false;
		if (anon)
			ok = UdpConnectionHandler::sendMessageAnon(ipPort, (uint8_t*)data.data(),data.size());
		else
			ok = UdpConnectionHandler::sendMessage(ipPort, (uint8_t*)data.data(),data.size());
		return ok;
	}

private:
	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		char * begin = (char*)p;
		char * end = begin + len;
		std::string data(begin,end);
		bool ok = m_msg.ParseFromString(data);
		if (!ok) {
			std::cout << "ProtobufHandler::onMessageReceived : Error from ParseFromString()\n";
		} else {
			onProtobufMessageReceived(peer);
		}
	}

public:
	T	m_msg;
};

//////////////////////////////////////////////////////////////////////////////

}

#endif
