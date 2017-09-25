#pragma once

#include "keilo_application.hpp"

#include <memory>
#include <list>
#include <atomic>
#include <mutex>
#include <queue>
#include <queue>
#include <winsock2.h>

class keilo_server
{
public:
	enum : int{
		create = 1,
		select,
		insert,
		update,
		remove,
		drop
	};

	enum : int {
		database = 1,
		table
	};

public:
	keilo_server();
	keilo_server(int port);
	~keilo_server();

public:
	void run(int port);

	void import_file(std::string file_name);

private:
	void initialize();
	void process_client(std::pair<SOCKET, sockaddr_in> client);

	std::string process_message(const char* message);

	int get_message_type(const char* message);
	int get_secondary_type(const char* message);

	void disconnect_client(SOCKET client);

private:
	void print_output();

private:
	std::thread* accept_thread = nullptr;

private:
	// application
	std::string create_database(std::string message, size_t pos);
	std::string select_database(std::string message, size_t pos);

	// database
	std::string create_table(std::string message, size_t pos);
	std::string select_table(std::string message, size_t pos);
	std::string drop_table(std::string message, size_t pos);

	// table
	std::string insert_record(std::string message, size_t pos);
	std::string update_record(std::string message, size_t pos);
	std::string remove_record(std::string message, size_t pos);

private:
	keilo_database* selected_database = nullptr;
	keilo_table* selected_table = nullptr;

private:
	SOCKET m_socket;
	WSADATA m_wsadata;
	int m_port;

	std::atomic<bool> running;

	std::queue<std::string> m_outputs;

	std::unique_ptr<keilo_application> m_application;
	std::list<std::pair<SOCKET, sockaddr_in>> m_clients;
	std::list<std::thread> m_client_processes;

private:
	std::thread print_thread;
};

