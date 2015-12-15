#define DELEGATOR_INCLUDE "delegator_sink.hpp"
#include <silicium/array_view.hpp>
#include <silicium/function.hpp>
#include <boost/preprocessor/list/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>
namespace tests
{
#include <silicium/delegator/generate.hpp>
}

#include <silicium/success.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>

namespace tests
{
	template <class Element, class Error, class Function>
	struct function_sink
	{
		typedef Element element_type;
		typedef Error error_type;

		explicit function_sink(Function function)
		    : m_function(std::move(function))
		{
		}

		// TODO: empty parameter list
		Si::optional<boost::uint64_t> max_size(Si::nothing) const
		{
			return Si::none;
		}

		error_type append(Si::array_view<element_type> elements)
		{
			return m_function(elements);
		}

		error_type append_n_times(Si::function<element_type()> const &generate, std::size_t count)
		{
			while (count > 0)
			{
				element_type element = generate();
				error_type error = m_function(Si::make_single_element_view(element));
				if (error)
				{
					return error;
				}
				--count;
			}
			return error_type();
		}

	private:
		Function m_function;
	};

	template <class Element, class Error, class Function>
	auto make_function_sink(Function &&function) -> function_sink<Element, Error, typename std::decay<Function>::type>
	{
		return function_sink<Element, Error, typename std::decay<Function>::type>(std::forward<Function>(function));
	}

	BOOST_AUTO_TEST_CASE(delegator_erase)
	{
		std::vector<std::unique_ptr<int>> results;
		auto function_sink_ = make_function_sink<std::unique_ptr<int>, Si::success>(
		    [&results](Si::array_view<std::unique_ptr<int>> elements) -> Si::success
		    {
			    std::move(elements.begin(), elements.end(), std::back_inserter(results));
			    return Si::success();
			});
		auto erased_sink_ = tests::Sink<std::unique_ptr<int>, Si::success>::erase(function_sink_);
		std::unique_ptr<int> in = Si::make_unique<int>(23);
		BOOST_CHECK(!erased_sink_.append(Si::make_single_element_view(in)));
		BOOST_REQUIRE(!in);
		BOOST_REQUIRE_EQUAL(1u, results.size());
		BOOST_CHECK_EQUAL(23, *results.front());
	}

	void append_example_value(Sink<std::unique_ptr<int>, Si::success>::fat_ref to)
	{
		std::unique_ptr<int> value = Si::make_unique<int>(23);
		BOOST_CHECK(!to.append(Si::make_single_element_view(value)));
	}

	BOOST_AUTO_TEST_CASE(delegator_fat_ref)
	{
		std::vector<std::unique_ptr<int>> results;
		auto function_sink_ = make_function_sink<std::unique_ptr<int>, Si::success>(
		    [&results](Si::array_view<std::unique_ptr<int>> elements) -> Si::success
		    {
			    std::move(elements.begin(), elements.end(), std::back_inserter(results));
			    return Si::success();
			});
		append_example_value(Sink<std::unique_ptr<int>, Si::success>::fat_ref(function_sink_));
		BOOST_REQUIRE_EQUAL(1u, results.size());
		BOOST_CHECK_EQUAL(23, *results.front());
	}

	BOOST_AUTO_TEST_CASE(delegator_const_method)
	{
		std::vector<std::unique_ptr<int>> results;
		auto function_sink_ = make_function_sink<std::unique_ptr<int>, Si::success>(
		    [](Si::array_view<std::unique_ptr<int>>) -> Si::success
		    {
			    return Si::success();
			});
		Sink<std::unique_ptr<int>, Si::success>::fat_ref const ref((function_sink_));
		BOOST_CHECK_EQUAL(Si::optional<boost::uint64_t>(), ref.max_size(Si::nothing()));
	}

	BOOST_AUTO_TEST_CASE(delegator_two_arguments)
	{
		std::vector<std::unique_ptr<int>> results;
		auto function_sink_ = make_function_sink<std::unique_ptr<int>, Si::success>(
		    [&results](Si::array_view<std::unique_ptr<int>> elements) -> Si::success
		    {
			    std::move(elements.begin(), elements.end(), std::back_inserter(results));
			    return Si::success();
			});
		auto erased_sink_ = tests::Sink<std::unique_ptr<int>, Si::success>::erase(function_sink_);
		BOOST_CHECK(!erased_sink_.append_n_times(
		    []()
		    {
			    return Si::make_unique<int>(23);
			},
		    1));
		BOOST_REQUIRE_EQUAL(1u, results.size());
		BOOST_CHECK_EQUAL(23, *results.front());
	}
}
