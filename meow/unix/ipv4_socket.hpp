////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__IPV4_SOCKET_HPP_
#define MEOW_UNIX__IPV4_SOCKET_HPP_

#include <sys/types.h>
#include <sys/socket.h>

#include <meow/unix/ipv4_address.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace ipv4 {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline address_t getpeername_ex(int fd)
	{
		sockaddr_in a;
		socklen_t len = sizeof(a);
		::getpeername(fd, (struct sockaddr*)&a, &len);
		assert(len == sizeof(a));
		return address_t(a);
	}

	inline address_t getsockname_ex(int fd)
	{
		sockaddr_in a;
		socklen_t len = sizeof(a);
		::getsockname(fd, (struct sockaddr*)&a, &len);
		assert(len == sizeof(a));
		return address_t(a);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace ipv4 {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__IPV4_SOCKET_HPP_

