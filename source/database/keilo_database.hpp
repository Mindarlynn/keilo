#pragma once

#include "keilo_table.hpp"

#include <string>
#include <mutex>

class keilo_database {
public:
	explicit keilo_database(const std::string& name);
	explicit keilo_database(std::ifstream* const file);
	keilo_database(const keilo_database& other);

	/**
	* \brief Create empty table.
	* \param name Table's name.
	* \return Result of creating table.
	*/
	std::string create_table(const std::string& name);

	/**
	* \brief Add table into the database.
	* \param other Table that will be added into the database.
	* \return Result of adding table.
	*/
	std::string add_table(const keilo_table& other);

	/**
	* \brief Select table that has same name with parameter.
	* \param name Table's name.
	* \return Selected table or nullptr.
	*/
	keilo_table* select_table(const std::string& name);

	/**
	* \brief Drop table that has same name with parameter.
	* \param name Table's name
	* \return Result of dropping table.
	*/
	std::string drop_table(const std::string& name);

private:
	/**
	* \brief Parse database file.
	* \param file Database file.
	*/
	void parse_file(std::ifstream* const file);

public:
	/**
	* \brief Returns list of table.
	* \return List of table.
	*/
	std::list<keilo_table> get_tables();

	/**
	* \brief Get table's name.
	* \return Table's name.
	*/
	std::string get_name() const;

private:
	/**
	* \brief Set database's name.
	* \param name Database's new name.
	*/
	void set_name(const std::string& name);

	/**
	* \brief Table's name.
	*/
	std::string name_;

	/**
	* \brief Mutex of `tables_`.
	*/
	std::mutex mutex_;

	/**
	* \brief List of table.
	*/
	std::list<keilo_table> tables_;
};
