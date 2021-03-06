////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// cd meow/test/format/
// g++ -std=c++11 -O0 -g3 -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.53.0 -o format_test format_test.cpp
//

#include <meow/format/sink/FILE.hpp>
#include <meow/format/sink/std_string.hpp>
#include <meow/format/inserter/floating_point.hpp>
#include <meow/format/inserter/integral.hpp>
#include <meow/format/inserter/pointer.hpp>
#include <meow/format/inserter/hex_string.hpp>
#include <meow/format/inserter/timeval.hpp>

#include <meow/format/format.hpp>
#include <meow/format/format_tmp.hpp>
#include <meow/format/format_to_string.hpp>

namespace ff = meow::format;
using ff::fmt;
using ff::write;

int main()
{
	ff::FILE_sink_t sink(stdout);

	fmt(sink, "test string: '{0}'\n", "lalala");
	fmt(sink, "test_number: {0}\n", 10);
	fmt(sink, "{0}", "hehe\n");
	fmt(sink, "preved\n", "");
	fmt(sink, "no args test\n");

	// write
	write(sink, "write test: hey, ", 10, "; float is: ", 0.25f, "\n");

	// numbers
	int number = 1234567890;
	fmt(sink, "number test, n = {0}\n", uint32_t(number));
	fmt(sink, "number test, n = {0}\n", 0);
	fmt(sink, "number test, base10: {0}, base16: {1}\n", -10, ff::as_hex(-10));
	fmt(sink, "number test, base16: 0x{0}\n", ff::as_hex(number));
	fmt(sink, "number test, base16_big: {0}\n", ff::as_HEX(number));
	fmt(sink, "number test, base2: {0}\n", ff::as_bin(number));
	fmt(sink, "number test, base10: {1}, base16: 0x{0}, base2: {2}\n", ff::as_hex(number), number, ff::as_bin(number));

	void *ptr = (void*)0xdeadbeef;
	fmt(sink, "pointer test: {0}\n", ptr);

	double const f_number = -12312.9283283;
	fmt(sink, "floating point test, float: {0}, double: {1}, ldouble: {2}\n", float(f_number), double(f_number), (long double)f_number);
	fmt(sink, "floating point fmt test: {0}, {1}\n", ff::as_printf("%1.2f", f_number), ff::as_printf("%e", f_number));

	fmt(sink, "as_printf() test: {0}\n", ff::as_printf("i'm printf %d, %f %.20E", 10, f_number, f_number));
	fmt(sink, "as_printf_tmp() test: {0}\n", ff::as_printf_tmp<64>("HAI %d, %f", number, f_number));

	// formatting exceptions test
	fmt(sink, "braces test1: {{"); fmt(sink, "\n"); // ok, should give 1 { in the end
	try { fmt(sink, "braces test2: "); fmt(sink, "{"); }	// bad string, should throw an exception
	catch (ff::bad_format_string_t const&) { printf("braces test exception caught, works!\n"); }
	fmt(sink, "braces test3: {{{0}}\n", "arg immediately");
	try { fmt(sink, "extra arguments test: "); fmt(sink, "{1}\n"); }
	catch (ff::bad_argref_number_t const&) { printf("extra arg exception caught, works!\n"); }
	try { fmt(sink, "bad argument string test: "); fmt(sink, "{HO}\n"); }
	catch (ff::bad_argref_string_t const&) { printf("bad argument string exception caught, works!\n"); }

	// std::string as sink test
	{
		std::string s;
		fmt(s, "test format to string: {0} {1}\n", 10, "abcde");
		printf("%s", s.c_str());
	}

	// as_hex_string inserter
	{
		char buf[24]; std::memset(buf, 0, sizeof(buf));
		ff::fmt(sink, "as_hex_string test[0]: {0}\n", ff::as_hex_string(meow::ref_array(buf)));
	}
	{
		meow::str_ref n_str((char*)&number, (size_t)sizeof(number));
		ff::fmt(sink, "as_hex_string test[1]: {0}\n", ff::as_hex_string(n_str));
	}
	{
		meow::str_ref n_str((char*)&number, (size_t)sizeof(number));
		ff::fmt(sink, "as_hex_string test[1]: {0}\n", ff::as_escaped_hex_string(n_str));
	}

	// fmt_tmp and fmt_str
	ff::fmt(sink, "fmt_tmp test, inner format: \"{0}\"\n", ff::fmt_tmp<256>("inner format whatever {0}, {1}", 10, -125.67890));
	ff::fmt(sink, "fmt_str test, inner format: \"{0}\"\n", ff::fmt_str("inner format whatever {0}, {1}", 10, -125.67890));

	// timeval
	{
		struct timeval tv = { 1234567890, 123456 };
		ff::fmt(sink, "{0}\n", tv);
	}

	return 0;
}

