#pragma once

#include "macro.h"

#include <iostream>
#include <vector>
#include <boost/asio.hpp>

class Server;
class Client;

NAMESPACE_BEGIN(snl)
class App
{
public:
	App() = default;
	~App()
	{
		if (_is_running == true)
			Stop();
	}

	void Init()
	{
		_signals.add(SIGTERM);
		_signals.async_wait([this](const boost::system::error_code& /*ec*/, int /*signum*/)
			{
				Stop();
			});

		// 타이머 시작 (1초 주기 예시)
		/*
		_timer.expires_after(std::chrono::seconds(1));
		_timer.async_wait(std::bind(&App::OnTimer, this, std::placeholders::_1));
		*/
	}

	void Run()
	{
		_is_running = true;

		for (const auto& server : _server_list)
			;
			// server.Start();

		for (const auto& client : _client_list)
			;
			// client.Start();

		_io_context.run();
	}

	void Stop()
	{
		_timer.cancel();

		boost::system::error_code ec;
		_signals.cancel(ec);

		_io_context.stop();
	}

private:
	boost::asio::io_context _io_context;
	boost::asio::high_resolution_timer _timer;

	boost::asio::signal_set _signals;

	std::atomic<bool> _is_running{ false };

	std::vector<Server> _server_list{};
	std::vector<Client> _client_list{};
};
NAMESPACE_END(snl)