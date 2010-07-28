////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_FORMAT_HPP_
#define MEOW_FORMAT_FORMAT_HPP_

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>

#include <meow/format/metafunctions.hpp>
#include <meow/format/format_parser.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#define FMT_NUMBERED_CALL(name, n) BOOST_PP_CAT(BOOST_PP_CAT(name, _), n)
#define FMT_STRING_STAGE_CALL(n) FMT_NUMBERED_CALL(fmt_string_stage, n)
#define FMT_TUNNEL_STAGE_CALL(n) FMT_NUMBERED_CALL(fmt_tunnel_stage, n)
#define FMT_INTERFACE_CALL(n) fmt

#define FMT_TEMPLATE_PARAMS(n) BOOST_PP_ENUM_TRAILING_PARAMS(n, class A)
#define FMT_DEF_PARAMS(n) BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n, A, const& a)
#define FMT_CALL_PARAMS(n) BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n, A, const& a)

#define FMT_TRAITS_CALL_n(z, n, call_name) 								\
	call_name<BOOST_PP_CAT(A, n)>::call(BOOST_PP_CAT(a, n)) 			\
/**/

#define FMT_TRAITS_CALL_AS_ARGS(n, call_name) \
	BOOST_PP_ENUM_TRAILING(n, FMT_TRAITS_CALL_n, call_name)

#define FMT_STRING_STAGE_FUNCTION_ARG_ARRAY_FILL(z, n, arr_name) \
	arr_name[n] = BOOST_PP_CAT(a, n);

#define FMT_STRING_STAGE_FUNCTION_BODY(n) 										\
	size_t const max_slices = get_max_slices_for_format(fmt); 					\
	str_ref slices[max_slices]; /* VLA, yo */ 									\
 																				\
	static size_t const n_arg_slices = n; 										\
	str_ref arg_slices[n_arg_slices]; 											\
	BOOST_PP_REPEAT(n, FMT_STRING_STAGE_FUNCTION_ARG_ARRAY_FILL, arg_slices); 	\
 																				\
	format_info_t const fi = parse_format_expression( 							\
								  fmt 											\
								, slices, max_slices 							\
								, arg_slices, n_arg_slices 						\
								); 												\
	sink::sink_write<S>::call(sink, fi.total_length, slices, fi.n_slices); 		\
	return sink;
/**/

#define FMT_TUNNEL_STAGE_FUNCTION_BODY(n) 								\
	return FMT_STRING_STAGE_CALL(n)( 									\
			  sink 														\
			, string_access<F>::call(fmt) 								\
			  FMT_TRAITS_CALL_AS_ARGS(n, string_access) 				\
			); 															\
/**/

#define FMT_INTERFACE_FUNCTION_BODY(n) 									\
	return FMT_TUNNEL_STAGE_CALL(n)( 									\
			  sink 														\
			, type_tunnel<F>::call(fmt) 								\
			  FMT_TRAITS_CALL_AS_ARGS(n, type_tunnel) 					\
			); 															\
/**/

#define FMT_DEFINE_MULTI_ARG_FUNCTION(n, fn_name, fn_body_macro) 		\
template<class S, class F FMT_TEMPLATE_PARAMS(n)> 						\
S& fn_name(S& sink, F const& fmt FMT_DEF_PARAMS(n)) 					\
{ 																		\
	fn_body_macro(n) 													\
} 																		\
/**/

#define DEFINE_FMT_FUNCTION(z, n, d) 									\
	FMT_DEFINE_MULTI_ARG_FUNCTION(n, FMT_STRING_STAGE_CALL(n), FMT_STRING_STAGE_FUNCTION_BODY) \
	FMT_DEFINE_MULTI_ARG_FUNCTION(n, FMT_TUNNEL_STAGE_CALL(n), FMT_TUNNEL_STAGE_FUNCTION_BODY) \
	FMT_DEFINE_MULTI_ARG_FUNCTION(n, FMT_INTERFACE_CALL(n), FMT_INTERFACE_FUNCTION_BODY) \
/**/

BOOST_PP_REPEAT(32, DEFINE_FMT_FUNCTION, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_FORMAT_HPP_
