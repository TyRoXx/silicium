#include <silicium/container_buffer.hpp>
#include <silicium/zlib/deflating_sink.hpp>
#include <silicium/zlib/inflating_sink.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(zlib_stream_wrappers)
	{
		zlib_deflate_stream deflator(Z_DEFAULT_COMPRESSION);
		std::string const original = "Hello";
		std::array<char, 4096> compressed;
		std::pair<std::size_t, std::size_t> compress_result = deflator.deflate(
			boost::make_iterator_range(original.data(), original.data() + original.size()),
			boost::make_iterator_range(compressed.data(), compressed.data() + compressed.size()),
			Z_FULL_FLUSH
		);
		BOOST_CHECK_EQUAL(0, compress_result.first);
		BOOST_CHECK_LT(compress_result.second, compressed.size());
		std::array<char, 4096> decompressed;
		zlib_inflate_stream inflator = zlib_inflate_stream::initialize();
		std::pair<std::size_t, std::size_t> decompress_result = inflator.inflate(
			boost::make_iterator_range(compressed.data(), compressed.data() + compressed.size() - compress_result.second),
			boost::make_iterator_range(decompressed.data(), decompressed.data() + decompressed.size()),
			Z_SYNC_FLUSH
		);
		auto const decompressed_length = decompressed.size() - decompress_result.second;
		BOOST_REQUIRE_EQUAL(original.size(), decompressed_length);
		BOOST_CHECK_EQUAL(original, std::string(decompressed.begin(), decompressed.begin() + decompressed_length));
	}

	template <class C>
	auto make_c_str_range(C const *str)
	{
		return boost::make_iterator_range(str, str + std::char_traits<C>::length(str));
	}

	BOOST_AUTO_TEST_CASE(zlib_sink)
	{
		std::vector<char> compressed;
		auto compressor = make_deflating_sink(make_container_buffer(compressed), zlib_deflate_stream(Z_DEFAULT_COMPRESSION));
		append(compressor, zlib_sink_element{make_c_str_range("Hello")});
		append(compressor, zlib_sink_element{flush{}});
		BOOST_CHECK_GE(compressed.size(), 1);
	}
}
