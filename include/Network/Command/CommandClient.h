//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_COMMANDCLIENT_H
#define MSGLIB_COMMANDCLIENT_H

//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/CommandHandler.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class CommandClient
{
public:
	bool	start(const std::string & request, const IpPort & ipPort);

	void	stop() {m_mgr.stop();}

	bool	isStopped() {return m_mgr.isStopped();}

	void	wait() {m_mgr.wait();}

private:
	ConnectionManager	m_mgr;
};

//////////////////////////////////////////////////////////////////////////////

}

#endif
