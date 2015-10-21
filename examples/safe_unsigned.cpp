#include <silicium/safe_arithmetic.hpp>
#include <iostream>

extern "C" bool checked_fma(boost::uint64_t *a, boost::uint64_t b, boost::uint64_t c)
{
	// Look at the optimized assembler of this function. You will not see any function call, so the
	// inlining works perfectly.
	// The generated code may not be optimal yet because the operators have not been optimized in
	// this regard.
	Si::overflow_or<Si::safe_number<boost::uint64_t>> product = Si::safe(b) * Si::safe(c);
	Si::overflow_or<Si::safe_number<boost::uint64_t>> sum = Si::safe(*a) + product;
	if (sum.is_overflow())
	{
		return false;
	}
	*a = sum.value()->value;
	return true;
}

namespace
{
	template <class T>
	void show_add(T first, T second)
	{
		std::cerr << first << " + " << second << " = " << (Si::safe(first) + Si::safe(second)) << '\n';
	}
}

int main()
{
	show_add(1u, 2u);
	show_add(std::numeric_limits<unsigned>::max(), 1u);
	show_add(std::numeric_limits<unsigned long long>::max(), 1ull);
	show_add(std::numeric_limits<unsigned long long>::max(), std::numeric_limits<unsigned long long>::max());
}
