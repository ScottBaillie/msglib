//////////////////////////////////////////////////////////////////////////////

#include <Network/IpAddress/IpAddress.h>

using namespace msglib;

#include <sys/socket.h>

//////////////////////////////////////////////////////////////////////////////

bool
IpAddress::set(const std::string & addr)
{
	if (addr.size()==0) return false;
	bool isv6 = false;

	for (auto c : addr) {
		if (c==':') {
			isv6 = true;
			break;
		}
	}

	int af;
	void * dst;
	struct in6_addr addr6;
	struct in_addr addr4;

	if (isv6) {
		af = AF_INET6;
		dst = &addr6;
	} else {
		af = AF_INET;
		dst = &addr4;
	}

	int ret = ::inet_pton(af, addr.c_str(), dst);

	if (ret != 1) return false;

	if (isv6) {
		m_isv6 = true;
		::memcpy(m_addr6,addr6.s6_addr,16);
	} else {
		m_isv6 = false;
		m_addr4 = addr4.s_addr;
	}

	return true;
}

bool
IpAddress::set(const struct sockaddr_in & addr)
{
	return set(addr.sin_addr.s_addr);
}

bool
IpAddress::set(const struct sockaddr_in6 & addr)
{
	return set((uint8_t*)addr.sin6_addr.s6_addr);
}

bool
IpAddress::set(const in_addr_t addr)
{
	m_isv6 = false;
	m_addr4 = addr;
	return true;
}

bool
IpAddress::set(uint8_t * addr)
{
	m_isv6 = true;
	::memcpy(m_addr6,addr,16);
	return true;
}

//////////////////////////////////////////////////////////////////////////////

std::string
IpAddress::getString() const
{
	char str[128];
	int af;
	void * src;
	struct in6_addr addr6;
	struct in_addr addr4;

	if (m_isv6) {
		af = AF_INET6;
		::memcpy(addr6.s6_addr,m_addr6,16);
		src = &addr6;
	} else {
		af = AF_INET;
		addr4.s_addr = m_addr4;
		src = &addr4;
	}

	auto p = ::inet_ntop(af, src, str, sizeof(str));

	if (p==0) return "";

	std::string ret;

	ret = str;

	return ret;
}

//////////////////////////////////////////////////////////////////////////////

void
IpAddress::init_sockaddr(struct sockaddr_in & addr, const in_port_t port) const
{
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = m_addr4;
}

void
IpAddress::init_sockaddr(struct sockaddr_in6 & addr, const in_port_t port) const
{
	addr.sin6_family = AF_INET6;
	addr.sin6_port = port;
	::memcpy(addr.sin6_addr.s6_addr,m_addr6,16);
}

//////////////////////////////////////////////////////////////////////////////

int
msglib::socket(int type, const bool isv6)
{
	int domain = AF_INET;
	if (isv6) domain = AF_INET6;
	int ret = ::socket(domain, type, 0);
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

int
msglib::bind(const int fd, const IpPort & ipPort)
{
	int ret;
	if (ipPort.addr.isv6()) {
		struct sockaddr_in6 addr;
		ipPort.addr.init_sockaddr(addr, ipPort.port);
		ret = ::bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
	} else {
		struct sockaddr_in addr;
		ipPort.addr.init_sockaddr(addr, ipPort.port);
		ret = ::bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

ssize_t
msglib::sendto(const int fd, const void * buf, size_t size, int flags, const IpPort & ipPort)
{
	ssize_t ret;
	if (ipPort.addr.isv6()) {
		struct sockaddr_in6 addr;
		ipPort.addr.init_sockaddr(addr, ipPort.port);
		ret = ::sendto(fd, buf, size, flags, (const struct sockaddr *)&addr, sizeof(addr));
	} else {
		struct sockaddr_in addr;
		ipPort.addr.init_sockaddr(addr, ipPort.port);
		ret = ::sendto(fd, buf, size, flags, (const struct sockaddr *)&addr, sizeof(addr));
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

ssize_t
msglib::recvfrom(const int fd, void * buf, size_t size, int flags, IpPort & ipPort, const bool isv6)
{
	ssize_t ret;
	socklen_t addrlen;
	if (isv6) {
		struct sockaddr_in6 addr;
		addrlen = sizeof(addr);
		ret = ::recvfrom(fd, buf, size, flags, (struct sockaddr *)&addr, &addrlen);
		if (ret != -1) {
			if (addrlen != sizeof(addr)) return -1;
			ipPort.addr.set(addr);
			ipPort.port = addr.sin6_port;
		}
	} else {
		struct sockaddr_in addr;
		addrlen = sizeof(addr);
		ret = ::recvfrom(fd, buf, size, flags, (struct sockaddr *)&addr, &addrlen);
		if (ret != -1) {
			if (addrlen != sizeof(addr)) return -1;
			ipPort.addr.set(addr);
			ipPort.port = addr.sin_port;
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

int
msglib::connect(const int fd, const IpPort & ipPort)
{
	int ret;
	if (ipPort.addr.isv6()) {
		struct sockaddr_in6 addr;
		ipPort.addr.init_sockaddr(addr, ipPort.port);
		ret = ::connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
	} else {
		struct sockaddr_in addr;
		ipPort.addr.init_sockaddr(addr, ipPort.port);
		ret = ::connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

int
msglib::accept(const int fd, IpPort & ipPort, const bool isv6)
{
	int ret;
	socklen_t addrlen;
	if (isv6) {
		struct sockaddr_in6 addr;
		addrlen = sizeof(addr);
		ret = ::accept(fd, (struct sockaddr *)&addr, &addrlen);
		if (ret != -1) {
			if (addrlen != sizeof(addr)) return -1;
			ipPort.addr.set(addr);
			ipPort.port = addr.sin6_port;
		}
	} else {
		struct sockaddr_in addr;
		addrlen = sizeof(addr);
		ret = ::accept(fd, (struct sockaddr *)&addr, &addrlen);
		if (ret != -1) {
			if (addrlen != sizeof(addr)) return -1;
			ipPort.addr.set(addr);
			ipPort.port = addr.sin_port;
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

