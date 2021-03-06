////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TO_ERROR_HPP_
#define MEOW_FORMAT__FORMAT_TO_ERROR_HPP_

#include <meow/error.hpp>
#include <meow/format/format_to_string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class F, class... A>
	inline error_t fmt_err(F const& f, A const&... args)
	{
		return error_t{fmt_str(f, args...)};
	}

	template<class... A>
	inline error_t write_err(A const&... args)
	{
		return error_t{write_str(args...)};
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TO_ERROR_HPP_

