//////////////////////////////////////////////////////////////////////////////

#include <Network/ConnectionHandler/CommandHandler.h>

using namespace msglib;

//////////////////////////////////////////////////////////////////////////////

std::mutex g_mutex;

//////////////////////////////////////////////////////////////////////////////

void
ServerCommandHandler::onProtobufMessageReceived()
{
	std::string request = m_msg.request().request();

	std::vector<std::string> responseStr;

	{
	std::scoped_lock<std::mutex> lock(g_mutex);
	responseStr = m_runCommand.runCommand(request);
	}

	Command command;
	Response response;
	command.set_type(2);

	for (auto s : responseStr) {
		std::string* p = response.add_response();
		*p = s;
	}

	*command.mutable_response() = response;

	sendMessage(command);
}

////////////////////////////////////////////////////////////////////////////// 

void
ClientCommandHandler::onConnectionAccepted()
{
	Command command;
	Request request;
	command.set_type(1);

	request.set_request(m_request);

	*command.mutable_request() = request;

	sendMessage(command);
}

////////////////////////////////////////////////////////////////////////////// 

void
ClientCommandHandler::onProtobufMessageReceived()
{
	int size = m_msg.response().response_size();

	for (int i=0; i<size; i++) {
		const std::string & str = m_msg.response().response(i);
		std::cout << str << "\n";
	}

	shutdown();
}

////////////////////////////////////////////////////////////////////////////// 
