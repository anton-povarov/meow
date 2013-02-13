////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__UNIQUE_PTR_HPP_
#define MEOW__UNIQUE_PTR_HPP_

#include <memory>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	std::unique_ptr<T> unique_new(T *p)
	{
		return std::unique_ptr<T>(p);
	}

	template<class T, class... A>
	std::unique_ptr<T> unique_create(A&&... a)
	{
		return std::unique_ptr<T>(new T(std::forward<A>(a)...));
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} //namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__UNIQUE_PTR_HPP_