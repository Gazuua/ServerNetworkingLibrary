#pragma once

#include <boost/asio.hpp>

#include "macro.h"

NAMESPACE_BEGIN(snl)
class Service;

// TODO : move to global header file?
constexpr std::chrono::seconds READ_TIMEOUT = std::chrono::seconds(300);

class Session : public std::enable_shared_from_this<Session>
{
public:
	explicit Session(boost::asio::io_context& ioc, boost::asio::ip::tcp::socket socket, std::uint32_t session_key)
		: _ioc(ioc)
		, _socket(std::move(socket))
		, _strand(boost::asio::make_strand(_ioc))
		, _session_key(session_key)
	{
	}

	Session() = delete;
	virtual ~Session() = default;

	void set_service(std::shared_ptr<Service> service)
	{
		_service = service;
	}

	void start()
	{
		// TODO : session start logic
		// read, write 코루틴 핸들러 정의 및 실행 (read timeout 고려)
		// 네트워크 IO 테스트 및 정상 동작 확인
	}

	void stop()
	{
		// TODO : session stop logic

		// boost::system::error_code ec;
		// _socket.close(ec);
	}

private:
	std::weak_ptr<Service> _service;
	boost::asio::io_context& _ioc;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::strand<boost::asio::io_context::executor_type> _strand;
	std::uint32_t _session_key;

	// std::array<uint8_t, BUFFER_SIZE> _read_buffer;
	// std::array<uint8_t, BUFFER_SIZE> _write_buffer;
};

NAMESPACE_END(snl)
