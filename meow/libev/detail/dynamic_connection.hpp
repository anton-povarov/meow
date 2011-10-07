////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__DYNAMIC_CONNECTION_HPP_
#define MEOW_LIBEV_DETAIL__DYNAMIC_CONNECTION_HPP_

#include <meow/str_ref.hpp>
#include <meow/libev/io_machine.hpp> // rd_consume_status_t, read_status_t
#include <meow/libev/detail/generic_connection.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct dynamic_connection_t;

	struct dynamic_reader_events_t
	{
		typedef dynamic_connection_t connection_t;

		virtual ~dynamic_reader_events_t() {}

		virtual buffer_ref           get_buffer(connection_t*) = 0;
		virtual rd_consume_status_t  consume_buffer(connection_t*, buffer_ref, read_status_t) = 0;
		virtual void                 on_closed(connection_t*, io_close_report_t const&) = 0;
	};

	struct dynamic_connection_t
		: public generic_connection_e_t<dynamic_reader_events_t>
	{
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__DYNAMIC_CONNECTION_HPP_

