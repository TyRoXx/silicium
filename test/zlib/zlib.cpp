#include <silicium/sink/container_buffer.hpp>
#include <silicium/zlib/deflating_sink.hpp>
#include <silicium/zlib/inflating_sink.hpp>
#include <silicium/sink/append.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_HAS_DEFLATING_SINK
BOOST_AUTO_TEST_CASE(zlib_stream_wrappers)
{
	Si::zlib_deflate_stream deflator(Z_DEFAULT_COMPRESSION);
	std::string const original = "Hello";
	std::array<char, 4096> compressed;
	std::pair<std::size_t, std::size_t> compress_result = deflator.deflate(
	    Si::make_iterator_range(original.data(), original.data() + original.size()),
	    Si::make_iterator_range(compressed.data(), compressed.data() + compressed.size()), Z_FULL_FLUSH);
	BOOST_CHECK_EQUAL(0u, compress_result.first);
	BOOST_CHECK_LT(compress_result.second, compressed.size());
	std::array<char, 4096> decompressed;
	Si::zlib_inflate_stream inflator = Si::zlib_inflate_stream::initialize();
	std::pair<std::size_t, std::size_t> decompress_result = inflator.inflate(
	    Si::make_iterator_range(compressed.data(), compressed.data() + compressed.size() - compress_result.second),
	    Si::make_iterator_range(decompressed.data(), decompressed.data() + decompressed.size()), Z_SYNC_FLUSH);
	auto const decompressed_length = decompressed.size() - decompress_result.second;
	BOOST_REQUIRE_EQUAL(original.size(), decompressed_length);
	BOOST_CHECK_EQUAL(original, std::string(decompressed.begin(), decompressed.begin() + decompressed_length));
}

BOOST_AUTO_TEST_CASE(zlib_deflating_sink_test)
{
	std::vector<char> compressed;
	auto compressor =
	    Si::make_deflating_sink(Si::make_container_buffer(compressed), Si::zlib_deflate_stream(Z_DEFAULT_COMPRESSION));
	Si::append(compressor, Si::zlib_sink_element{Si::make_c_str_range("Hello")});
	Si::append(compressor, Si::zlib_sink_element{Si::flush{}});
	BOOST_CHECK_GE(compressed.size(), 1u);
}
#endif
