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

	if (m_udphlrmap.find(data.ipPort) != m_udphlrmap.end()) {
		response.push_back("Entry already exists");
		return;
	}

	ok = m_udpmgr.add(data);
	if (!ok) {
		response.push_back("Error adding entry : " + data.ipPort.getString());
		return;
	}

	m_udphlrmap[data.ipPort] = hlr;

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

	if (m_udphlrmap.find(ipPort) == m_udphlrmap.end()) {
		response.push_back("Entry not found");
		return;
	}

	m_udpmgr.remove(m_udphlrmap[ipPort]->m_fd);

	m_udphlrmap.erase(ipPort);

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

	for (auto i : m_udphlrmap) {
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

	if (m_udphlrmap.find(lipPort) == m_udphlrmap.end()) {
		response.push_back("Entry not found");
		return;
	}

	std::shared_ptr<UdpUserData> ud(new UdpUserData);

	ud->ipPort = ripPort;

	ok = m_udphlrmap[lipPort]->postUserData(m_udphlrmap[lipPort], ud, false);
	if (!ok) {
		response.push_back("Error posting message to handler");
		return;
	}

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

void
App::tcpserver_add(std::vector<std::string> & tokens, std::vector<std::string> & response)
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

	if (m_tcpservermap.find(ipPort) != m_tcpservermap.end()) {
		response.push_back("Entry already exists");
		return;
	}

	TcpServerConnectionHandlerPtr hlr(new TcpServerConnectionHandler);
	ConnectionData data;

	data.ipPort = ipPort;
	data.hlr = hlr;
	data.server = true;

	ok = m_tcpmgr.add(data);
	if (!ok) {
		response.push_back("Error adding server connection");
		return;
	}

	m_tcpservermap[ipPort] = hlr;

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::tcpclient_add(std::vector<std::string> & tokens, std::vector<std::string> & response)
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

	if (m_tcpclientmap.find(ipPort) != m_tcpclientmap.end()) {
		response.push_back("Entry already exists");
		return;
	}

	TcpClientConnectionHandlerPtr hlr(new TcpClientConnectionHandler);
	ConnectionData data;

	data.ipPort = ipPort;
	data.hlr = hlr;
	data.server = false;

	ok = m_tcpmgr.add(data);
	if (!ok) {
		response.push_back("Error adding client connection");
		return;
	}

	m_tcpclientmap[ipPort] = hlr;

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::tcpclient_send(std::vector<std::string> & tokens, std::vector<std::string> & response)
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

	if (m_tcpclientmap.find(ipPort) == m_tcpclientmap.end()) {
		response.push_back("Entry not found");
		return;
	}

	std::shared_ptr<TcpUserData> ud(new TcpUserData);

	ok = m_tcpclientmap[ipPort]->postUserData(m_tcpclientmap[ipPort], ud, false);
	if (!ok) {
		response.push_back("Error posting message to handler");
		return;
	}

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::tcpserver_close(std::vector<std::string> & tokens, std::vector<std::string> & response)
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

	if (m_tcpservermap.find(ipPort) == m_tcpservermap.end()) {
		response.push_back("Entry not found");
		return;
	}

	m_tcpservermap.erase(ipPort);

	ok = m_tcpmgr.removeServer(ipPort);
	if (!ok) {
		response.push_back("Error removing server");
		return;
	}

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////

void
App::tcpclient_close(std::vector<std::string> & tokens, std::vector<std::string> & response)
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

	if (m_tcpclientmap.find(ipPort) == m_tcpclientmap.end()) {
		response.push_back("Entry not found");
		return;
	}

	std::shared_ptr<TcpUserData> ud(new TcpUserData);

	ud->m_close = true;

	ok = m_tcpclientmap[ipPort]->postUserData(m_tcpclientmap[ipPort], ud, false);
	if (!ok) {
		response.push_back("Error posting message to handler");
		return;
	}

	m_tcpclientmap.erase(ipPort);

	response.push_back("OK");
}

//////////////////////////////////////////////////////////////////////////////
