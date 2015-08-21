#include <silicium/terminate_on_exception.hpp>
#include <silicium/variant.hpp>
#include <ctime>
#include <iostream>

int main()
{
#if SILICIUM_HAS_VARIANT
	Si::variant<int, std::time_t, double> r(std::time(nullptr));
	return Si::visit<int>(
		r,
		[](int v)
		{
			return v;
		},
		[](std::time_t t)
		{
			return t != 0;
		},
		[](double d)
		{
			return std::isnan(d);
		}
	);
#else
	std::cerr << "This example requires a more recent compiler for Si::variant to work\n";
#endif
}
