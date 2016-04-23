#include <experimental/filesystem>
#include <experimental/generator>
#include <experimental/resumable>
#include <boost/range/algorithm/copy.hpp>
#include <iostream>

namespace
{
    std::experimental::generator<char> generate_something()
    {
        __yield_value 'x';
    }
}

int main()
{
    for (auto &&e : generate_something())
    {
        std::cout << e;
    }
}
