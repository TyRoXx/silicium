#include <silicium/fast_variant.hpp>
#include <ctime>

int main()
{
	Si::fast_variant<int, std::time_t, double> r(std::time(nullptr));
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
}
