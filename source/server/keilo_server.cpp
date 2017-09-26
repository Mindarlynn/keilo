#include "keilo_server.hpp"

#include <utility>
#include <thread>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/asio.hpp>


namespace asio = boost::asio;
namespace ip = asio::ip;



keilo_server::keilo_server(int port) : m_application(new keilo_application()), m_clients(std::list < ip::tcp::socket > ()), m_client_processes(std::list<std::thread>()), m_port(port), m_acceptor(m_io_service, ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), m_port)), print_thread(std::thread(&keilo_server::print_output, this))
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
		client.close();
	}
	m_acceptor.close();
}

void keilo_server::run()
{
	initialize();
}

void keilo_server::import_file(std::string file_name)
{
	m_application->import_file(file_name);
}

void keilo_server::initialize()
{
	running = true;

	m_acceptor.listen(5);

	accept_thread = new std::thread([&]() {
		while (running.load()) {
			
			// accept client

			ip::tcp::socket _client(m_io_service);
			m_acceptor.accept(_client);

			m_clients.push_back(std::move(_client));

			m_client_processes.push_back(std::thread([&]() {
				bool found = false;
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
	});
}

void keilo_server::process_client(ip::tcp::socket& client)
{
	// receive request and send result
	asio::streambuf read_buffer;
	asio::read(client, read_buffer);

	std::string read_data = asio::buffer_cast<const char*>(read_buffer.data());
	read_data.erase(--read_data.end());

	asio::write(client, asio::buffer(process_message(read_data.c_str())));
}

const std::string keilo_server::process_message(const char * message)
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

	m_outputs.push("[disconnect_client] (" + selected_client->remote_endpoint().address().to_string() + ") disconnected.");
	selected_client->close();
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

	if (selected_database = m_application->select_database(name.str()); selected_database)
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

	if (selected_table = selected_database->select_table(name.str()); selected_table)
		return "Successfully selected table that was named \"" + name.str() + "\".";
	else
		return "Table that was named \"" + name.str() + "\" does not exist in database \"" + selected_database->get_name() + "\".";

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
