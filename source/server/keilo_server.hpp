#pragma once

#include "keilo_application.hpp"

#include <tcp_socket.hpp>
#include <memory>
#include <list>
#include <atomic>
#include <mutex>

#include <string_process.h>

#pragma region Command
	#define CREATE			"create"
	#define SELECT			"select"
	#define JOIN			"join"
	#define INSERT			"insert"
	#define UPDATE			"update"
	#define REMOVE			"remove"
	#define DROP			"drop"
	#define EXPORT_FILE		"export"
	#define IMPORT_FILE		"import"
	#define CLEAR			"clear"
	#define DATABASE		"database"
	#define TABLE			"table"
	#define RECORD			"record"
#pragma endregion

class keilo_server
{
public:

	explicit keilo_server(const int& port);
	~keilo_server();

#pragma region User Accessable Functions
	void run();
	void import_file(const std::string& file_name) const;
	//void run_local();
#pragma endregion

#pragma region Networking

	std::thread accept_thread;
	std::vector<std::thread> login_threads;

	/**
	 * \brief Communicate with client.
	 * \param client Client that accepted from `accept_thread`.
	 * \param database Selected database.
	 * \param table Selected table.
	 */
	void process_client(tcp_socket**const client, keilo_database** database, keilo_table** table);

	/**
	 * \brief Process commands that client sent.
	 * \param message Message that client sent.
	 * \param database Selected database.
	 * \param table Selected table.
	 * \return result of processing command.
	 */
	std::string process_message(const std::string& message, keilo_database** database, keilo_table** table) const;

	/**
	 * \brief 
	 * \param client check this parameter whether exist in client list.
	 * \return iterator of list
	 */
	std::list<tcp_socket*>::iterator find_client(tcp_socket*const client);

	/**
	 * \brief List of clients.
	 */
	std::list<tcp_socket*> clients;

	/**
	* \brief Server Socket(TCP).
	*/
	tcp_socket* socket;

	/**
	* \brief Error handling variable that Windows support.
	*/
	WSADATA wsa;
	int res_wsa;

	/**
	 * \brief List of clients that is communicating with server.
	 */
	std::list<std::thread> client_processes;
#pragma endregion


#pragma region Processing
	/**
	 * \brief Create database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \return Result of creating database
	 */
	std::string create_database(const std::string& message, u_int pos) const;

	/**
	 * \brief Select database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database will be assigned into this variable.
	 * \return Result of selecting database
	 */
	std::string select_database(const std::string& message, u_int pos, keilo_database** const database) const;

	/**
	 * \brief  Export database to file.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \return Result of exporting database
	 */
	std::string export_database(const std::string& message, u_int pos, keilo_database** const database) const;

	/**
	 * \brief Import database from file.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \return Result of importing database.
	 */
	std::string import_database(const std::string& message, u_int pos) const;

	/**
	 * \brief Create table into the database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \return Result of creating table.
	 */
	static std::string create_table(const std::string& message, u_int pos, keilo_database** const database);

	/**
	 * \brief Select table into the database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \param table Selected table will be assigned into this variable.
	 * \return Result of selecting table
	 */
	static std::string select_table(const std::string& message, u_int pos, keilo_database** const database, keilo_table** const table);

	/**
	 * \brief Join table.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \param table Selected table.
	 * \return Result of joining tables
	 */
	std::string join_table(const std::string& message, u_int pos, keilo_database** const database, keilo_table** const table) const;

	/**
	 * \brief Drop table.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \param table Selected table.
	 * \return Result of dropping table.
	 */
	static std::string drop_table(const std::string& message, u_int pos, keilo_database** const database, keilo_table** const table);

	/**
	 * \brief Select records from table
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Selected records.
	 */
	static std::string select_record(const std::string& message, u_int pos, keilo_table** const table);

	/**
	 * \brief Insert record into table.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Result of inserting record.
	 */
	static std::string insert_record(const std::string& message, u_int pos, keilo_table** const table);

	/**
	 * \brief Update record that meets the condition.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Result of updating record.
	 */
	static std::string update_record(const std::string& message, u_int pos, keilo_table** const table);

	/**
	 * \brief Remove record that meets the condition.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Result of removing record.
	 */
	static std::string remove_record(const std::string& message, u_int pos, keilo_table** const table);
#pragma endregion

	/**
	 * \brief Database that has user informations.
	 */
	keilo_database* user_database;
	/**
	 * \brief Whether program is running.
	 */
	std::atomic<bool> is_running = false;

	/**
	 * \brief Core database.
	 */
	std::unique_ptr<keilo_application> application;

	string_process* processer;
};
