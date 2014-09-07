#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>
#include <silicium/transform_if_initialized.hpp>
#include <silicium/empty.hpp>
#include <silicium/coroutine.hpp>
#include <silicium/for_each.hpp>

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
			auto pointer = original_pointer;
			for (command com : program)
			{
				switch (com)
				{
				case command::ptr_increment:
					++pointer;
					if (pointer == boost::size(memory))
					{
						pointer = 0;
					}
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
					break;

				case command::value_increment:
					++memory[pointer];
					break;

				case command::value_decrement:
					--memory[pointer];
					break;

				case command::write:
					yield(static_cast<char>(memory[pointer]));
					break;

				case command::read:
					{
						auto in = yield.get_one(input);
						if (!in)
						{
							return;
						}
						memory[pointer] = *in;
						break;
					}
				}
			}
		});
	}
}

BOOST_AUTO_TEST_CASE(bf_empty)
{
	Si::empty<char> source;
	std::array<char, 1> memory{{}};
	auto interpreter = bf::execute(source, boost::iterator_range<bf::command const *>(), memory, 0);
	auto done = Si::for_each(std::move(interpreter), [](char output)
	{
		boost::ignore_unused_variable_warning(output);
		BOOST_FAIL("no output expected");
	});
	done.start();
}
