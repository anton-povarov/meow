////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_CONFIG__PLATFORM_BITS_HPP_
#define MEOW_CONFIG__PLATFORM_BITS_HPP_

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__)
#	define MEOW_PLATFORM_BITS 64
#else
#	define MEOW_PLATFORM_BITS 32
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
	
#endif // MEOW_CONFIG__PLATFORM_BITS_HPP_

