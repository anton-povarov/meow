////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__UNIQUE_ID_HPP_
#define MEOW__UNIQUE_ID_HPP_

#include <inttypes.h>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef uint64_t unique_id_t;

	template<class Tag>
	struct unique_id
	{
		static unique_id_t generate_unique_id()
		{
			static unique_id_t uid = 0;
			return ++uid;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__UNIQUE_ID_HPP_
