#include "keilo_server.hpp"

#include <winsock2.h>
#include <utility>
#include <thread>
#include <iostream>
#include <sstream>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

keilo_server::keilo_server() : m_application(new keilo_application()), m_clients(std::list<std::pair<SOCKET, sockaddr_in>>()), m_client_processes(std::list<std::thread>())
{
	print_thread = std::thread(&keilo_server::print_output, this);
}

keilo_server::keilo_server(int port) : m_application(new keilo_application()), m_clients(std::list<std::pair<SOCKET, sockaddr_in>>()), m_client_processes(std::list<std::thread>()), m_port(port)
{
}

keilo_server::~keilo_server()
{
	if (running.load())
		running = false;
	if (accept_thread->joinable())
		accept_thread->join();
	if (accept_thread)
		delete accept_thread;
	for (auto& client : m_clients) {
		closesocket(client.first);
	}
	closesocket(m_socket);
}

void keilo_server::run(int port)
{
	this->m_port = port;
	initialize();
}

void keilo_server::import_file(std::string file_name)
{
	m_application->import_file(file_name);
}

void keilo_server::initialize()
{
	sockaddr_in server_address;

	if (WSAStartup(MAKEWORD(2, 2), &m_wsadata))
		throw std::exception("WSAStartup error");

	m_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
		throw std::exception("socket error");

	ZeroMemory(&server_address, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(m_port);

	if (bind(m_socket, (SOCKADDR*)&server_address, sizeof server_address) == SOCKET_ERROR)
		throw std::exception("bind error");

	if (listen(m_socket, 5) == SOCKET_ERROR)
		throw std::exception("listen error");

	running = true;

	accept_thread = new std::thread([&]() {
		while (running.load()) {
			SOCKET client_socket;
			sockaddr_in client_address;
			int size_of_client_address = sizeof client_address;
			client_socket = accept(m_socket, (SOCKADDR*)&client_address, &size_of_client_address);
			if (client_socket == INVALID_SOCKET)
				throw std::exception("accept error");
			std::pair<SOCKET, sockaddr_in> client{ client_socket, client_address };
			m_clients.push_back(client);
			m_client_processes.push_back(std::thread([&]() {
				bool found = false;
				do {
					process_client(client);
					for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
						if (it->first == client.first) {
							found = true;
						}
					}
				} while (found);
			}));
		}
	});
}

void keilo_server::process_client(std::pair<SOCKET, sockaddr_in> client)
{
	send(client.first, "", 0, 0);
	char buffer[1024];
	int buffer_byte;
	while ((buffer_byte = recv(client.first, buffer, 1024, 0)) > 0) {
		buffer[buffer_byte] = 0;
		std::stringstream a;
		a << "[client_process] : " << buffer;
		m_outputs.push(a.str());
		auto result = process_message(buffer);
		send(client.first, result.c_str(), result.length(), 0);
	}
}

std::string keilo_server::process_message(const char * message)
{
	std::stringstream result;
	std::stringstream buffer;

	auto i = 0;
	while (message[i] == ' ')
		buffer << message[i++];
	i += 1;

	switch (get_message_type(buffer.str().c_str())) {
	case create:
		buffer.clear();
		while (message[i] == ' ')
			buffer << message[i++];
		i += 1;
		switch (get_secondary_type(buffer.str().c_str())) {
		case database:
			result << create_database(message, i);
			break;
		case table:
			result << create_table(message, i);
			break;
		default:
			result << "Unknown command.";
		}
		break;
	case select:
		buffer.clear();
		while (message[i] == ' ')
			buffer << message[i++];
		i += 1;
		switch (get_secondary_type(buffer.str().c_str())) {
		case database:
			result << select_database(message, i);
			break;
		case table:
			result << select_table(message, i);
			break;
		default:
			result << "Unknown command.";
		}
		break;
	case insert:
		result << insert_record(message, i);
		break;
	case update:
		result << update_record(message, i);
		break;
	case remove:
		result << remove_record(message, i);
		break;
	case drop:
		result << drop_table(message, i);
		break;
	default:
		result << "Unknown command.";
	}

	return result.str();
}

void keilo_server::disconnect_client(SOCKET _client)
{
	auto selected_client = m_clients.end();
	char host[15] = { 0, };

	for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
		if (it->first == _client) {
			selected_client = it;
			inet_ntop(PF_INET, &it->second, host, 15);
			break;
		}
	}

	if (selected_client == m_clients.end())
		throw std::exception("[disconnect_client] Could not find client.");

	closesocket(selected_client->first);
	std::cout << "[disconnect_client] " << " (" << host << ") disconnected." << std::endl;
	m_clients.erase(selected_client);

	for (auto& process : m_client_processes) {
		if (process.joinable())
			process.join();
	}
}

void keilo_server::print_output()
{
	while (true)
		if (m_outputs.size() > 0) {
			std::cout << m_outputs.front() << std::endl;
			m_outputs.pop();
		}
}

int keilo_server::get_message_type(const char * message)
{
	auto type = 0;
	if (strcmp("create", message) == 0)
		type = create;
	if (strcmp("select", message) == 0)
		type = select;
	if (strcmp("insert", message) == 0)
		type = insert;
	if (strcmp("update", message) == 0)
		type = update;
	if (strcmp("remove", message) == 0)
		type = remove;
	if (strcmp("drop", message) == 0)
		type = drop;

	return type;
}

int keilo_server::get_secondary_type(const char * message)
{
	auto type = 0;
	if (strcmp("table", message) == 0)
		type = table;
	if (strcmp("database", message) == 0)
		type = database;
	return type;
}

std::string keilo_server::create_database(std::string message, size_t pos)
{
	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	return m_application->create_database(name.str());
}

std::string keilo_server::select_database(std::string message, size_t pos)
{
	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	selected_database = m_application->select_database(name.str());

	if (selected_database)
		return "Successfully selected database that was named \"" + name.str() + "\".";
	else
		return "Database that was named \"" + name.str() + "\" does not exist in server";
}

std::string keilo_server::create_table(std::string message, size_t pos)
{
	if (!selected_database)
		return "Please select database";

	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	return selected_database->create_table(name.str());
}

std::string keilo_server::select_table(std::string message, size_t pos)
{
	if (!selected_database)
		return "Please select database";

	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	selected_table = selected_database->select_table(name.str());

	if (!selected_table)
		return "Table that was named \"" + name.str() + "\" does not exist in database \"" + selected_database->get_name() + "\".";
	else
		return "Successfully selected table that was named \"" + name.str() + "\".";

}

std::string keilo_server::drop_table(std::string message, size_t pos)
{
	std::stringstream name;
	while (message[pos] != ';')
		name << message[pos++];

	return selected_database->drop_table(name.str());
}

std::string keilo_server::insert_record(std::string message, size_t pos)
{
	while (message[pos - 1 != '('])
		pos++;

	keilo_record record;
	while (message[pos] != ')') {
		while (message[pos] != ';') {
			std::stringstream identifier;
			while (message[pos] != ':')
				identifier << message[pos++];

			std::stringstream value;
			while (message[pos] != ';')
				value << message[pos++];
			record.push_back(keilo_instance{ identifier.str(), value.str() });
		}
	}

	return selected_table->insert(record);
}

std::string keilo_server::update_record(std::string message, size_t pos)
{
	std::stringstream condition_identifier;
	while (message[pos] != ':')
		condition_identifier << message[pos++];

	std::stringstream condition_value;
	while (message[pos] != ' ')
		condition_value << message[pos++];

	std::stringstream new_identifier;
	while (message[pos] != ':')
		new_identifier << message[pos++];

	std::stringstream new_value;
	while (message[pos] != ';')
		new_value << message[pos++];

	return selected_table->update(keilo_instance{ condition_identifier.str(), condition_value.str() }, keilo_instance{ new_identifier.str(), new_value.str() });
}

std::string keilo_server::remove_record(std::string message, size_t pos)
{
	std::stringstream identifier;
	while (message[pos] != ':')
		identifier << message[pos++];

	std::stringstream value;
	while (message[pos] != ';')
		value << message[pos++];

	return selected_table->remove(keilo_instance{ identifier.str(), value.str() });
}
