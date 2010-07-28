////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__FLOATING_POINT_HPP_
#define MEOW_FORMAT_INSERTER__FLOATING_POINT_HPP_

#include <cstdlib>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_floating_point.hpp>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>

#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
	namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

		template<class T>
		struct floating_point_wrapper_t
		{
			char const *printf_format;
			T const 	value;

			floating_point_wrapper_t(char const *f, T const& v)
				: printf_format(f)
				, value(v)
			{
			}
		};

		template<class T> struct floating_point_traits;

		template<> struct floating_point_traits<float> {
			static char const* printf_format() { return "%f"; }
		};

		template<> struct floating_point_traits<double> {
			static char const* printf_format() { return "%f"; }
		};

		template<> struct floating_point_traits<long double> {
			static char const* printf_format() { return "%Lf"; }
		};

////////////////////////////////////////////////////////////////////////////////////////////////
	} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	detail::floating_point_wrapper_t<T>
	float_fmt(char const *f, T const& v)
	{
		return detail::floating_point_wrapper_t<T>(f, v);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	struct type_tunnel<T, typename boost::enable_if<boost::is_floating_point<T> >::type>
	{
		enum { buffer_size = sizeof("-01234567890123456789.01234567890123456789") };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(T v, buffer_t const& buf = buffer_t())
		{
			ssize_t n = ::snprintf(buf.get(), buf.size(), detail::floating_point_traits<T>::printf_format(), v);
			BOOST_ASSERT(n > 0);
			return str_ref(buf.get(), n);
		}
	};

	template<class T>
	struct type_tunnel<detail::floating_point_wrapper_t<T> >
	{
		enum { buffer_size = sizeof("-01234567890123456789.01234567890123456789") };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(detail::floating_point_wrapper_t<T> const& v, buffer_t const& buf = buffer_t())
		{
			ssize_t n = ::snprintf(buf.get(), buf.size(), v.printf_format, v.value);
			BOOST_ASSERT(n > 0);
			return str_ref(buf.get(), n);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__FLOATING_POINT_HPP_
