//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_IPADDRESS_H
#define MSGLIB_IPADDRESS_H

//////////////////////////////////////////////////////////////////////////////

#include <string>

#include <cstdint>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

typedef unsigned __int128 uint128_t;

//////////////////////////////////////////////////////////////////////////////

// struct in_addr {
//     in_addr_t s_addr;
// };
//
// struct in6_addr {
//     uint8_t   s6_addr[16];
// };
//
// struct sockaddr_in {
//     sa_family_t     sin_family;     /* AF_INET */
//     in_port_t       sin_port;       /* Port number */
//     struct in_addr  sin_addr;       /* IPv4 address */
// };
//
// struct sockaddr_in6 {
//     sa_family_t     sin6_family;    /* AF_INET6 */
//     in_port_t       sin6_port;      /* Port number */
//     uint32_t        sin6_flowinfo;  /* IPv6 flow info */
//     struct in6_addr sin6_addr;      /* IPv6 address */
//     uint32_t        sin6_scope_id;  /* Set of interfaces for a scope */
// };

//////////////////////////////////////////////////////////////////////////////

class IpAddress
{
public:
	IpAddress() {}
	IpAddress(const std::string & addr)		{set(addr);}
	IpAddress(const struct sockaddr_in & addr)	{set(addr);}
	IpAddress(const struct sockaddr_in6 & addr)	{set(addr);}
	IpAddress(const in_addr_t addr)			{set(addr);}
	IpAddress(uint8_t * addr)			{set(addr);}

	bool		set(const std::string & addr);
	bool		set(const struct sockaddr_in & addr);
	bool		set(const struct sockaddr_in6 & addr);
	bool		set(const in_addr_t addr);
	bool		set(uint8_t * addr);

	bool		isv6() const {return m_isv6;}

	std::string	getString() const;

	in_addr_t	get_addr4() const {return m_addr4;}

	void		get_addr6(uint8_t * addr) const {::memcpy(addr,m_addr6,16);}

	void		init_sockaddr(struct sockaddr_in & addr, const in_port_t port);
	void		init_sockaddr(struct sockaddr_in6 & addr, const in_port_t port);


	bool operator == (const IpAddress & addr) const
	{
		if (m_isv6 != addr.m_isv6) return false;
		if (m_isv6) {
			if (::memcmp(m_addr6,addr.m_addr6,16) != 0) return false;
		} else {
			if (m_addr4 != addr.m_addr4) return false;
		}
		return true;
	}

	bool operator != (const IpAddress & addr) const
	{
		return(!(operator == (addr)));
	}

	size_t		getHash() const
	{
		std::size_t h1 = std::hash<bool>{}(m_isv6);
		std::size_t h2 = std::hash<in_addr_t>{}(m_addr4);
		std::string s="0123456701234567";
		for (uint32_t u0=0;u0<16;u0++) s[u0] = m_addr6[u0];
		std::size_t h3 = std::hash<std::string>{}(s);
		return h1 ^ h2 ^ h3;
	}

private:
	bool		m_isv6 = false;
	in_addr_t	m_addr4 = 0;
	uint8_t		m_addr6[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
};

//////////////////////////////////////////////////////////////////////////////

class IpPort
{
public:
	bool operator == (const IpPort & ipPort) const
	{
		if (port != ipPort.port) return false;
		if (addr != ipPort.addr) return false;
		return true;
	}

	bool operator != (const IpPort & ipPort) const {return ! operator==(ipPort);}

	uint16_t	getPort() const {return ::ntohs(port);}
	void		setPort(const uint16_t inport) {port = ::htons(inport);}

public:
	in_port_t	port = 0;
	IpAddress	addr;
};

//////////////////////////////////////////////////////////////////////////////

struct ConnectionKey
{
	IpPort	src;
	IpPort	dst;
};

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

template<>
struct std::hash<msglib::IpAddress>
{
	std::size_t operator()(const msglib::IpAddress & s) const noexcept
	{
		return s.getHash();
	}
};

//////////////////////////////////////////////////////////////////////////////

template<>
struct std::hash<msglib::IpPort>
{
	std::size_t operator()(const msglib::IpPort & s) const noexcept
	{
		std::size_t h1 = std::hash<in_port_t>{}(s.port);
		std::size_t h2 = s.addr.getHash();
		return h1 ^ h2;
	}
};

//////////////////////////////////////////////////////////////////////////////

template<>
struct std::hash<msglib::ConnectionKey>
{
	std::size_t operator()(const msglib::ConnectionKey & s) const noexcept
	{
		std::size_t h1 = std::hash<msglib::IpPort>{}(s.src);
		std::size_t h2 = std::hash<msglib::IpPort>{}(s.dst);
		return h1 ^ h2;
	}
};

//////////////////////////////////////////////////////////////////////////////

#endif
