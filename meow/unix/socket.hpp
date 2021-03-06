////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__SOCKET_HPP_
#define MEOW_UNIX__SOCKET_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cassert>

#include <meow/api_call_error.hpp>
#include <meow/unix/libc_wrapper.hpp>
#include <meow/format/format_to_string.hpp>
#include <meow/format/inserter/sockaddr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef socklen_t               os_socklen_t;

    typedef struct sockaddr         os_sockaddr_t;
    typedef struct sockaddr_in      os_sockaddr_in_t;
    typedef struct sockaddr_in6     os_sockaddr_in6_t;

    typedef struct sockaddr_storage os_sockaddr_storage_t;

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, socket, 	((int, "domain: %d"))
													((int, "type: %d"))
													((int, "proto: %d"))
													);
/*
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, bind, 	((int, "sock: %d"))
													((os_sockaddr_t const*, "%p"))
													((socklen_t, "%d"))
													);
*/
	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, listen, 	((int, "sock: %d"))
													((int, "backlog: %d"))
													);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, connect, ((int, "sock: %d"))
													((os_sockaddr_t const*, "%p"))
													((socklen_t, "%d"))
													);

	MEOW_DEFINE_LIBC_THROWING_WRAPPER(int, accept, ((int, "sock: %d"))
													((os_sockaddr_t*, "%p"))
													((socklen_t*, "%p"))
													);

////////////////////////////////////////////////////////////////////////////////////////////////

	inline int bind_ex(int fd, os_sockaddr_t const *a, os_socklen_t const a_len)
	{
		if (-1 == ::bind(fd, a, a_len))
			throw meow::api_call_error(meow::format::fmt_str("bind({0}, {1}, {2})", fd, a, a_len).c_str());
		return fd;
	}

	template<class T>
	T getsockopt_ex(int fd, int level, int opt)
	{
		T result;
		socklen_t opt_len = sizeof(result);
		if (-1 == ::getsockopt(fd, level, opt, (void*)&result, &opt_len))
			throw meow::api_call_error("getsockopt(%d, %d, %d)", fd, level, opt);
		return result;
	}

	template<class T>
	int setsockopt_ex(int fd, int level, int opt, T const& value)
	{
		int r = ::setsockopt(fd, level, opt, &value, sizeof(value));
		if (-1 == r)
			throw meow::api_call_error("setsockopt(%d, %d, %d, val_sz: %zu)", fd, level, opt, sizeof(value));
		return r;
	}

	template<class AddressT>
	AddressT getpeername_ex(int fd)
	{
		AddressT a = {};
		socklen_t len = sizeof(a);

		if (0 != ::getpeername(fd, reinterpret_cast<struct sockaddr*>(&a), &len))
			throw meow::api_call_error("getpeername(%d, %p, %u)", fd, &a, len);

		return a;
	}

	template<class AddressT>
	AddressT getsockname_ex(int fd)
	{
		AddressT a = {};
		socklen_t len = sizeof(a);

		if (0 != ::getsockname(fd, reinterpret_cast<struct sockaddr*>(&a), &len))
			throw meow::api_call_error("getsockname(%d, %p, %u)", fd, &a, len);

		return a;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__SOCKET_HPP_

