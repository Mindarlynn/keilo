#include "keilo_server.hpp"
#include "keilo_core.hpp"

#include <utility>
#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <iomanip>
#include <winsock2.h>

#pragma warning(disable:4996)

keilo_server::keilo_server(const int port) :
	print_thread_(std::thread(&keilo_server::print_output, this)),
	accept_thread_(std::thread(&keilo_server::accept_client, this)),
	port_(port),
	clients_(std::list<client>()),
	client_processes_(std::list<std::thread>()),
	application_(new keilo_application())
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa_) != 0)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());

	if (socket_ = socket(AF_INET, SOCK_STREAM, 0); socket_ == INVALID_SOCKET)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());

	ZeroMemory(&address_, sizeof SOCKADDR_IN);

	address_.sin_family = AF_INET;
	address_.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	address_.sin_port = htons(port_);

	if (bind(socket_, reinterpret_cast<SOCKADDR*>(&address_), sizeof address_) == SOCKET_ERROR)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());

	std::cout << "Successfully initialized winsock2." << std::endl;
}

keilo_server::~keilo_server()
{
	if (is_running_.load())
		is_running_ = false;
	if (accept_thread_.joinable())
	{
		const auto tmp = socket(AF_INET, SOCK_STREAM, 0);
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(6060);
		connect(tmp, reinterpret_cast<SOCKADDR*>(&addr), sizeof addr);
		closesocket(tmp);
		accept_thread_.join();
	}
	if (print_thread_.joinable())
		print_thread_.join();
	for (auto& processes : client_processes_)
		if (processes.joinable())
			processes.join();
	for (auto& client : clients_)
		closesocket(client.socket);
	closesocket(socket_);
}

void keilo_server::run()
{
	if (listen(socket_, 10) == SOCKET_ERROR)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());
	is_running_ = true;
	push_output("Successfully started server.");
	while (is_running_.load());
}

void keilo_server::run_local()
{
	keilo_database* selected_database = nullptr;
	keilo_table* selected_table = nullptr;
	std::string input;
	while (is_running_.load())
	{
		while (is_printing_.load());
		std::stringstream select_status;
		if (selected_database)
		{
			select_status << "[" << selected_database->get_name();
			if (selected_table)
				select_status << "_" << selected_table->get_name();
			select_status << "]> ";
		}
		else
			select_status << "[none]> ";

		std::cout << select_status.str();

		if (getline(std::cin, input); input == "exit")
		{
			is_running_ = false;
			break;
		}

		push_output(process_message(input, &selected_database, &selected_table));
	}
}

std::string keilo_server::import_file(const std::string file_name, const bool ps)
{
	auto output = application_->import_file(file_name);
	if (ps)
	{
		push_output(output);
	}
	return output;
}

void keilo_server::print_output()
{
	while (!is_running_.load());
	while (is_running_.load())
	{
		output_mutex_.lock();
		while (outputs_.size() > 0)
		{
			is_printing_ = true;
			std::cout << outputs_.front() << std::endl;
			outputs_.pop();
		}
		output_mutex_.unlock();
		if (is_printing_.load())
			is_printing_ = false;
	}
}

void keilo_server::push_output(const std::string message)
{
	output_mutex_.lock();
	outputs_.push(message);
	output_mutex_.unlock();
}

void keilo_server::accept_client()
{
	while (!is_running_.load());
	while (is_running_.load())
	{
		client _client;
		int addrlen = sizeof _client.address;

		_client.socket = accept(socket_, reinterpret_cast<SOCKADDR*>(&_client.address), &addrlen);

		const std::string addr = inet_ntoa(_client.address.sin_addr);
		push_output("[" + addr + ":" + std::to_string(_client.address.sin_port) + "] connected.");

		clients_.push_back(_client);

		client_processes_.push_back(std::thread([&]()
		{
			auto client_addr = _client.address;
			keilo_database* selected_database = nullptr;
			keilo_table* selected_table = nullptr;

			while (is_running_.load())
			{
				auto found = clients_.end();
				for (auto it = clients_.begin(); it != clients_.end(); ++it)
				{
					if (it->address.sin_addr.S_un.S_addr == client_addr.sin_addr.S_un.S_addr &&
						it->address.sin_port == client_addr.sin_port)
					{
						found = it;
						break;
					}
				}
				if (found == clients_.end())
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
	const auto received = recv(_client.socket, read_data, 1024, 0);
	if (received == 0)
	{
		disconnect_client(_client);
		return;
	}

	read_data[received] = 0;

	const std::string addr = inet_ntoa(_client.address.sin_addr);
	push_output("[" + addr + ":" + std::to_string(_client.address.sin_port) + "] " + std::string(read_data));

	auto processed_message = process_message(read_data, database, table);
	if (send(_client.socket, processed_message.c_str(), processed_message.length(), 0) == SOCKET_ERROR)
		throw std::exception(std::to_string(WSAGetLastError()).c_str());
}

std::string keilo_server::process_message(std::string message, keilo_database** database, keilo_table** table)
{
	std::stringstream result;
	auto pos = 0;

	if (message.find(create_) != std::string::npos)
	{
		if (pos = message.find(database_); pos != std::string::npos)
			result << create_database(message, pos + database_.length());
		else if (pos = message.find(table_); pos != std::string::npos)
			result << create_table(message, pos + table_.length(), database);
		else
			result << "Unknown command.";
	}
	else if (message.find(select_) != std::string::npos)
	{
		if (pos = message.find(database_); pos != std::string::npos)
			result << select_database(message, pos + database_.length(), database);
		else if (pos = message.find(table_); pos != std::string::npos)
			result << select_table(message, pos + table_.length(), database, table);
		else if (pos = message.find(record_); pos != std::string::npos)
			result << select_record(message, pos + record_.length(), table);
		else
			result << "Unknown command.";
	}
	else if (pos = message.find(join_); pos != std::string::npos)
	{
		result << join_table(message, pos + join_.length(), database, table);
	}
	else if (pos = message.find(insert_); pos != std::string::npos)
	{
		result << insert_record(message, pos + insert_.length(), table);
	}
	else if (pos = message.find(update_); pos != std::string::npos)
	{
		result << update_record(message, pos + update_.length(), table);
	}
	else if (pos = message.find(remove_); pos != std::string::npos)
	{
		result << remove_record(message, pos + remove_.length(), table);
	}
	else if (pos = message.find(drop_); pos != std::string::npos)
	{
		result << drop_table(message, pos + drop_.length(), database, table);
	}
	else if (pos = message.find(import_file_); pos != std::string::npos)
	{
		result << import_database(message, pos + import_file_.length());
	}
	else if (pos = message.find(export_file_); pos != std::string::npos)
	{
		result << export_database(message, pos + export_file_.length(), database);
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

void keilo_server::disconnect_client(client& client)
{
	auto selected_client = clients_.end();

	for (auto it = clients_.begin(); it != clients_.end(); ++it)
	{
		if (it->address.sin_addr.S_un.S_addr == client.address.sin_addr.S_un.S_addr &&
			it->address.sin_port == client.address.sin_port)
		{
			selected_client = it;
			break;
		}
	}

	if (selected_client == clients_.end())
		throw std::exception("[disconnect_client] Could not find client.");

	const std::string addr = inet_ntoa(selected_client->address.sin_addr);
	push_output(("[" + addr + ":" + std::to_string(selected_client->address.sin_port) + "] disconnected.").c_str());

	closesocket(selected_client->socket);
	clients_.erase(selected_client);
}

std::string keilo_server::create_database(std::string message, size_t pos) const
{
	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		name << message[pos++];
	}

	return application_->create_database(name.str());
}

std::string keilo_server::select_database(std::string message, size_t pos, keilo_database** database) const
{
	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		name << message[pos++];
	}

	std::stringstream returns;

	if (*database = application_->select_database(name.str()); database)
		returns << "Successfully selected database that was named \"" << name.str() << "\".";
	else
		returns << "Database that was named \"" << name.str() << "\" does not exist in server";

	return returns.str();
}

std::string keilo_server::export_database(std::string message, size_t pos, keilo_database** database) const
{
	if (!*database)
		return "Please select database";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream file_name;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		file_name << message[pos++];
	}

	if (file_name.str().find(".json") == std::string::npos)
		return "File name has to include extensions. (this program only support *.json files)";

	return application_->export_database((*database)->get_name(), file_name.str());
}

std::string keilo_server::import_database(std::string message, size_t pos)
{
	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream file_name;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		file_name << message[pos++];
	}
	return import_file(file_name.str(), false);
}

std::string keilo_server::create_table(std::string message, size_t pos, keilo_database** database)
{
	if (!*database)
		return "Please select database.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		name << message[pos++];
	}

	return (*database)->create_table(name.str());
}

std::string keilo_server::select_table(std::string message, size_t pos, keilo_database** database,
                                       keilo_table** table)
{
	if (!*database)
		return "Please select database.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		name << message[pos++];
	}

	if (*table = (*database)->select_table(name.str()); *table)
		return "Successfully selected table that was named \"" + name.str() + "\".";
	return "Table that was named \"" + name.str() + "\" does not exist in database \"" + (*database)->get_name() + "\".";
}

std::string keilo_server::join_table(std::string message, size_t pos, keilo_database** database,
                                     keilo_table** table) const
{
	if (!*table)
		return "Please select table.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	if (message.find("_") != std::string::npos)
	{
		std::stringstream database_name;
		while (pos < message.length())
		{
			if (message[pos] == '_')
				break;
			database_name << message[pos++];
		}
		pos += 1;

		auto selected_database = application_->select_database(database_name.str());
		if (!selected_database)
			return "Database \"" + database_name.str() + "\" is not exist.";

		std::stringstream table_name;
		while (pos < message.length())
		{
			if (message[pos] == ';')
				break;
			table_name << message[pos++];
		}

		const auto selected_table = selected_database->select_table(table_name.str());

		if (!selected_table)
			return "Table \"" + table_name.str() + "\" is not exist in database \"" + selected_database->get_name() + "\".";

		auto joined_table = (*table)->join(selected_table);
		(*database)->add_table(joined_table);
	}
	else
	{
		std::stringstream table_name;
		while (pos < message.length())
		{
			if (message[pos] == ';')
				break;
			table_name << message[pos++];
		}

		const auto selected_table = (*database)->select_table(table_name.str());

		if (!selected_table)
			return "Table \"" + table_name.str() + "\" is not exist in database \"" + (*database)->get_name() + "\".";

		auto joined_table = (*table)->join(selected_table);
		(*database)->add_table(joined_table);
	}
	return "Successfully joined tables and added it to database \"" + (*database)->get_name() + "\".";
}

std::string keilo_server::drop_table(std::string message, size_t pos, keilo_database** database, keilo_table** table)
{
	if (!*database)
		return "Please select database.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		name << message[pos++];
	}
	if (*table)
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
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream identifier;
	while (pos < message.length())
	{
		if (message[pos] == ':')
			break;
		if (message[pos] == ' ')
			while (message[pos] != ':')
			{
				pos++;
			}
		else
			identifier << message[pos++];
	}

	while (++pos < message.length())
		if (message[pos] != ' ')
			break;

	std::stringstream selected_record;
	const auto records = (*table)->get_records();
	if (identifier.str() == "all")
	{
		for (auto record = records.cbegin(); record != records.cend();)
		{
			selected_record << '{' << std::endl;
			for (auto instance = record->cbegin(); instance != record->cend();)
			{
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
	else
	{
		std::stringstream value;
		while (pos < message.length())
		{
			if (message[pos] == ';')
				break;
			value << message[pos++];
		}

		auto record = (*table)->select_record(keilo_instance{identifier.str(), value.str()});

		if (record.size() > 0)
		{
			selected_record << '{' << std::endl;
			for (auto instance = record.cbegin(); instance != record.cend();)
			{
				selected_record << '\t' << instance->first << ':' << instance->second;
				if (++instance; instance != record.cend())
					selected_record << ',';
				selected_record << std::endl;
			}
			selected_record << '}' << std::endl;
		}
		else
			selected_record << "There is no record that has \"" << value.str() << "\" as " << identifier.str() << " in table "
				<< (*table)->get_name() << "." << std::endl;
	}

	return selected_record.str();
}

std::string keilo_server::insert_record(std::string message, size_t pos, keilo_table** table)
{
	if (!*table)
		return "Please select table";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		++pos;
	}

	keilo_record record;

	while (pos < message.length())
	{
		std::stringstream identifier;
		while (pos < message.length())
		{
			if (message[pos] == ':')
				break;
			identifier << message[pos++];
		}

		++pos;

		while (pos < message.length())
		{
			if (message[pos] != ' ')
				break;
			++pos;
		}

		std::stringstream value;
		while (pos < message.length())
		{
			if (message[pos] == ',' || message[pos] == ';')
				break;
			value << message[pos++];
		}

		record.push_back(keilo_instance{identifier.str(), value.str()});

		while (pos < message.length())
		{
			if (message[pos] != ' ' && message[pos] != ',' && message[pos] != ';')
				break;
			pos++;
		}
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
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream condition_identifier;
	while (pos < message.length())
	{
		if (message[pos] == ':')
			break;
		condition_identifier << message[pos++];
	}
	pos += 1;

	std::stringstream condition_value;
	while (pos < message.length())
	{
		if (message[pos] == ' ')
			break;
		condition_value << message[pos++];
	}
	pos += 1;

	std::stringstream new_identifier;
	while (pos < message.length())
	{
		if (message[pos] == ':')
			break;
		new_identifier << message[pos++];
	}
	pos += 1;

	std::stringstream new_value;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		new_value << message[pos++];
	}

	return (*table)->update_record(keilo_instance{condition_identifier.str(), condition_value.str()},
	                               keilo_instance{new_identifier.str(), new_value.str()});
}

std::string keilo_server::remove_record(std::string message, size_t pos, keilo_table** table)
{
	if (!*table)
		return "Please select table.";

	if (pos >= message.length())
		return "Syntax error.";

	while (pos < message.length())
	{
		if (message[pos] != ' ')
			break;
		pos++;
	}

	std::stringstream identifier;
	while (pos < message.length())
	{
		if (message[pos] == ':')
			break;
		identifier << message[pos++];
	}
	pos += 1;

	std::stringstream value;
	while (pos < message.length())
	{
		if (message[pos] == ';')
			break;
		value << message[pos++];
	}

	return (*table)->remove_record(keilo_instance{identifier.str(), value.str()});
}
