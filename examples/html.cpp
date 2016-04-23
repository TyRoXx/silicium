#include <silicium/html/tree.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <vector>
#include <iostream>

int main()
{
#if SILICIUM_HAS_HTML_TREE
    using namespace Si::html;
    auto document = tag(
        "html",
        tag("head", tag("title", text("Title"))) +
            tag("body",
                text("Hello, ") + raw("<b>world</b>") +
                    fixed_length<5>(
                        [](Si::Sink<char, Si::success>::interface &destination)
                        {
                            Si::html::unpaired_element(destination, "br");
                        })));
    std::vector<char> generated;
    auto sink =
        Si::Sink<char, Si::success>::erase(Si::make_container_sink(generated));
    document.generate(sink);
    std::cout.write(
        generated.data(), static_cast<std::streamsize>(generated.size()));
    std::cout << '\n';
#else
    std::cerr << "This example requires a more recent compiler\n";
#endif
}
