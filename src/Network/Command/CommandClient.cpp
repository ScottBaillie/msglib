//////////////////////////////////////////////////////////////////////////////

#include <Network/Command/CommandClient.h>

using namespace msglib;

#include <iostream>

//////////////////////////////////////////////////////////////////////////////

bool
CommandClient::start(const std::string & request, const IpPort & ipPort)
{
	bool ok;
	ConnectionData data;

	data.ipPort = ipPort;
	data.hlr = ConnectionHandlerPtr(new ClientCommandHandler(request));
	data.server = false;

	ok = m_mgr.add(data);
	if (!ok) {
		std::cout << "CommandClient::start : Error from m_mgr.add()\n";
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

 
