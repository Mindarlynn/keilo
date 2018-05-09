#pragma once

#include "keilo_core.hpp"

#include <string>
#include <mutex>
#include <unordered_map>

class keilo_table
{
public:
	explicit keilo_table(std::string name);
	keilo_table(std::string name, std::list<keilo_record> records);
	keilo_table(const keilo_table& other);

	/**
	* \brief Join tables.
	* \param other Table that will be joined.
	* \return Joined table.
	*/
	keilo_table join(keilo_table* other);

	/**
	 * \brief Select record that meets the condition.
	 * \param where Instance that record has.
	 * \return Selected record.
	 */
	keilo_record* select_record(keilo_instance where);

	/**
	 * \brief Insert record into the table.
	 * \param record Record will be inserted into the table.
	 * \return Result of inserting record.
	 */
	std::string insert_record(keilo_record& record);

	/**
	 * \brief Update record that meets the condition.
	 * \param from To find record that has `from`'s value.
	 * \param to Record's new values.
	 * \return Result of Updating record.
	 */
	std::string update_record(keilo_instance from, keilo_instance to);

	/**
	 * \brief Remove record that meets the condition.
	 * \param where To find record that has `where`'s value.
	 * \return Result of removing record.
	 */
	std::string remove_record(keilo_instance where);

	/**
	 * \brief Get list of records.
	 * \return List of record.
	 */
	std::list<keilo_record> get_records();

	/**
	 * \brief Get Count of records.
	 * \return Count of records.
	 */
	u_int count();

	/**
	 * \brief Get Table's name
	 * \return Table's name
	 */
	std::string get_name() const;

private:
	/**
	 * \brief Set table's name.
	 * \param name Table's new name.
	 */
	void set_name(std::string name);

	/**
	 * \brief Table's name
	 */
	std::string name_;

	/**
	 * \brief Mutex of `records_`.
	 */
	std::mutex mutex_;

	/**
	 * \brief List of record.
	 */
	std::list<keilo_record> records_;
};
