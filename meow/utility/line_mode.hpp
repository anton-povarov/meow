////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__LINE_MODE_HPP_
#define MEOW_UTILITY__LINE_MODE_HPP_

#include <inttypes.h> // uint32_t

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct line_mode {
		typedef uint32_t int_t;

		static int_t const prefix = 0x00000001;
		static int_t const suffix = 0x00000010;
		static int_t const middle = 0x00000000;
		static int_t const single = prefix | suffix;
	};
	typedef line_mode::int_t line_mode_t;

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UTILITY__LINE_MODE_HPP_

