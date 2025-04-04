//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_COMMANDHANDLER_H
#define MSGLIB_COMMANDHANDLER_H

//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/ProtobufHandler.h>

#include "gen/command.pb.h"

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class RunCommand
{
public:
	virtual ~RunCommand() {}

	virtual std::vector<std::string> runCommand(const std::string & command) = 0;
};

//////////////////////////////////////////////////////////////////////////////

class ServerCommandHandler : public ProtobufHandler<Command>
{
public:
	virtual ~ServerCommandHandler() {}

	ServerCommandHandler(RunCommand & runCommand)
		: m_runCommand(runCommand)
	{
		m_buffer.setSize(65536);
	}

	virtual void onProtobufMessageReceived();

	virtual void onUserData(MsglibDataPtr data) {}

	virtual std::shared_ptr<ConnectionHandler> clone()
	{
		std::shared_ptr<ConnectionHandler> ret(new ServerCommandHandler(m_runCommand));
		return ret;
	}

private:
	RunCommand &	m_runCommand;
};

//////////////////////////////////////////////////////////////////////////////

class ClientCommandHandler : public ProtobufHandler<Command>
{
public:
	virtual ~ClientCommandHandler() {}

	ClientCommandHandler(const std::string & request)
		: m_request(request)
	{
		m_buffer.setSize(65536);
	}

	virtual void onConnectionAccepted();

	virtual void onProtobufMessageReceived();

	virtual void onUserData(MsglibDataPtr data) {}

private:
	std::string	m_request;
};

//////////////////////////////////////////////////////////////////////////////

}

#endif
