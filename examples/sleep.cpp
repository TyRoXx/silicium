#include <boost/asio.hpp>
#include <silicium/coroutine.hpp>
#include <silicium/asio/timer.hpp>
#include <silicium/total_consumer.hpp>
#include <silicium/constant_observable.hpp>
#include <iostream>
//TODO: fewer includes for such a simple example?

template <class Duration>
void sleep(boost::asio::io_service &io, Si::yield_context yield, Duration duration)
{
	//TODO: the duration should not have to be wrapped in an observable
	auto timer = Si::make_timer(io, Si::make_constant_observable(duration));
	//TODO: use the call operator instead of a get_one method?
	boost::optional<Si::timer_elapsed> result = yield.get_one(timer);
	//TODO: this should work without the optional wrapper
	assert(result);
}

int main()
{
	boost::asio::io_service io;
	//TODO: this has to work without total_consumer
	auto coro = Si::make_total_consumer(Si::make_coroutine([&io](Si::yield_context yield) -> Si::nothing //TODO: the return type should be void
	{
		std::cout << "Going to sleep" << std::endl;
		sleep(io, yield, std::chrono::seconds(1));
		std::cout << "Waking up\n";
		return {};
	}));
	//TODO: should coroutines start immediately by default?
	coro.start();
	io.run();
}
