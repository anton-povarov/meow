////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2018 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__BASE64_HPP_
#define MEOW_FORMAT_INSERTER__BASE64_HPP_

#include <meow/str_ref.hpp>
#include <meow/convert/base64.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline std::string as_base64(str_ref s, base64_tables_t const& tables = base64_default_tables())
	{
		return base64_encode(s, tables);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__BASE64_HPP_

