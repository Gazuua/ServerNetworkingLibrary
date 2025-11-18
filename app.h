#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>

#include "macro.h"
#include "service.h"

NAMESPACE_BEGIN(snl)
class App
{
	using self_t = App;

public:
	explicit App(boost::asio::io_context& ioc)
		: _ioc(ioc)
		, _signals(_ioc, SIGINT, SIGTERM)
		, _concurrency(std::max(1u, std::thread::hardware_concurrency()))
		, _timer(_ioc)
		, _timer_interval(std::chrono::milliseconds(1000))
		, _timer_function(nullptr)
		, _started(false)
	{
		_threads.reserve(_concurrency);
		std::cout << "current concurrency : " << _concurrency << std::endl;
	}

	App() = delete;

	~App()
	{
		stop();
	}

	self_t& set_concurrency(std::size_t concurrency)
	{
		_concurrency = concurrency;
		return *this;
	}

	self_t& set_timer_function(std::function<void()> func)
	{
		_timer_function = func;
		return *this;
	}

	self_t& set_timer_interval(std::chrono::milliseconds ms)
	{
		_timer_interval = ms;
		return *this;
	}

	void run()
	{
		if (_started.exchange(true) == true)
			return;

		boost::asio::co_spawn(_signals.get_executor(), signal_handler(), boost::asio::detached);
		boost::asio::co_spawn(_timer.get_executor(), timer_handler(), boost::asio::detached);

		for (const auto& service : _services)
			service->start();

		for (size_t count = 0; count < _concurrency; ++count)
		{
			_threads.emplace_back(std::thread([this]()
				{
					std::cout << "thread started!! id : " << std::this_thread::get_id() << std::endl;
					_ioc.run();
				}));
		}
	}

	void stop()
	{
		if (_started.exchange(false) == false)
			return;

		boost::system::error_code ec;
		_signals.cancel(ec);
		_timer.cancel();
		_ioc.stop();

		for (auto& thread : _threads)
		{
			if (thread.joinable())
				thread.join();
		}

		_threads.clear();
	}

	void wait_signal()
	{
		_stop_promise.get_future().wait();
	}

private:
	boost::asio::awaitable<void> signal_handler()
	{
		co_await _signals.async_wait(boost::asio::use_awaitable);
		_stop_promise.set_value();
		co_return;
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

			if (_timer_function != nullptr)
				_timer_function();
		}
	}

	boost::asio::io_context& _ioc;

	// TODO : signal manager로 분리
	boost::asio::signal_set _signals;
	std::promise<void> _stop_promise;

	std::size_t _concurrency;

	// TODO : timer manager로 분리
	boost::asio::high_resolution_timer _timer;
	std::chrono::milliseconds _timer_interval;
	std::function<void()> _timer_function;

	std::atomic<bool> _started;

	// TODO : service 객체 정의
	std::vector<std::shared_ptr<Service>> _services;

	// TODO : thread manager 로 분리
	std::vector<std::thread> _threads;
};
NAMESPACE_END(snl)
