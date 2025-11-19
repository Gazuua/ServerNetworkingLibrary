#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>

#include "macro.h"
#include "session.h"

NAMESPACE_BEGIN(snl)

// TODO : move to global header file?
constexpr std::size_t MAX_SESSION_COUNT = 1000;

class Service : public std::enable_shared_from_this<Service>
{
public:
	explicit Service(boost::asio::io_context& ioc, const std::string& ip, const std::string& port)
		: _ioc(ioc)
		, _acceptor(ioc)
	{
		_ip = ip;
		_port = port;
	}

	Service() = delete;
	~Service()
	{
		stop();
	}

	void start()
	{
		boost::asio::ip::tcp::resolver resolver(_ioc);
		boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(_ip, _port).begin();

		_acceptor.open(endpoint.protocol());
		_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
		_acceptor.bind(endpoint);
		_acceptor.listen();

		boost::asio::co_spawn(_ioc, std::bind(&Service::accept_handler, shared_from_this()), boost::asio::detached);
	}

	void stop()
	{
		// TODO : service stop logic
	}

private:
	boost::asio::awaitable<void> accept_handler()
	{
		boost::system::error_code ec;
		for (;;)
		{
			ec.clear();

			boost::asio::ip::tcp::socket socket
				= co_await _acceptor.async_accept(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
			if (ec)
				co_return;

			// TODO : session key 생성 로직 필요
			static std::uint32_t session_key = 0;
			auto session = std::make_shared<Session>(_ioc, std::move(socket), ++session_key);
			{
				std::lock_guard<std::mutex> lock(_session_map_mutex);
				_session_map.emplace(session_key, session);
				// TODO : MAX_SESSION_COUNT 초과 시 예외처리
			}
		}
	}

	boost::asio::io_context& _ioc;
	boost::asio::ip::tcp::acceptor _acceptor;
	std::string _ip;
	std::string _port;
	
	std::unordered_map<std::uint32_t, std::shared_ptr<Session>> _session_map;
	std::mutex _session_map_mutex;
};
NAMESPACE_END(snl)
