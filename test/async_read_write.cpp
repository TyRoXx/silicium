#include <silicium/sink/sink.hpp>
#include <silicium/source/source.hpp>
#include <silicium/array_view.hpp>
#include <silicium/variant.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/io_service.hpp>

namespace Si
{
	template <class Element>
	struct async_reader
	{
		typedef Element element_type;

		struct reader_buffered
		{
		};

		struct writer_buffered
		{
			array_view<element_type> ready;

			explicit writer_buffered(array_view<element_type> ready)
			    : ready(ready)
			{
			}
		};

		typedef variant<reader_buffered, writer_buffered> buffer_mode;

		buffer_mode check_buffer();
		void mark_as_read(std::size_t ready_read);

		template <class Handler>
		void async_read(array_view<element_type> destination, Handler &&handle_finished);
	};
}

template <class Sum, class Input, class Combinator>
struct accumulator
{
	typedef Sum element_type;

	explicit accumulator(Sum initial_sum, Input input, Combinator combine)
	    : m_sum(std::move(initial_sum))
	    , m_sum_changed(true)
	    , m_input(std::move(input))
	    , m_combine(std::move(combine))
	{
	}

	typename Si::async_reader<element_type>::buffer_mode check_buffer()
	{
		if (m_sum_changed)
		{
			return typename Si::async_reader<element_type>::writer_buffered(
			    Si::array_view<element_type>(m_sum, Si::bounded_size_t::literal<1>()));
		}
		else
		{
			return typename Si::async_reader<element_type>::writer_buffered(Si::array_view<element_type>());
		}
	}

	void mark_as_read(std::size_t ready_read)
	{
	}

	template <class Handler>
	void async_read(Si::array_view<element_type> destination, Handler &&handle_finished)
	{
	}

private:
	Sum m_sum;
	bool m_sum_changed;
	Input m_input;
	Combinator m_combine;
};

template <class Sum, class Input, class Combinator>
accumulator<typename std::decay<Sum>::type, typename std::decay<Input>::type, typename std::decay<Combinator>::type>
make_accumulator(Sum &&initial_sum, Input &&input, Combinator &&combinator)
{
	return accumulator<typename std::decay<Sum>::type, typename std::decay<Input>::type,
	                   typename std::decay<Combinator>::type>(
	    std::forward<Sum>(initial_sum), std::forward<Input>(input), std::forward<Combinator>(combinator));
}

BOOST_AUTO_TEST_CASE(async_read_write_test)
{
	typedef int message;
	boost::asio::io_service dispatcher;
	Si::async_reader<message> reader;
	auto sum = make_accumulator(0, reader, [](int &left, int right)
	                            {
		                            left += right;
		                        });
	Si::visit<void>(sum.check_buffer(),
	                [](Si::async_reader<message>::writer_buffered buffered)
	                {
		                Si::ignore_unused_variable_warning(buffered);
		            },
	                [](Si::async_reader<message>::reader_buffered buffered)
	                {
		                Si::ignore_unused_variable_warning(buffered);
		            });
}
