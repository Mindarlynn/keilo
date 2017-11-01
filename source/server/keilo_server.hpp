#pragma once

#include "keilo_application.hpp"
#include "keilo_core.hpp"

#include <memory>
#include <list>
#include <atomic>
#include <mutex>

#define SECURE_NETWORK

class keilo_server
{
public:

	explicit keilo_server(int port);
	~keilo_server();

#pragma region User Accessable Functions
	void run();
	//void run_local();

	/**
	 * \brief Import database file to server.
	 * \param file_name Name of database file.
	 * \param ps Whether to output.
	 * \return Result of importing file.
	 */
	std::string import_file(std::string file_name, bool ps = true) const;
#pragma endregion

private:
#ifdef SECURE_NETWORK
	SOCKET keyserver_;
	sockaddr_in keyserver_addr_;

	void connect_to_key_server(char* address, int port);
	std::string request_encrypt(const std::string data) const;
	std::string request_decrypt(const std::string data) const;

	static std::string read(const SOCKET socket);
	static void write(const SOCKET socket, const std::string data);
#endif

#pragma region Networking
	/**
	 * \brief `accept_thread_` 's function. Accept clients and process client's messages.
	 */
	void accept_client();

	/**
	 * \brief Communicate with client.
	 * \param client Client that accepted from `accept_thread_`.
	 * \param database Selected database.
	 * \param table Selected table.
	 */
	void process_client(client& client, keilo_database** database, keilo_table** table);

	/**
	 * \brief Process commands that client sent.
	 * \param message Message that client sent.
	 * \param database Selected database.
	 * \param table Selected table.
	 * \return result of processing command.
	 */
	std::string process_message(std::string message, keilo_database** database, keilo_table** table);

	/**
	 * \brief Disconnect client.
	 * \param address Address of client that will be disconnected from server.
	 */
	void disconnect_client(const SOCKADDR_IN address);

	/**
	 * \brief Thread that accpet clients.
	 */
	std::thread accept_thread_;

	/**
	 * \brief List of clients.
	 */
	std::list<client> clients_;

	/**
	 * \brief Server Socket(TCP).
	 */
	SOCKET socket_;

	/**
	 * \brief Error handling variable that Windows support.
	 */
	WSADATA wsa_;

	/**
	 * \brief Address of server.
	 */
	SOCKADDR_IN address_;

	/**
	* \brief Port number of server.
	*/
	int port_;

	/**
	 * \brief List of clients that is communicating with server.
	 */
	std::list<std::thread> client_processes_;
#pragma endregion


#pragma region Processing
	/**
	 * \brief Create database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \return Result of creating database
	 */
	std::string create_database(std::string message, size_t pos) const;

	/**
	 * \brief Select database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database will be assigned into this variable.
	 * \return Result of selecting database
	 */
	std::string select_database(std::string message, size_t pos, keilo_database** database) const;

	/**
	 * \brief  Export database to file.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \return Result of exporting database
	 */
	std::string export_database(std::string message, size_t pos, keilo_database** database) const;

	/**
	 * \brief Import database from file.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \return Result of importing database.
	 */
	std::string import_database(std::string message, size_t pos);

	/**
	 * \brief Create table into the database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \return Result of creating table.
	 */
	static std::string create_table(std::string message, size_t pos, keilo_database** database);

	/**
	 * \brief Select table into the database.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \param table Selected table will be assigned into this variable.
	 * \return Result of selecting table
	 */
	static std::string select_table(std::string message, size_t pos, keilo_database** database, keilo_table** table);

	/**
	 * \brief Join table.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \param table Selected table.
	 * \return Result of joining tables
	 */
	std::string join_table(std::string message, size_t pos, keilo_database** database, keilo_table** table) const;

	/**
	 * \brief Drop table.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param database Selected database.
	 * \param table Selected table.
	 * \return Result of dropping table.
	 */
	static std::string drop_table(std::string message, size_t pos, keilo_database** database, keilo_table** table);

	/**
	 * \brief Select records from table
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Selected records.
	 */
	static std::string select_record(std::string message, size_t pos, keilo_table** table);

	/**
	 * \brief Insert record into table.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Result of inserting record.
	 */
	static std::string insert_record(std::string message, size_t pos, keilo_table** table);

	/**
	 * \brief Update record that meets the condition.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Result of updating record.
	 */
	static std::string update_record(std::string message, size_t pos, keilo_table** table);

	/**
	 * \brief Remove record that meets the condition.
	 * \param message Message that was sent from client.
	 * \param pos Iterator's position.
	 * \param table Selected table.
	 * \return Result of removing record.
	 */
	static std::string remove_record(std::string message, size_t pos, keilo_table** table);
#pragma endregion

#pragma region Command
	const std::string create_ = "create";
	const std::string select_ = "select";
	const std::string join_ = "join";
	const std::string insert_ = "insert";
	const std::string update_ = "update";
	const std::string remove_ = "remove";
	const std::string drop_ = "drop";
	const std::string export_file_ = "export";
	const std::string import_file_ = "import";
	const std::string clear_ = "clear";
	const std::string database_ = "database";
	const std::string table_ = "table";
	const std::string record_ = "record";
#pragma endregion

	/**
	 * \brief Database that has user informations.
	 */
	std::unique_ptr<keilo_database> user_database_;
	/**
	 * \brief Whether program is running.
	 */
	std::atomic<bool> is_running_ = false;

	/**
	 * \brief Core database.
	 */
	std::unique_ptr<keilo_application> application_;
};
