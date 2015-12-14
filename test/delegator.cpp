#define DELEGATOR_INCLUDE "delegator_sink.hpp"
#include <silicium/array_view.hpp>
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

		error_type append(Si::array_view<element_type> elements)
		{
			return m_function(elements);
		}

	private:
		Function m_function;
	};

	template <class Element, class Error, class Function>
	auto make_function_sink(Function &&function) -> function_sink<Element, Error, typename std::decay<Function>::type>
	{
		return function_sink<Element, Error, typename std::decay<Function>::type>(std::forward<Function>(function));
	}

	BOOST_AUTO_TEST_CASE(delegator_default_constructor)
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
		erased_sink_.append(Si::make_single_element_view(in));
		BOOST_REQUIRE(!in);
		BOOST_REQUIRE_EQUAL(1u, results.size());
		BOOST_CHECK_EQUAL(23, *results.front());
	}
}
