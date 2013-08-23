////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_SINK__STD_STRING_HPP_
#define MEOW_FORMAT_SINK__STD_STRING_HPP_

#include <string>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct sink_write<std::basic_string<CharT, std::char_traits<CharT> > >
	{
		static void call(
				  std::basic_string<CharT, std::char_traits<CharT> >& sink
				, size_t total_len
				, string_ref<CharT const> const *slices
				, size_t n_slices
				)
		{
			sink.reserve(sink.size() + total_len);

			for (size_t i = 0; i < n_slices; ++i)
				sink.append(slices[i].data(), slices[i].size());
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_SINK__STD_STRING_HPP_

