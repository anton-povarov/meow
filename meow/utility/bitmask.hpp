////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__BITMASK_HPP_
#define MEOW_UTILITY__BITMASK_HPP_

////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T bitmask_set(T& mask, T bit)
{
	return mask |= bit;
}

template<class T>
inline T bitmask_clear(T& mask, T bit)
{
	return mask &= ~bit;
}

template<class T>
inline T bitmask_test(T const& mask, T bit)
{
	return (mask & bit);
}

////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UTILITY__BITMASK_HPP_
