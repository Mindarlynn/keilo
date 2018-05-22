#pragma once

#include "keilo_database.hpp"

#include <string>
#include <mutex>

class keilo_application
{
public:
	/**
	 * \brief Create empty database.
	 * \param name Database's name.
	 * \return Result of creating database.
	 */
	std::string create_database(const std::string& name);

	/**
	 * \brief Select database that has same name with parameter.
	 * \param name Database's name.
	 * \return Selected database or nullptr.
	 */
	keilo_database* select_database(const std::string& name);

	/**
	 * \brief Import database from file.
	 * \param file_name Database file's name.
	 * \return Result of importing file.
	 */
	std::string import_file(const std::string& file_name);

	/**
	 * \brief Export database to file.
	 * \param database_name Database's name.
	 * \param file_name File's name to export.
	 * \return Result of exporting database.
	 */
	std::string export_database(const std::string& database_name, const std::string& file_name);

	/**
	 * \brief Get databases.
	 * \return List of database.
	 */
	std::list<keilo_database> get_databases();

private:
	/**
	 * \brief Mutex of `databases_`. 
	 */
	std::mutex mutex_;

	/**
	 * \brief List of database.
	 */
	std::list<keilo_database> databases_;
};
