#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(boost::asio::io_context& ioc, const std::string& ip, const std::string& port)
		: _ioc(ioc), _acceptor(ioc)
	{
		_ip = ip;
		_port = port;
	}

	Service() = delete;
	~Service()
	{
		stop();
	}

	void init()
	{
		boost::asio::ip::tcp::resolver resolver(_ioc);
		boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(_ip, _port).begin();

		_acceptor.open(endpoint.protocol());
		_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
		_acceptor.bind(endpoint);
		_acceptor.listen();

		boost::asio::co_spawn(_ioc, std::bind(&Service::accept_handler, shared_from_this()), boost::asio::detached);
	}

	void start()
	{

	}

	void stop()
	{

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

			// TODO : Session 클래스 정의
			// 안 기다려도 되는 앵간한 작업들은 싹다 detached 코루틴으로 백그라운드 처리하면 효율 up
			// boost::asio::co_spawn(_ioc, Session(std::move(socket)), boost::asio::detached);
		}
	}

	boost::asio::io_context& _ioc;
	boost::asio::ip::tcp::acceptor _acceptor;
	std::string _ip;
	std::string _port;
};