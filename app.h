#pragma once
#include "macro.h"

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>

class Service;

NAMESPACE_BEGIN(snl)
class App
{
	using self_t = App;

public:
	App(boost::asio::io_context& ioc)
		: _ioc(ioc)
		, _signals(_ioc, SIGINT, SIGTERM)
		, _timer(_ioc)
		, _timer_interval(std::chrono::milliseconds(1000))
		, tick_function(nullptr)
		, _started(false)
	{
	}

	~App()
	{
		if (_started == true)
			stop();
	}

	void run()
	{
		if (_started.exchange(true) == true)
			return;

		boost::asio::co_spawn(_signals.get_executor(), signal_handler(), boost::asio::detached);
		boost::asio::co_spawn(_timer.get_executor(), timer_handler(), boost::asio::detached);

		for (const auto& service : _services)
			; // service.Start();

		_ioc.run();
	}

	void stop()
	{
		if (_started.exchange(false) == false)
			return;

		boost::system::error_code ec;
		_signals.cancel(ec);
		_timer.cancel();
		_ioc.stop();
	}

	boost::asio::awaitable<void> signal_handler()
	{
		co_await _signals.async_wait(boost::asio::use_awaitable);
		stop();
	}	

	boost::asio::awaitable<void> timer_handler()
	{
		boost::system::error_code ec;
		for(;;)
		{
			ec.clear();

			_timer.expires_after(_timer_interval);
			co_await _timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
			if (ec) // _timer.cancel() called or Error returned
				co_return;

			if (tick_function != nullptr)
				tick_function();
		}
	}

private:
	boost::asio::io_context& _ioc;
	boost::asio::signal_set _signals;

	boost::asio::high_resolution_timer _timer;
	std::chrono::milliseconds _timer_interval;
	std::function<void()> tick_function;

	std::atomic<bool> _started;

	std::vector<std::shared_ptr<Service>> _services;
};
NAMESPACE_END(snl)
