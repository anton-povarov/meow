////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__FORMAT_TIMEVAL_HPP_
#define MEOW_LOGGING__FORMAT_TIMEVAL_HPP_

#include <cstdio>
#include <ctime>

#include <meow/tmp_buffer.hpp>
#include <meow/format/metafunctions.hpp>
#include <meow/format/inserter/integral.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE:
// this header is *NOT* a duplicate for format/inserter/timeval.hpp
//  as we have very special timeval formatting for logging purposes

	struct time_as_log_insert_wrapper_t
	{
		timeval_t const& tv;
	};

	inline time_as_log_insert_wrapper_t as_log_ts(timeval_t const& tv)
	{
		time_as_log_insert_wrapper_t const r = { .tv = tv };
		return r;
	}

	template<>
	struct type_tunnel<time_as_log_insert_wrapper_t>
	{
		static size_t const buffer_size = sizeof("YYYYmmdd hhmmss.xxxxxx");
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(time_as_log_insert_wrapper_t const& twrap, buffer_t const& buf = buffer_t())
		{
			struct tm tm;
			::gmtime_r(&twrap.tv.tv_sec, &tm);

			char *begin = buf.begin();
			char *p = buf.end();

			auto const microseconds = twrap.tv.tv_nsec / (nsec_in_sec / usec_in_sec);

			p = detail::integer_to_string(begin, p - begin, microseconds);

			int const field_size = 6;
			int const printed_size = (buf.end() - p);
			for (int i = 0; i < field_size - printed_size; ++i)
				*--p = '0';

			*--p = '.';
			p = detail::integer_to_string(begin, p - begin, tm.tm_sec);
			if (tm.tm_sec < 10)
				*--p = '0';
			p = detail::integer_to_string(begin, p - begin, tm.tm_min);
			if (tm.tm_min < 10)
				*--p = '0';
			p = detail::integer_to_string(begin, p - begin, tm.tm_hour);
			if (tm.tm_hour < 10)
				*--p = '0';
			*--p = ' ';
			p = detail::integer_to_string(begin, p - begin, tm.tm_mday);
			if (tm.tm_mday < 10)
				*--p = '0';
			p = detail::integer_to_string(begin, p - begin, tm.tm_mon + 1);
			if (tm.tm_mon < 9)
				*--p = '0';
			p = detail::integer_to_string(begin, p - begin, tm.tm_year + 1900);

			return str_ref(p, buf.end());
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__FORMAT_TIMEVAL_HPP_

