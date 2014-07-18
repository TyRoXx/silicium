#include <reactive/coroutine.hpp>
#include <silicium/http/http.hpp>
#include <silicium/fast_variant.hpp>
#include <reactive/total_consumer.hpp>
#include <reactive/consume.hpp>
#include <reactive/enumerate.hpp>
#include <reactive/linux/inotify.hpp>

int main()
{
	boost::asio::io_service io;
	rx::linux::inotify_observable notifier(io);
	auto w = notifier.watch("/home/virtual/dev", IN_ALL_EVENTS);
	auto printer = rx::transform(rx::enumerate(rx::ref(notifier)), [](rx::linux::file_notification const &event) -> rx::detail::nothing
	{
		std::cerr << event.mask << " " << event.name << '\n';
		return rx::detail::nothing{};
	});
	auto all = rx::make_total_consumer(printer);
	all.start();
	io.run();
}
