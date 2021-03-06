////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__FD_HANDLE_HPP_
#define MEOW_UNIX__FD_HANDLE_HPP_

#include <meow/movable_handle.hpp>
#include <meow/unix/unistd.hpp> // for close_ex

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct fd_handle_traits
	{
		typedef int handle_type;

		static handle_type null() { return handle_type(-1); }
		static bool is_null(handle_type const& h) { return h == null(); }
		static void close(handle_type& h) { os_unix::close_ex(h); }
	};

	typedef meow::movable_handle<fd_handle_traits> fd_handle_t;

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__FD_HANDLE_HPP_

