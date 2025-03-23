//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_COMMANDSERVER_H
#define MSGLIB_COMMANDSERVER_H

//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/CommandHandler.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class CommandServer
{
public:
	bool	start(RunCommand & runCommand, const IpPort & ipPort);

	void	stop();

private:
	ConnectionManager	m_mgr;
};

//////////////////////////////////////////////////////////////////////////////

}

#endif 
