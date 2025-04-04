//////////////////////////////////////////////////////////////////////////////

#include <Network/Command/CommandServer.h>

using namespace msglib;

#include <iostream>

//////////////////////////////////////////////////////////////////////////////

bool
CommandServer::start(RunCommand & runCommand, const IpPort & ipPort)
{
	bool ok;
	ConnectionData data;

	data.ipPort = ipPort;
	data.hlr = ConnectionHandlerPtr(new ServerCommandHandler(runCommand));
	data.server = true;

	ok = m_mgr.add(data);
	if (!ok) {
		std::cout << "CommandServer::start : Error from m_mgr.add()\n";
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

void
CommandServer::stop()
{
	m_mgr.stop();
}

//////////////////////////////////////////////////////////////////////////////
 
