#include "tcp_socket.hpp"

#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32")

byte string_process::key_[CryptoPP::DES::DEFAULT_KEYLENGTH];

tcp_socket::tcp_socket(string_process** const processer) : socket_(0), addr_len_(0)
{
	processer_ = *processer;
}

tcp_socket::tcp_socket(const char*& ip, const u_short& port, string_process** const processer)
{
	if (!initialize_socket())
		throw std::exception("Some errors caused while initializing socket.");
	if (!bind_socket(ip, port))
		throw std::exception("Some errors caused while binding socket.");
	processer_ = *processer;
}

tcp_socket::tcp_socket(const u_long& ip, const u_short& port, string_process** const processer)
{
	if (!initialize_socket())
		throw std::exception("Some errors caused while initializing socket.");
	if (!bind_socket(ip, port))
		throw std::exception("Some errors caused while binding socket.");
	processer_ = *processer;
}

tcp_socket::~tcp_socket()
{
	if (is_initialized_)
		this->stop();
}

bool tcp_socket::initialize_socket()
{
	socket_ = socket(AF_INET, SOCK_STREAM, 0);
	is_initialized_ = socket_ != INVALID_SOCKET;

	return is_initialized_;
}

bool tcp_socket::bind_socket(const char*& ip, const u_short& port)
{
	ZeroMemory(&address_, sizeof address_);

	address_.sin_family = AF_INET;
	address_.sin_addr.S_un.S_addr = inet_addr(ip);
	address_.sin_port = htons(port);

	is_bound_ = bind(socket_, reinterpret_cast<SOCKADDR*>(&address_), sizeof address_) != SOCKET_ERROR;

	return is_bound_;
}

bool tcp_socket::bind_socket(const u_long& ip, const u_short& port)
{
	ZeroMemory(&address_, sizeof address_);

	address_.sin_family = AF_INET;
	address_.sin_addr.S_un.S_addr = ip;
	address_.sin_port = htons(port);

	is_bound_ = bind(socket_, reinterpret_cast<SOCKADDR*>(&address_), sizeof address_) != SOCKET_ERROR;

	return is_bound_;
}

bool tcp_socket::start() const
{
	return listen(socket_, SOMAXCONN) != SOCKET_ERROR;
}

bool tcp_socket::connect_to(const std::string& ip, const u_short& port)
{
	if (!is_initialized_)
		initialize_socket();
	memset(&address_, 0, sizeof(SOCKADDR_IN));

	address_.sin_family = AF_INET;
	address_.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	address_.sin_port = htons(port);

	return connect(socket_, reinterpret_cast<SOCKADDR*>(&address_), sizeof address_) != SOCKET_ERROR;
}

void tcp_socket::accept_client(tcp_socket** const socket) const
{
	(*socket)->addr_len_ = sizeof (*socket)->address_;

	(*socket)->socket_ = accept(socket_, reinterpret_cast<SOCKADDR*>(&(*socket)->address_), &(*socket)->addr_len_);
	(*socket)->is_initialized_ = (*socket)->socket_ != INVALID_SOCKET;

	if (!(*socket)->is_initialized_)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());
}

bool tcp_socket::send(const std::string& data)
{
	return send_data(processer_ ? processer_->compress(data) : data);
}

std::string tcp_socket::recv()
{
	return processer_ ? processer_->decompress(receive_data()) : receive_data();
}

std::string tcp_socket::receive_data()
{
	char buffer[1024];

	const auto recvlen = ::recv(socket_, buffer, 1024, 0);
	// Disconnect
	if (recvlen <= 0)
	{
		this->stop();
		return processer_ ? processer_->compress("Disconnected") : "Disconnected";
	}

	//buffer[recvlen] = 0;

	return buffer;
}

bool tcp_socket::send_data(const std::string& data)
{
	const auto ret = ::send(socket_, data.c_str(), data.length(), 0);
	if (ret == -1)
	{
		this->stop();
		return false;
	}
	return true;
}

void tcp_socket::stop()
{
	if (is_initialized_)
	{
		is_initialized_ = false;
		closesocket(socket_);
	}
}
