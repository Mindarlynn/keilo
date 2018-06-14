#include "keilo_server.hpp"
#include "keilo_core.hpp"

#include <stdio.h>

#include <utility>
#include <memory>
#include <thread>
#include <sstream>
#include <string>
#include <exception>

#include <string_process.h>

#define COLON ':'
#define COMMA ','
#define SEMICOLON ';'
#define SPACE ' '
#define UNDER_BAR '_'

#pragma warning(suppress : 4996)
#pragma comment(lib, "tcp_socket")

keilo_server::keilo_server(const int& port)
	: clients(),
	  res_wsa(WSAStartup(MAKEWORD(2, 2), &wsa)),
	  client_processes(),
	  application(new keilo_application()) {
	if (res_wsa != 0) throw std::exception("error with wsa startup.");

	processer = new string_process();
	socket = new tcp_socket(INADDR_ANY, 6060, &processer);

	printf((application->import_file("/user/user.json") + '\n').c_str());

	user_database = application->select_database("user_database");
}

keilo_server::~keilo_server() {
	if (is_running.load()) is_running = false;
	for (auto it = login_threads.begin(); it != login_threads.end(); ++it) {
		if (it->joinable()) it->join();
		login_threads.erase(it);
	}
	if (accept_thread.joinable()) {
		socket->stop();
		delete socket;
		accept_thread.join();
	}
	for (auto it = client_processes.begin(); it != client_processes.end(); ++it) {
		if (it->joinable()) it->join();
		client_processes.erase(it);
	}
	for (auto& client : clients) client->stop();
	for (auto it = clients.begin(); it != clients.end(); ++it) {
		(*it)->stop();
		clients.erase(it);
	}
	WSACleanup();
}

void keilo_server::run() {
	is_running = socket->start();
	accept_thread = std::thread([this]() {
		try {
			while (is_running.load()) {
				auto client = new tcp_socket(&processer);
				socket->accept_client(&client);

				login_threads.emplace_back([this, client]() {
					printf("[%s:%d] connected\n", client->get_ip().c_str(),
					       client->get_port());

					std::string user_info[] = {"ID : ", "Password : "};
					for (auto i = 0; i < 2; ++i) {
						if (!client->send(user_info[i])) {
							printf("[%s:%d] Disconnected\n", client->get_ip().c_str(),
							       client->get_port());
							break;
						}

						user_info[i] = client->recv();
					}

					bool has_user;

					for (const auto& record :
					     user_database->select_table("user")->get_records()) {
						auto has_id = false, has_pw = false;

						for (const auto& field : record) {
							if (field.first == "ID" && field.second == user_info[0])
								has_id = true;
							if (field.first == "Password" && field.second == user_info[1])
								has_pw = true;
							has_user = has_id & has_pw;
						}
						if (has_user) break;
					}
					if (has_user) {
						std::string message = "success";

						client->send(message);

						clients.emplace_back(client);

						client_processes.emplace_back([this, client]() {
							keilo_database* database = nullptr;
							keilo_table* table = nullptr;

							while (is_running.load()) {
								if (auto it = find_client(client); it == clients.end())
									break;
								else
									process_client(&*it, &database, &table);
							}
						});
					}
					else {
						const auto message = "User " + user_info[0] + "is not member of this server";

						client->send(message);
						client->stop();
						delete client;
					}
				});
			}
		}
		catch (std::exception& e) {
			is_running = false;
			printf("\nerror : %s\n", e.what());
		}
	});
	printf("Successfully started server.\n");
	accept_thread.join();
}

void keilo_server::import_file(const std::string& file_name) const {
	printf("%s\n", application->import_file(file_name).c_str());
}

void keilo_server::process_client(tcp_socket** const client,
                                  keilo_database** database,
                                  keilo_table** table) {
	auto received = (*client)->recv();

	printf("[%s:%d] %s\n", (*client)->get_ip().c_str(), (*client)->get_port(),
	       received.c_str());

	if (!(*client)->send(process_message(received, database, table))) {
		if (const auto it = find_client(*client); it != clients.end()) {
			delete *it;
			clients.erase(it);
		}
	}
}

std::string keilo_server::process_message(const std::string& message,
                                          keilo_database** database,
                                          keilo_table** table) const {
	std::string result;
	u_int pos;

	if (message.find(CREATE) != std::string::npos) {
		if (pos = message.find(DATABASE); pos != std::string::npos)
			result = create_database(message, pos + strlen(DATABASE));
		else if (pos = message.find(TABLE); pos != std::string::npos)
			result = create_table(message, pos + strlen(TABLE), database);
		else
			result = "Unknown command.";
	}
	else if (message.find(SELECT) != std::string::npos) {
		if (pos = message.find(DATABASE); pos != std::string::npos)
			result = select_database(message, pos + strlen(DATABASE), database);
		else if (pos = message.find(TABLE); pos != std::string::npos)
			result = select_table(message, pos + strlen(TABLE), database, table);
		else if (pos = message.find(RECORD); pos != std::string::npos)
			result = select_record(message, pos + strlen(RECORD), table);
		else
			result = "Unknown command.";
	}
	else if (pos = message.find(JOIN); pos != std::string::npos) {
		result = join_table(message, pos + strlen(JOIN), database, table);
	}
	else if (pos = message.find(INSERT); pos != std::string::npos) {
		result = insert_record(message, pos + strlen(INSERT), table);
	}
	else if (pos = message.find(UPDATE); pos != std::string::npos) {
		result = update_record(message, pos + strlen(UPDATE), table);
	}
	else if (pos = message.find(REMOVE); pos != std::string::npos) {
		result = remove_record(message, pos + strlen(REMOVE), table);
	}
	else if (pos = message.find(DROP); pos != std::string::npos) {
		result = drop_table(message, pos + strlen(DROP), database, table);
	}
	else if (pos = message.find(IMPORT_FILE); pos != std::string::npos) {
		result = import_database(message, pos + strlen(IMPORT_FILE));
	}
	else if (pos = message.find(EXPORT_FILE); pos != std::string::npos) {
		result = export_database(message, pos + strlen(EXPORT_FILE), database);
	}
	else
		result = "Unknown command.";

	return result;
}

std::list<tcp_socket*>::iterator keilo_server::find_client(
	tcp_socket* const client) {
	for (auto it = clients.begin(); it != clients.end(); ++it)
		if (*client == **it) return it;
	return clients.end();
}

std::string keilo_server::create_database(const std::string& message,
                                          u_int pos) const {
	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		name << message[pos++];
	}

	return application->create_database(name.str());
}

std::string keilo_server::select_database(
	const std::string& message, u_int pos,
	keilo_database** const database) const {
	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		name << message[pos++];
	}

	if (*database = application->select_database(name.str()); !*database)
		return "Database named \"" + name.str() + "does not exist in this server";

	return "Successfully selected database names " + name.str();
}

std::string keilo_server::export_database(
	const std::string& message, u_int pos,
	keilo_database** const database) const {
	if (!*database) return "Please select database";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream file_name;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		file_name << message[pos++];
	}

	if (file_name.str().find(".json") == std::string::npos)
		return "File name has to include extensions. (this program only support "
			"*.json files)";

	return application->export_database((*database)->get_name(), file_name.str());
}

std::string keilo_server::import_database(const std::string& message,
                                          u_int pos) const {
	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream file_name;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		file_name << message[pos++];
	}

	return application->import_file(file_name.str());
}

std::string keilo_server::create_table(const std::string& message, u_int pos,
                                       keilo_database** const database) {
	if (!*database) return "Please select database.";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		name << message[pos++];
	}

	return (*database)->create_table(name.str());
}

std::string keilo_server::select_table(const std::string& message, u_int pos,
                                       keilo_database** const database,
                                       keilo_table** const table) {
	if (!*database) return "Please select database.";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		name << message[pos++];
	}

	if (name.str() == "all") {
		std::stringstream tables;
		auto all_tables = (*database)->get_tables();
		for (auto it = all_tables.begin(); it != all_tables.end(); ++it) {
			tables << it->get_name();
			auto tmp = it;
			if (++tmp != all_tables.end()) tables << std::endl;
		}
		return tables.str();
	}

	if (*table = (*database)->select_table(name.str()); !*table)
		return "Table named " + name.str() + "does not exist in database " + (*database)->get_name();

	return "Successfully selected table named " + name.str();
}

std::string keilo_server::join_table(const std::string& message, u_int pos,
                                     keilo_database** const database,
                                     keilo_table** const table) const {
	if (!*table) return "Please select table.";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	if (message.find(UNDER_BAR) != std::string::npos) {
		std::stringstream database_name;
		while (pos < message.length()) {
			if (message[pos] == UNDER_BAR) break;
			database_name << message[pos++];
		}
		pos += 1;

		auto selected_database = application->select_database(database_name.str());
		if (!selected_database)
			return "Database " + database_name.str() + " is not exist.";

		std::stringstream table_name;
		while (pos < message.length()) {
			if (message[pos] == SEMICOLON) break;
			table_name << message[pos++];
		}

		const auto selected_table =
			selected_database->select_table(table_name.str());
		if (!selected_table)
			return "Table " + table_name.str() + 
			"is not exist in database " + selected_database->get_name();

		const auto joined_table = (*table)->join(selected_table);
		(*database)->add_table(joined_table);
	}
	else {
		std::stringstream table_name;
		while (pos < message.length()) {
			if (message[pos] == SEMICOLON) break;
			table_name << message[pos++];
		}

		const auto selected_table = (*database)->select_table(table_name.str());
		if (!selected_table)
			return "Table " + table_name.str() + 
			" is not exist in database " + (*database)->get_name();

		const auto joined_table = (*table)->join(selected_table);
		(*database)->add_table(joined_table);
	}

	return "Successfully joined tables and added it to database " + (*database)->get_name();
}

std::string keilo_server::drop_table(const std::string& message, u_int pos,
                                     keilo_database** const database,
                                     keilo_table** const table) {
	if (!*database) return "Please select database.";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream name;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		name << message[pos++];
	}
	if (*table)
		if (name.str() == (*table)->get_name()) *table = nullptr;

	return (*database)->drop_table(name.str());
}

std::string keilo_server::select_record(const std::string& message, u_int pos,
                                        keilo_table** const table) {
	if (!*table) return "Please select table.";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream key;
	while (pos < message.length()) {
		if (message[pos] == COLON || message[pos] == SEMICOLON) break;

		if (message[pos] == SPACE)
			while (message[pos] != COLON) pos++;
		else
			key << message[pos++];
	}

	while (++pos < message.length())
		if (message[pos] != SPACE) break;

	std::stringstream selected_record;
	if (key.str() == "all") {
		const auto records = (*table)->get_records();
		for (auto record = records.cbegin(); record != records.cend();) {
			selected_record << '{' << std::endl;
			for (auto field = record->cbegin(); field != record->cend();) {
				selected_record << '\t' << field->first << ':' << field->second;
				if (++field; field != record->cend()) selected_record << ',';
				selected_record << std::endl;
			}
			selected_record << '}';
			if (++record; record != records.cend()) selected_record << ',';
			selected_record << std::endl;
		}
	}
	else {
		std::stringstream value;
		while (pos < message.length()) {
			if (message[pos] == ';') break;
			value << message[pos++];
		}

		if (const auto record =
				(*table)->select_record(keilo_field{key.str(), value.str()});
			record->size() <= 0)
			selected_record << "There is no record that has " << value.str()
				<< " as " << key.str() << " in table "
				<< (*table)->get_name() << std::endl;
		else {
			selected_record << '{' << std::endl;
			for (auto field = record->cbegin(); field != record->cend();) {
				selected_record << '\t' << field->first << ':' << field->second;
				if (++field; field != record->cend()) selected_record << ',';
				selected_record << std::endl;
			}
			selected_record << '}' << std::endl;
		}
	}
	return selected_record.str();
}

std::string keilo_server::insert_record(const std::string& message, u_int pos,
                                        keilo_table** const table) {
	if (!*table) return "Please select table";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		++pos;
	}

	keilo_record record;

	while (pos < message.length()) {
		std::stringstream key;
		while (pos < message.length()) {
			if (message[pos] == COLON) break;
			key << message[pos++];
		}

		++pos;

		while (pos < message.length()) {
			if (message[pos] != SPACE) break;
			++pos;
		}

		std::stringstream value;
		while (pos < message.length()) {
			if (message[pos] == COMMA || message[pos] == SEMICOLON) break;
			value << message[pos++];
		}

		record.emplace_back(key.str(), value.str());

		while (pos < message.length()) {
			if (message[pos] != SPACE && message[pos] != ',' &&
				message[pos] != SEMICOLON)
				break;
			pos++;
		}
	}
	return (*table)->insert_record(record);
}

std::string keilo_server::update_record(const std::string& message, u_int pos,
                                        keilo_table** const table) {
	if (!*table) return "Please select table.";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream condition_key;
	while (pos < message.length()) {
		if (message[pos] == COLON) break;
		condition_key << message[pos++];
	}
	pos += 1;

	std::stringstream condition_value;
	while (pos < message.length()) {
		if (message[pos] == SPACE) break;
		condition_value << message[pos++];
	}
	pos += 1;

	std::stringstream new_key;
	while (pos < message.length()) {
		if (message[pos] == COLON) break;
		new_key << message[pos++];
	}
	pos += 1;

	std::stringstream new_value;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		new_value << message[pos++];
	}

	return (*table)
		->update_record(keilo_field{condition_key.str(), condition_value.str()},
		                keilo_field{new_key.str(), new_value.str()});
}

std::string keilo_server::remove_record(const std::string& message, u_int pos,
                                        keilo_table** const table) {
	if (!*table) return "Please select table.";

	if (pos >= message.length()) return "Syntax error.";

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream key;
	while (pos < message.length()) {
		if (message[pos] == COLON) break;
		key << message[pos++];
	}
	pos += 1;

	while (pos < message.length()) {
		if (message[pos] != SPACE) break;
		pos++;
	}

	std::stringstream value;
	while (pos < message.length()) {
		if (message[pos] == SEMICOLON) break;
		value << message[pos++];
	}

	return (*table)->remove_record(keilo_field{key.str(), value.str()});
}
