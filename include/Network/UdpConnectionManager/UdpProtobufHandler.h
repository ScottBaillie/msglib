//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_UDPPROTOBUFHANDLER_H
#define MSGLIB_UDPPROTOBUFHANDLER_H

//////////////////////////////////////////////////////////////////////////////

#include <Network/UdpConnectionManager/UdpConnectionManager.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

template <class T>
class UdpProtobufHandler : public UdpConnectionHandler
{
public:
	virtual void onProtobufMessageReceived(const IpPort & peer) = 0;

	bool sendMessage(const IpPort & ipPort, google::protobuf::Message & msg, const bool anon=false)
	{
		std::string data;
		bool ok = msg.SerializeToString(&data);
		if (!ok) return false;
		if (anon)
			ok = sendMessageAnon(ipPort, data.data(),data.size());
		else
			ok = sendMessage(ipPort, data.data(),data.size());
		return ok;
	}

private:
	virtual void onMessageReceived(const IpPort & peer, uint8_t * p, const size_t len)
	{
		char * begin = (char*)p + sizeof(uint64_t);
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
