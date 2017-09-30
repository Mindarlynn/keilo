#include "keilo_server.hpp"

#include <utility>
#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <exception>

#include <boost/asio.hpp>


namespace asio = boost::asio;
namespace ip = asio::ip;



keilo_server::keilo_server(int port) : 
	m_application(new keilo_application()), 
	m_clients(std::list < ip::tcp::socket > ()), 
	m_client_processes(std::list<std::thread>()), 
	m_port(port), 
	m_acceptor(m_io_service, ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), m_port)),
	print_thread(std::thread(&keilo_server::print_output, this)),
	accept_thread(std::thread(&keilo_server::accept_client, this))
{
}

keilo_server::~keilo_server()
{
	if (running.load())
		running = false;
	if (accept_thread.joinable())
		accept_thread.join();
	if (print_thread.joinable())
		print_thread.join();
	for (auto& client : m_clients)
		client.close();
	m_acceptor.close();
}

void keilo_server::run()
{
	m_acceptor.listen(5);
	run_local();
}

void keilo_server::run_local()
{
	running = true;
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
		
		push_output(process_message(input));
	}
}

std::string keilo_server::import_file(std::string file_name, bool ps)
{
	selected_database = nullptr;
	selected_table = nullptr;
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
	while (running.load()) {
		ip::tcp::socket _client(m_io_service);
		m_acceptor.accept(_client);

		m_clients.push_back(std::move(_client));

		m_client_processes.push_back(std::thread([&]() {
			auto found = false;
			do {
				process_client(_client);
				for (const auto& client : m_clients) {
					if (client.remote_endpoint().address().to_string() == _client.remote_endpoint().address().to_string() &&
						client.remote_endpoint().port() == _client.remote_endpoint().port())
					{
						found = true;
						break;
					}
				}
			} while (found);
		}));
	}
}

void keilo_server::process_client(ip::tcp::socket& client)
{
	asio::streambuf read_buffer;
	asio::read(client, read_buffer);

	std::string read_data = asio::buffer_cast<const char*>(read_buffer.data());
	read_data.erase(--read_data.end());

	asio::write(client, asio::buffer(process_message(read_data.c_str())));
}

const std::string keilo_server::process_message(std::string message)
{
	
	std::stringstream result;

	if (message.find(CREATE) != std::string::npos) {
		if (auto pos = message.find(DATABASE); pos != std::string::npos)
			result << create_database(message, pos + DATABASE.length());
		else if (auto pos = message.find(TABLE); pos != std::string::npos)
			result << create_table(message, pos + TABLE.length());
		else
			result << "Unknown command.";
	}	
	else if (message.find(SELECT) != std::string::npos) {
		if (auto pos = message.find(DATABASE); pos != std::string::npos)
			result << select_database(message, pos + DATABASE.length());
		else if (auto pos = message.find(TABLE); pos != std::string::npos)
			result << select_table(message, pos + TABLE.length());
		else if (auto pos = message.find(RECORD); pos != std::string::npos)
			result << select_record(message, pos + RECORD.length());
		else
			result << "Unknown command.";
	}
	else if (auto pos = message.find(JOIN); pos != std::string::npos) {
		result << join_table(message, pos + JOIN.length());
	}
	else if (auto pos = message.find(INSERT); pos != std::string::npos) {
		result << insert_record(message, pos + INSERT.length());
	}
	else if (auto pos = message.find(UPDATE); pos != std::string::npos) {
		result << update_record(message, pos + UPDATE.length());
	}
	else if (auto pos = message.find(REMOVE); pos != std::string::npos) {
		result << remove_record(message, pos + REMOVE.length());
	}
	else if (auto pos = message.find(DROP); pos != std::string::npos) {
		result << drop_table(message, pos + DROP.length());
	}
	else if (auto pos = message.find(IMPORT_FILE); pos != std::string::npos) {
		result << import_database(message, pos + IMPORT_FILE.length());
	}
	else if (auto pos = message.find(EXPORT_FILE); pos != std::string::npos) {
		result << export_database(message, pos + EXPORT_FILE.length());
	}
	else if (auto pos = message.find(CLEAR); pos != std::string::npos) {
		system("cls");
	}
	else
		result << "Unknown command.";

	return result.str();
}

void keilo_server::disconnect_client(ip::tcp::socket _client)
{
	auto selected_client = m_clients.end();

	for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
		if (it->remote_endpoint().address().to_string() == _client.remote_endpoint().address().to_string() && 
			it->remote_endpoint().port() == _client.remote_endpoint().port()) 
		{
			selected_client = it;
			break;
		}
	}

	if (selected_client == m_clients.end())
		throw std::exception("[disconnect_client] Could not find client.");

	push_output(("[disconnect_client] (" + selected_client->remote_endpoint().address().to_string() + ") disconnected.").c_str());
	selected_client->close();
	m_clients.erase(selected_client);

	for (auto& process : m_client_processes) {
		if (process.joinable())
			process.join();
	}
}

void keilo_server::print_output()
{
	while (!running.load());
	while (running.load()) {
		m_output_mutex.lock();
		if (m_outputs.size() > 0) {
			std::cout << m_outputs.front() << std::endl;
			m_outputs.pop();
		}
		m_output_mutex.unlock();
	}
}

void keilo_server::accept_client()
{
	while (running.load()) {		
		ip::tcp::socket _client(m_io_service);
		m_acceptor.accept(_client);

		m_clients.push_back(std::move(_client));

		m_client_processes.push_back(std::thread([&]() {
			auto found = false;
			do {
				process_client(_client);
				for (const auto& client : m_clients) {
					if (client.remote_endpoint().address().to_string() == _client.remote_endpoint().address().to_string() &&
						client.remote_endpoint().port() == _client.remote_endpoint().port())
					{
						found = true;
						break;
					}
				}
			} while (found);
		}));
	}
}

int keilo_server::get_message_type(const std::string message)
{
	auto type = 0;
	if (message == "create")
		type = create;
	else if (message == "select")
		type = select;
	else if (message == "insert")
		type = insert;
	else if (message == "update")
		type = update;
	else if (message == "remove")
		type = remove;
	else if (message == "drop")
		type = drop;
	else if (message == "export")
		type = exp_file;
	else if (message == "import")
		type = imp_file;

	return type;
}

int keilo_server::get_secondary_type(const std::string message)
{
	auto type = 0;
	if (message == "table")
		type = table;
	else if (message == "database")
		type = database;
	else if (message == "record")
		type = record;
	return type;
}

void keilo_server::push_output(const std::string message)
{
	m_output_mutex.lock();
	m_outputs.push(message);
	m_output_mutex.unlock();
}

std::string keilo_server::create_database(std::string message, size_t pos)
{
	if (pos >= message.length())
		return "Syntax error";
	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	return m_application->create_database(name.str());
}

std::string keilo_server::select_database(std::string message, size_t pos)
{
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	if (selected_database = m_application->select_database(name.str()); selected_database)
		return "Successfully selected database that was named \"" + name.str() + "\".";
	else
		return "Database that was named \"" + name.str() + "\" does not exist in server";
}

std::string keilo_server::export_database(std::string message, size_t pos)
{
	if (!selected_database)
		return "Please select database";
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream file_name;
	while (message[pos] != ';' && pos < message.length())
		file_name << message[pos++];
	file_name << ".klo";

	return  m_application->export_database(selected_database->get_name(), file_name.str());
}

std::string keilo_server::import_database(std::string message, size_t pos)
{
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream file_name;
	while (message[pos] != ';' && pos < message.length())
		file_name << message[pos++];
	return import_file(file_name.str(), false);
}

std::string keilo_server::create_table(std::string message, size_t pos)
{
	if (!selected_database)
		return "Please select database";
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	return selected_database->create_table(name.str());
}

std::string keilo_server::select_table(std::string message, size_t pos)
{
	if (!selected_database)
		return "Please select database";
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	if (selected_table = selected_database->select_table(name.str()); selected_table)
		return "Successfully selected table that was named \"" + name.str() + "\".";
	else
		return "Table that was named \"" + name.str() + "\" does not exist in database \"" + selected_database->get_name() + "\".";
}

std::string keilo_server::drop_table(std::string message, size_t pos)
{
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream name;
	while (message[pos] != ';' && pos < message.length())
		name << message[pos++];

	return selected_database->drop_table(name.str());
}

std::string keilo_server::select_record(std::string message, size_t pos)
{
	if (!selected_table)
		return "Please select table.";
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream identifier;
	while (message[pos] != ':' && pos < message.length())
		identifier << message[pos++];
	pos += 1;
	std::stringstream value;
	while (message[pos] != ';' && pos < message.length())
		value << message[pos++];

	auto record = selected_table->select_record(keilo_instance{ identifier.str(), value.str() });
	std::stringstream selected_record;

	if (record.size() > 0) {
		selected_record << "(" << std::endl;
		for (const auto& instance : record)
			selected_record << instance.first << ":" << instance.second << ";" << std::endl;
		selected_record << ")" << std::endl;
	}
	else
		selected_record << "There is no record that has " << value.str() << " as " << identifier.str() << "in table " << selected_table->get_name() << "." << std::endl;

	return selected_record.str();
}

std::string keilo_server::insert_record(std::string message, size_t pos)
{
	// 수정 필요
	/*
	while (message[pos - 1] != '(')
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
	}*/

	if (!selected_table)
		return "Please select table";

	keilo_record record;

	while (pos < message.length()) {
		std::stringstream identifier;
		while (message[pos] != ':' && pos < message.length())
			identifier << message[pos++];
		pos += 1;

		std::stringstream value;
		while ((message[pos] != ',' && message[pos] != ';') && pos < message.length())
			value << message[pos++];

		record.push_back(keilo_instance{ identifier.str(), value.str() });
		 
		while (message[pos - 1] != ' ' && message[pos - 1] != ','  && message[pos - 1] != ';' && pos < message.length())
			pos++;
	}

	return selected_table->insert(record);
}

std::string keilo_server::update_record(std::string message, size_t pos)
{
	if (!selected_table)
		return "Please select table";
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream condition_identifier;
	while (message[pos] != ':' && pos < message.length())
		condition_identifier << message[pos++];
	pos += 1;

	std::stringstream condition_value;
	while (message[pos] != ' '&& pos < message.length())
		condition_value << message[pos++];
	pos += 1;

	std::stringstream new_identifier;
	while (message[pos] != ':'&& pos < message.length())
		new_identifier << message[pos++];
	pos += 1;

	std::stringstream new_value;
	while (message[pos] != ';'&& pos < message.length())
		new_value << message[pos++];
	pos += 1;

	return selected_table->update(keilo_instance{ condition_identifier.str(), condition_value.str() }, keilo_instance{ new_identifier.str(), new_value.str() });
}

std::string keilo_server::remove_record(std::string message, size_t pos)
{
	if (!selected_table)
		return "Please select table";
	if (pos >= message.length())
		return "Syntax error";

	std::stringstream identifier;
	while (message[pos] != ':' && pos < message.length())
		identifier << message[pos++];
	pos += 1;
	std::stringstream value;
	while (message[pos] != ';' && pos < message.length())
		value << message[pos++];

	return selected_table->remove(keilo_instance{ identifier.str(), value.str() });
}
