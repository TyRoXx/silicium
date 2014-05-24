#include <silicium/copy.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(copy_empty_source)
	{
		Si::memory_source<char> source_;
		std::vector<char> copied;
		auto sink_ = Si::make_container_sink(copied);
		Si::copy(static_cast<Si::source<char> &>(source_), static_cast<Si::sink<char> &>(sink_));
		BOOST_CHECK(copied.empty());
	}

	BOOST_AUTO_TEST_CASE(copy_non_empty)
	{
		std::string const message = "hello";
		Si::memory_source<char> source_(boost::make_iterator_range(message.data(), message.data() + message.size()));
		std::string copied;
		auto sink_ = Si::make_container_sink(copied);
		Si::copy(static_cast<Si::source<char> &>(source_), static_cast<Si::sink<char> &>(sink_));
		BOOST_CHECK_EQUAL(message, copied);
	}
}
