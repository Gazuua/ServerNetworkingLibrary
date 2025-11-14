#include "app.h"

int main()
{
	boost::asio::io_context ioc;
	snl::App app{ ioc };
	app
		.set_timer_interval(std::chrono::milliseconds(1))
		.set_timer_function([]() { static std::atomic<int> ticks{ 0 }; std::cout << "tick on thread " << std::this_thread::get_id() << " : " << ++ticks << std::endl; })
		.run();

	app.wait_signal();
}