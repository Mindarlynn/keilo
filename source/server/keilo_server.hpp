#pragma once

#include "keilo_application.hpp"
#include "keilo_core.hpp"

#include <memory>
#include <list>
#include <atomic>
#include <mutex>
#include <queue>


class keilo_server
{
public:
	
	keilo_server(int port);
	~keilo_server();

public:  // user accessable functions
	void run();
	void run_local();

	std::string import_file(std::string file_name, bool ps = true);

private: // outupt
	void print_output();
	void push_output(const std::string message);

	std::thread print_thread;
	std::queue<std::string> m_outputs;
	std::mutex m_output_mutex;
	std::atomic<bool> printing = false;

private: // networking
	void accept_client();
	void process_client(client& _client, keilo_database* database, keilo_table* table);
	const std::string process_message(std::string message, keilo_database* database, keilo_table* table);
	void disconnect_client(client& _client);

	std::thread accept_thread;
	int m_port;
	std::list<client> m_clients;
	SOCKET m_socket;
	WSADATA m_wsa;
	SOCKADDR_IN m_addr;
	std::list<std::thread> m_client_processes;


private: // database processing
	std::string create_database(std::string message, size_t pos);
	std::string select_database(std::string message, size_t pos, keilo_database* database);
	std::string export_database(std::string message, size_t pos, keilo_database* database);
	std::string import_database(std::string message, size_t pos);

	std::string create_table(std::string message, size_t pos, keilo_database* database);
	std::string select_table(std::string message, size_t pos, keilo_database* database, keilo_table* table);
	std::string join_table(std::string message, size_t pos, keilo_database* database, keilo_table* table);
	std::string drop_table(std::string message, size_t pos, keilo_database* database, keilo_table* table);

	std::string select_record(std::string message, size_t pos, keilo_table* table);
	std::string insert_record(std::string message, size_t pos, keilo_table* table);
	std::string update_record(std::string message, size_t pos, keilo_table* table);
	std::string remove_record(std::string message, size_t pos, keilo_table* table);

private: // commands
	const std::string CREATE = "create";
	const std::string SELECT = "select";
	const std::string JOIN = "join";
	const std::string INSERT = "insert";
	const std::string UPDATE = "update";
	const std::string REMOVE = "remove";
	const std::string DROP = "drop";
	const std::string EXPORT_FILE = "export";
	const std::string IMPORT_FILE = "import";
	const std::string CLEAR = "clear";
	const std::string DATABASE = "database";
	const std::string TABLE = "table";
	const std::string RECORD = "record";

private:
	std::atomic<bool> running = false;

	std::unique_ptr<keilo_application> m_application;
};