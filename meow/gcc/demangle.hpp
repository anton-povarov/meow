////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_GCC__DEMANGLE_HPP_
#define MEOW_GCC__DEMANGLE_HPP_

#include <cxxabi.h>		// for __cxa_demangle

#include <cstdlib> 		// for std::free
#include <stdexcept>

#include <meow/movable_handle.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct demangled_memory_traits
	{
		typedef char* handle_type;

		static handle_type null() { return NULL; }
		static bool is_null(handle_type h) { return null() == h; }
		static void close(handle_type& h) { std::free(static_cast<void*>(h)); }
	};

	typedef meow::movable_handle<demangled_memory_traits> demangled_string_t;

////////////////////////////////////////////////////////////////////////////////////////////////

	inline demangled_string_t gcc_demangle_name(char const *name)
	{
		int status = 0;
		demangled_string_t res(abi::__cxa_demangle(name, NULL, NULL, &status));

		if (0 == status)
			return res;

		switch (status)
		{
			case -1: throw std::runtime_error("gcc_demangle_name: memory allocation failure");
			case -2: throw std::runtime_error("gcc_demangle_name: invalid mangled name");
			case -3: assert(false && "gcc_demangle_name: invalid argument, MUST NOT HAPPEN");
			default: assert(false && "gcc_demangle_name: unknown result status, MUST NOT HAPPEN");
		}
	}

	inline int gcc_demangle_name(demangled_string_t *result, char const *name)
	{
		int status = 0;
		result->reset(abi::__cxa_demangle(name, NULL, NULL, &status));
		return status;
	}

	inline char const* gcc_demangle_name_tmp(char const *name, int *status = NULL, demangled_string_t str = demangled_string_t())
	{
		int s = gcc_demangle_name(&str, name);
		if (status)
			*status = s;

		return get_handle(str);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_GCC__DEMANGLE_HPP_

