////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_HASH__HASH_FWD_HPP_
#define MEOW_HASH__HASH_FWD_HPP_

#include <stdint.h>
#include <boost/utility/enable_if.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef uint32_t hash_result_t;

	template<class Tag>
	struct hash_impl;
#if 0 && MEOW_HASH_FUNCTIONS_IMPL_INTERFACE // the example interface that's expected
	{
		static hash_result_t hash_word_array(
									  uint32_t const *p 	// base pointer
									, int len_32 			// length in uint32_t's
									, uint32_t initval 		// arbitrary initial value
									);

		static hash_result_t hash_blob(
									  void const *p 		// base pointer
									, int len 				// length in bytes
									, uint32_t initval 		// arbitrary initial value
									);
	};
#endif
	template<class T, class Enabler = void> struct hash; // no default impl

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_HASH__HASH_FWD_HPP_
