#include "keilo_server.hpp"
#include "keilo_core.hpp"

#include <utility>
#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <cstdlib>
#include <winsock2.h>

#pragma warning(disable:4996)

keilo_server::keilo_server(int port) : 
	m_application(new keilo_application()), 
	m_clients(std::list <client> ()), 
	m_client_processes(std::list<std::thread>()), 
	m_port(port), 
	print_thread(std::thread(&keilo_server::print_output, this)),
	accept_thread(std::thread(&keilo_server::accept_client, this))
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());
	
	if (m_socket = socket(AF_INET, SOCK_STREAM, 0); m_socket == INVALID_SOCKET)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());

	ZeroMemory(&m_addr, sizeof SOCKADDR_IN);

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	m_addr.sin_port = htons(m_port);

	if (bind(m_socket, (SOCKADDR*)&m_addr, sizeof m_addr) == SOCKET_ERROR)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());

	std::cout << "Successfully initialized winsock2." << std::endl;
}

keilo_server::~keilo_server()
{
	if (running.load())
		running = false;
	if (accept_thread.joinable()) {
		SOCKET tmp = socket(AF_INET, SOCK_STREAM, 0);
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(6060);
		connect(tmp, (SOCKADDR*)&addr, sizeof addr);
		closesocket(tmp);
		accept_thread.join();
	}
	if (print_thread.joinable())
		print_thread.join();
	for (auto& processes : m_client_processes)
		if (processes.joinable())
			processes.join();
	for (auto& client : m_clients)
		closesocket(client.sock);
	closesocket(m_socket);
}

void keilo_server::run()
{
	if (listen(m_socket, 10) == SOCKET_ERROR)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());
	running = true;
	push_output("Successfully started server.");
	while (running.load());
}

void keilo_server::run_local()
{
	keilo_database* selected_database = nullptr;
	keilo_table* selected_table = nullptr;
	std::string input;
	while (running.load()) {
		while (printing.load());
		std::stringstream select_status;
		if (selected_database) {
			select_status << "[" << selected_database->get_name();
			if (selected_table)
				select_status << "_" << selected_table->get_name();
			select_status << "]> ";
		}
		else
			select_status << "[none]> ";

		std::cout << select_status.str();

		if (std::getline(std::cin, input); input == "exit") {
			running = false;
			break;
		}
		
		push_output(process_message(input, &selected_database, &selected_table));
	}
}

std::string keilo_server::import_file(std::string file_name, bool ps)
{
	auto output = m_application->import_file(file_name);
	if (ps) {
		push_output(output);
	}
	return output;
}

void keilo_server::print_output()
{
	while (!running.load());
	while (running.load()) {
		m_output_mutex.lock();
		while (m_outputs.size() > 0) {
			printing = true;
			std::cout << m_outputs.front() << std::endl;
			m_outputs.pop();
		}
		m_output_mutex.unlock();
		if (printing.load())
			printing = false;
	}
}

void keilo_server::push_output(const std::string message)
{
	m_output_mutex.lock();
	m_outputs.push(message);
	m_output_mutex.unlock();
}

void keilo_server::accept_client()
{
	while (!running.load());
	while (running.load()) {
		client _client;
		int addrlen = sizeof _client.addr;

		_client.sock = accept(m_socket, (SOCKADDR*)&_client.addr, &addrlen);

		std::string addr = inet_ntoa(_client.addr.sin_addr);
		push_output("[" + addr + ":" + std::to_string(_client.addr.sin_port) +"] connected.");

		m_clients.push_back(_client);

		m_client_processes.push_back(std::thread([&]() {
			auto client_addr = _client.addr;
			keilo_database* selected_database = nullptr;
			keilo_table* selected_table = nullptr;

			while (running.load()) {
				auto found = m_clients.end();
				for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
					if (it->addr.sin_addr.S_un.S_addr == client_addr.sin_addr.S_un.S_addr &&
						it->addr.sin_port == client_addr.sin_port)
					{
						found = it;
						break;
					}
				}
				if (found == m_clients.end())
					break;
				else
					process_client(*found, &selected_database, &selected_table);
			}
		}));
	}
}

void keilo_server::process_client(client& _client, keilo_database** database, keilo_table** table)
{
	char read_data[1024];
	auto received = recv(_client.sock, read_data, 1024, 0);
	if (received == 0) {
		disconnect_client(_client);
		return;
	}

	read_data[received] = 0;

	std::string addr = inet_ntoa(_client.addr.sin_addr);
	push_output("[" + addr + ":" + std::to_string(_client.addr.sin_port) + "] " + std::string(read_data));
	
	auto processed_message = process_message(read_data, database, table);
	if (send(_client.sock, processed_message.c_str(), processed_message.length(), 0) == SOCKET_ERROR)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());
}

const std::string keilo_server::process_message(std::string message, keilo_database** database, keilo_table** table)
{
	std::stringstream result;

	if (message.find(CREATE) != std::string::npos) {
		if (auto pos = message.find(DATABASE); pos != std::string::npos)
			result << create_database(message, pos + DATABASE.length());
		else if (auto pos = message.find(TABLE); pos != std::string::npos)
			result << create_table(message, pos + TABLE.length(), database);
		else
			result << "Unknown command.";
	}	
	else if (message.find(SELECT) != std::string::npos) {
		if (auto pos = message.find(DATABASE); pos != std::string::npos)
			result << select_database(message, pos + DATABASE.length(), database);
		else if (auto pos = message.find(TABLE); pos != std::string::npos)
			result << select_table(message, pos + TABLE.length(), database, table);
		else if (auto pos = message.find(RECORD); pos != std::string::npos)
			result << select_record(message, pos + RECORD.length(), table);
		else
			result << "Unknown command.";
	}
	else if (auto pos = message.find(JOIN); pos != std::string::npos) {
		result << join_table(message, pos + JOIN.length(),database , table);
	}
	else if (auto pos = message.find(INSERT); pos != std::string::npos) {
		result << insert_record(message, pos + INSERT.length(),table);
	}
	else if (auto pos = message.find(UPDATE); pos != std::string::npos) {
		result << update_record(message, pos + UPDATE.length(),table);
	}
	else if (auto pos = message.find(REMOVE); pos != std::string::npos) {
		result << remove_record(message, pos + REMOVE.length(),table);
	}
	else if (auto pos = message.find(DROP); pos != std::string::npos) {
		result << drop_table(message, pos + DROP.length(),database, table);
	}
	else if (auto pos = message.find(IMPORT_FILE); pos != std::string::npos) {
		result << import_database(message, pos + IMPORT_FILE.length());
	}
	else if (auto pos = message.find(EXPORT_FILE); pos != std::string::npos) {
		result << export_database(message, pos + EXPORT_FILE.length(),database);
	}
	/*
	else if (auto pos = message.find(CLEAR); pos != std::string::npos) {
		system("cls");
	}
	*/
	else
		result << "Unknown command.";

	return result.str();
}

void keilo_server::disconnect_client(client& _client)
{
	auto selected_client = m_clients.end();

	for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
		if (it->addr.sin_addr.S_un.S_addr == _client.addr.sin_addr.S_un.S_addr &&
			it->addr.sin_port == _client.addr.sin_port) 
		{
			selected_client = it;
			break;
		}
	}

	if (selected_client == m_clients.end())
		throw std::exception("[disconnect_client] Could not find client.");

	std::string addr = inet_ntoa(selected_client->addr.sin_addr);
	push_output(("[" + addr + ":" + std::to_string(selected_client->addr.sin_port) + "] disconnected.").c_str());

	closesocket(selected_client->sock);
	m_clients.erase(selected_client);
}

std::string keilo_server::create_database(std::string message, size_t pos)
{
	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream name;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			name << message[pos++];

	return m_application->create_database(name.str());
}

std::string keilo_server::select_database(std::string message, size_t pos, keilo_database** database)
{
	if (pos >= message.length())
		return "Syntax error.";
	
	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream name;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			name << message[pos++];

	std::stringstream returns;

	if (*database = m_application->select_database(name.str()); database)
		returns << "Successfully selected database that was named \"" << name.str() << "\".";
	else 
		returns << "Database that was named \"" << name.str() << "\" does not exist in server";

	return returns.str();
}

std::string keilo_server::export_database(std::string message, size_t pos, keilo_database** database)
{
	if (!*database)
		return "Please select database";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream file_name;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			file_name << message[pos++];

	if (file_name.str().find(".json") == std::string::npos)
		return "File name has to include extensions. (this program only support *.json files)";

	return  m_application->export_database((*database)->get_name(), file_name.str());
}

std::string keilo_server::import_database(std::string message, size_t pos)
{
	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream file_name;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			file_name << message[pos++];
	return import_file(file_name.str(), false);
}

std::string keilo_server::create_table(std::string message, size_t pos, keilo_database** database)
{
	if (!*database)
		return "Please select database.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream name;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			name << message[pos++];

	return (*database)->create_table(name.str());
}

std::string keilo_server::select_table(std::string message, size_t pos, keilo_database** database, keilo_table** table)
{
	if (!*database)
		return "Please select database.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream name;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			name << message[pos++];
		
	if (*table = (*database)->select_table(name.str()); *table)
		return "Successfully selected table that was named \"" + name.str() + "\".";
	else
		return "Table that was named \"" + name.str() + "\" does not exist in database \"" + (*database)->get_name() + "\".";
}

std::string keilo_server::join_table(std::string message, size_t pos, keilo_database** database, keilo_table** table)
{
	if (!*table)
		return "Please select table.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	if (message.find("_") != std::string::npos) {
		std::stringstream database_name;
		while (pos < message.length())
			if (message[pos] == '_')
				break;
			else
				database_name << message[pos++];
		pos += 1;
	
		auto _database = m_application->select_database(database_name.str());
		if (!_database)
			return "Database \"" + database_name.str() + "\" is not exist.";

		std::stringstream table_name;
		while (pos < message.length())
			if (message[pos] == ';')
				break;
			else
				table_name << message[pos++];

		auto _table = _database->select_table(table_name.str());
		
		if (!_table)
			return "Table \"" + table_name.str() + "\" is not exist in database \"" + _database->get_name() + "\".";

		(*database)->add_table((*table)->join(_table));
	}
	else {
		std::stringstream table_name;
		while (pos < message.length())
			if (message[pos] == ';')
				break;
			else
				table_name << message[pos++];

		auto _table = (*database)->select_table(table_name.str());

		if (!_table)
			return "Table \"" + table_name.str() + "\" is not exist in database \"" + (*database)->get_name() + "\".";

		(*database)->add_table((*table)->join(_table));
	}
	return "Successfully joined tables and added it to database \"" + (*database)->get_name() + "\".";
}

std::string keilo_server::drop_table(std::string message, size_t pos, keilo_database** database, keilo_table** table)
{
	if(!*database)
		return "Please select database.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream name;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			name << message[pos++];
	if(*table)
		if (name.str() == (*table)->get_name())
			table = nullptr;

	return (*database)->drop_table(name.str());
}

std::string keilo_server::select_record(std::string message, size_t pos, keilo_table** table)
{
	if (!*table)
		return "Please select table.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream identifier;
	while (pos < message.length()) 
		if (message[pos] == ':')
			break;
		else
			identifier << message[pos++];
	pos += 1;

	std::stringstream selected_record;
	const std::list<keilo_record> records = (*table)->get_records();
	if (identifier.str() == "all") {
		for (auto record = records.cbegin(); record != records.cend();) {
			selected_record << '{' << std::endl;
			for (auto instance = record->cbegin(); instance != record->cend();) {
				selected_record << '\t' << instance->first << ':' << instance->second;
				if (++instance; instance != record->cend())
					selected_record << ',';
				selected_record << std::endl;
			}
			selected_record << '}';
			if (++record; record != records.cend())
				selected_record << ',';
			selected_record << std::endl;
		}
	}
	else {
		std::stringstream value;
		while (pos < message.length())
			if (message[pos] == ';')
				break;
			else
				value << message[pos++];

		auto record = (*table)->select_record(keilo_instance{ identifier.str(), value.str() });

		if (record.size() > 0) {
			selected_record << '{' << std::endl;
			for (auto instance = record.cbegin(); instance != record.cend();) {
				selected_record << '\t' << instance->first << ':' << instance->second;
				if (++instance; instance != record.cend())
					selected_record << ',';
				selected_record << std::endl;
			}
			selected_record << '}' << std::endl;
		}
		else
			selected_record << "There is no record that has \"" << value.str() << "\" as " << identifier.str() << " in table " << (*table)->get_name() << "." << std::endl;
	}

	return selected_record.str();
}

std::string keilo_server::insert_record(std::string message, size_t pos, keilo_table** table)
{
	if (!*table)
		return "Please select table";

	keilo_record record;

	while (pos < message.length()) {
		std::stringstream identifier;
		while (pos < message.length())
			if (message[pos] == ':')
				break;
			else
				identifier << message[pos++];
		pos += 1;

		std::stringstream value;
		while (pos < message.length())
			if (message[pos] == ',' || message[pos] == ';')
				break;
			else
				value << message[pos++];

		record.push_back(keilo_instance{ identifier.str(), value.str() });

		while (pos < message.length())
			if (message[pos] != ' ' && message[pos] != ',' && message[pos] != ';')
				break;
			else
				pos++;
	}

	return (*table)->insert_record(record);
}

std::string keilo_server::update_record(std::string message, size_t pos, keilo_table** table)
{
	if (!*table)
		return "Please select table.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream condition_identifier;
	while (pos < message.length())
		if (message[pos] == ':')
			break;
		else
			condition_identifier << message[pos++];
	pos += 1;

	std::stringstream condition_value;
	while (pos < message.length())
		if (message[pos] == ' ')
			break;
		else
			condition_value << message[pos++];
	pos += 1;

	std::stringstream new_identifier;
	while (pos < message.length())
		if (message[pos] == ':')
			break;
		else
			new_identifier << message[pos++];
	pos += 1;

	std::stringstream new_value;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			new_value << message[pos++];
	pos += 1;

	return (*table)->update_record(keilo_instance{ condition_identifier.str(), condition_value.str() }, keilo_instance{ new_identifier.str(), new_value.str() });
}

std::string keilo_server::remove_record(std::string message, size_t pos, keilo_table** table)
{
	if (!*table)
		return "Please select table.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
		if (message[pos] != ' ')
			break;
		else
			pos++;

	std::stringstream identifier;
	while (pos < message.length())
		if (message[pos] == ':')
			break;
		else
			identifier << message[pos++];
	pos += 1;

	std::stringstream value;
	while (pos < message.length())
		if (message[pos] == ';')
			break;
		else
			value << message[pos++];

	return (*table)->remove_record(keilo_instance{ identifier.str(), value.str() });
}
