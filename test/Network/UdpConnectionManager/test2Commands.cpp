//////////////////////////////////////////////////////////////////////////////

#include "test2.h"

//////////////////////////////////////////////////////////////////////////////

void
App::udpserver_add(std::vector<std::string> & tokens, std::vector<std::string> & response)
{
	if (tokens.size() != 3) {
		response.push_back("Incorrect number of arguments");
		return;
	}

	Test2UdpConnectionHandlerPtr hlr(new Test2UdpConnectionHandler);

	UdpConnectionData data;

	data.hlr = hlr;
	bool ok = data.ipPort.addr.set(tokens[1]);
	if (!ok) {
		response.push_back("Invalid IP address");
		return;
	}
	data.ipPort.setPort(::atoi(tokens[2].c_str()));

	ok = m_mgr.add(data);
	if (!ok) {
		response.push_back("Error adding entry : " + data.ipPort.getString());
		return;
	}

	m_hlrmap[data.ipPort] = hlr;

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::udpserver_close(std::vector<std::string> & tokens, std::vector<std::string> & response)
{
	if (tokens.size() != 3) {
		response.push_back("Incorrect number of arguments");
		return;
	}

	IpPort ipPort;

	bool ok = ipPort.addr.set(tokens[1]);
	if (!ok) {
		response.push_back("Invalid IP address");
		return;
	}
	ipPort.setPort(::atoi(tokens[2].c_str()));

	if (m_hlrmap.find(ipPort) == m_hlrmap.end()) {
		response.push_back("Entry not found");
		return;
	}

	m_mgr.remove(m_hlrmap[ipPort]->m_fd);

	m_hlrmap.erase(ipPort);

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::list(std::vector<std::string> & tokens, std::vector<std::string> & response)
{
	if (tokens.size() != 1) {
		response.push_back("Incorrect number of arguments");
		return;
	}

	for (auto i : m_hlrmap) {
		response.push_back("Listening : " + i.first.getString());
	}

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::udpserver_send(std::vector<std::string> & tokens, std::vector<std::string> & response)
{
	if (tokens.size() != 5) {
		response.push_back("Incorrect number of arguments");
		return;
	}

	IpPort lipPort;
	IpPort ripPort;

	bool ok = lipPort.addr.set(tokens[1]);
	if (!ok) {
		response.push_back("Invalid IP address");
		return;
	}
	lipPort.setPort(::atoi(tokens[2].c_str()));

	ok = ripPort.addr.set(tokens[3]);
	if (!ok) {
		response.push_back("Invalid IP address");
		return;
	}
	ripPort.setPort(::atoi(tokens[4].c_str()));

	if (m_hlrmap.find(lipPort) == m_hlrmap.end()) {
		response.push_back("Entry not found");
		return;
	}

	std::shared_ptr<TestUserData> ud(new TestUserData);

	ud->ipPort = ripPort;

	m_hlrmap[lipPort]->postUserData(m_hlrmap[lipPort], ud, false);

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::udpclient_sendanon(std::vector<std::string> & tokens, std::vector<std::string> & response)
{
	if (tokens.size() != 3) {
		response.push_back("Incorrect number of arguments");
		return;
	}

	IpPort ipPort;

	bool ok = ipPort.addr.set(tokens[1]);
	if (!ok) {
		response.push_back("Invalid IP address");
		return;
	}
	ipPort.setPort(::atoi(tokens[2].c_str()));

	std::vector<uint8_t> buffer(128);

	ok = UdpConnectionHandler::sendMessageAnon(ipPort, buffer.data(), buffer.size());
	if (!ok) {
		response.push_back("Error sending packet");
		return;
	}

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////
