#include <silicium/asio/timer.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/terminate_on_exception.hpp>
#include <iostream>

template <class YieldContext, class Duration>
void sleep(boost::asio::io_service &io, YieldContext &&yield, Duration duration)
{
	auto timer = Si::asio::make_timer(io);
	timer.expires_from_now(duration);
	//TODO: use the call operator instead of a get_one method?
	Si::optional<Si::asio::timer_elapsed> result = yield.get_one(Si::ref(timer));
	//TODO: this should work without the optional wrapper
	assert(result);
}

int main()
{
	boost::asio::io_service io;
	Si::spawn_coroutine([&io](Si::spawn_context yield)
	{
		std::cout << "Going to sleep" << std::endl;
		sleep(io, yield, boost::chrono::seconds(1));
		std::cout << "Waking up\n";
	});
	io.run();
}
