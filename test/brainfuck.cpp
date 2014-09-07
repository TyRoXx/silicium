#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>
#include <silicium/transform_if_initialized.hpp>
#include <silicium/empty.hpp>
#include <silicium/coroutine.hpp>
#include <silicium/source.hpp>
#include <silicium/for_each.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>

namespace bf
{
	enum class command
	{
		ptr_increment,
		ptr_decrement,
		value_increment,
		value_decrement,
		write,
		read,
		begin_loop,
		end_loop
	};

	boost::optional<command> detect_command(char c)
	{
		switch (c)
		{
		case '>': return command::ptr_increment;
		case '<': return command::ptr_decrement;
		case '+': return command::value_increment;
		case '-': return command::value_decrement;
		case '.': return command::write;
		case ',': return command::read;
		case '[': return command::begin_loop;
		case ']': return command::end_loop;
		default: return boost::none;
		}
	}

	template <class Source>
	auto scan(Source &&source)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> Si::conditional_transformer<command, typename std::decay<Source>::type, boost::optional<command>(*)(char)>
#endif
	{
		return Si::transform_if_initialized(std::forward<Source>(source), detect_command);
	}

#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
#	define SILICIUM_CAPTURE(x) = (x)
#else
#	define SILICIUM_CAPTURE(x)
#endif

	template <class CommandRange>
	typename CommandRange::const_iterator find_loop_begin(
		CommandRange const &program,
		std::unordered_map<std::ptrdiff_t, typename CommandRange::const_iterator> &cached_loop_pairs,
		typename CommandRange::const_iterator const end_)
	{
		using std::begin;
		auto cached = cached_loop_pairs.find(std::distance(program.begin(), end_));
		if (cached != end(cached_loop_pairs))
		{
			return cached->second;
		}
		std::size_t current_depth = 0;
		auto i = end_;
		for (;;)
		{
			assert(i != begin(program));
			--i;
			if (*i == command::end_loop)
			{
				++current_depth;
			}
			else if (*i == command::begin_loop)
			{
				if (current_depth == 0)
				{
					break;
				}
				--current_depth;
			}
		}
		assert(current_depth == 0);
		cached_loop_pairs.insert(std::make_pair(std::distance(program.begin(), end_), i));
		return i;
	}

	template <class CommandRange>
	typename CommandRange::const_iterator find_loop_end(
		CommandRange const &program,
		std::unordered_map<std::ptrdiff_t, typename CommandRange::const_iterator> &cached_loop_pairs,
		typename CommandRange::const_iterator const begin)
	{
		using std::end;
		auto cached = cached_loop_pairs.find(std::distance(program.begin(), begin));
		if (cached != end(cached_loop_pairs))
		{
			return cached->second;
		}
		std::size_t current_depth = 0;
		auto i = begin;
		for (;; ++i)
		{
			assert(i != end(program));
			if (*i == command::begin_loop)
			{
				++current_depth;
			}
			else if (*i == command::end_loop)
			{
				if (current_depth == 0)
				{
					break;
				}
				--current_depth;
			}
		}
		assert(current_depth == 0);
		cached_loop_pairs.insert(std::make_pair(std::distance(program.begin(), begin), i));
		return i;
	}

	template <class Input, class CommandRange, class MemoryRange>
	auto execute(Input &&input, CommandRange const &program, MemoryRange &memory, std::size_t original_pointer)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> Si::coroutine_observable<char>
#endif
	{
		return Si::make_coroutine<char>([
			input SILICIUM_CAPTURE(std::forward<Input>(input)),
			program,
			&memory,
			original_pointer
			](Si::yield_context<char> &yield) mutable
		{
			std::unordered_map<std::ptrdiff_t, CommandRange::const_iterator> loop_pairs;
			auto pointer = original_pointer;
			auto pc = boost::begin(program);
			for (; pc != boost::end(program);)
			{
				switch (*pc)
				{
				case command::ptr_increment:
					++pointer;
					if (pointer == boost::size(memory))
					{
						pointer = 0;
					}
					++pc;
					break;

				case command::ptr_decrement:
					if (pointer == 0)
					{
						pointer = boost::size(memory) - 1;
					}
					else
					{
						--pointer;
					}
					++pc;
					break;

				case command::value_increment:
					++memory[pointer];
					++pc;
					break;

				case command::value_decrement:
					--memory[pointer];
					++pc;
					break;

				case command::write:
					yield(static_cast<char>(memory[pointer]));
					++pc;
					break;

				case command::read:
					{
						auto in = yield.get_one(input);
						if (!in)
						{
							return;
						}
						memory[pointer] = *in;
						++pc;
						break;
					}

				case command::begin_loop:
					if (memory[pointer])
					{
						++pc;
					}
					else
					{
						pc = find_loop_end(program, loop_pairs, pc) + 1;
					}
					break;

				case command::end_loop:
					if (memory[pointer])
					{
						pc = find_loop_begin(program, loop_pairs, pc) + 1;
					}
					else
					{
						++pc;
					}
					break;
				}
			}
		});
	}
}

namespace Si
{
	template <class Element, class Source>
	struct source_observable
	{
		using element_type = Element;

		source_observable()
		{
		}

		explicit source_observable(Source source)
			: source(std::move(source))
		{
		}

		void async_get_one(observer<Element> &receiver)
		{
			boost::optional<Element> value = Si::get(source);
			if (value)
			{
				receiver.got_element(std::move(*value));
			}
			else
			{
				receiver.ended();
			}
		}

	private:

		Source source;
	};

	template <class Element, class Source>
	auto make_source_observable(Source &&source)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> source_observable<Element, typename std::decay<Source>::type>
#endif
	{
		return source_observable<Element, typename std::decay<Source>::type>(std::forward<Source>(source));
	}
}

BOOST_AUTO_TEST_CASE(bf_empty)
{
	Si::empty<char> input;
	std::array<char, 1> memory{{}};
	auto interpreter = bf::execute(input, boost::iterator_range<bf::command const *>(), memory, 0);
	auto done = Si::for_each(std::move(interpreter), [](char output)
	{
		boost::ignore_unused_variable_warning(output);
		BOOST_FAIL("no output expected");
	});
	done.start();
}

namespace
{
	template <class CharSource>
	std::vector<bf::command> scan_all(CharSource code)
	{
		std::vector<bf::command> result;
		auto decoded = Si::make_transforming_source<boost::optional<bf::command>>(code, bf::detect_command);
		for (;;)
		{
			auto cmd = Si::get(decoded);
			if (!cmd)
			{
				break;
			}
			if (*cmd)
			{
				result.emplace_back(**cmd);
			}
		}
		return result;
	}
}

BOOST_AUTO_TEST_CASE(bf_hello_world)
{
	std::string const hello_world = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
	std::array<char, 100> memory{ {} };
	Si::empty<char> input;
	auto interpreter = bf::execute(input, scan_all(Si::make_container_source(hello_world)), memory, 0);
	std::string printed;
	auto done = Si::for_each(std::move(interpreter), [&printed](char output)
	{
		printed.push_back(output);
	});
	done.start();
	BOOST_CHECK_EQUAL("Hello World!\n", printed);
}
