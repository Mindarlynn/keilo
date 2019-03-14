#include <iostream>
#include <string>

#include "keilo.hpp"

int main() {
	try {
		keilo::application app{};

		// (application.hpp)
		app.create_database("db1");
		auto db1 = app.select_database("db1");

		const keilo::database db2{ "db2" };
		app.add_database(db2);

		// adding database
		std::cout << "<<adding database>>" <<"\n\n";
		for (const auto& db : app.get_databases())
			std::cout << db.get_name() << std::endl;
		std::cout << std::endl;

		app.drop_database("db2");

		// dropping database
		std::cout << "<<dropping database>>" <<"\n\n";
		for (const auto& db : app.get_databases())
			std::cout << db.get_name() << std::endl;
		std::cout << std::endl;

		// (database.hpp)
		db1->create_table("tb1", "name");
		auto tb1 = db1->select_table("tb1");

		const keilo::table tb2{ "tb2", "name" };
		db1->add_table(tb2);

		// adding table
		std::cout<<"<<adding table>>" <<"\n\n";
		for(const auto& tb : db1->get_tables()) 
			std::cout << tb.get_name() << std::endl;
		std::cout << std::endl;

		db1->drop_table("tb2");

		// dropping table
		std::cout << "<<dropping table>>" <<"\n\n";
		for (const auto& tb : db1->get_tables())
			std::cout << tb.get_name() << std::endl;
		std::cout << std::endl;

		// (table.hpp)

		// inserting
		tb1->insert_record({ {{"name", "a"}, {"hobby", "soccer"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "c"}, {"hobby", "baseball"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "b"}, {"hobby", "basketball"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "e"}, {"hobby", "game"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "d"}, {"hobby", "movie"}}, tb1->get_key() });

		std::cout << "<<inserting>>" <<"\n\n";
		for (auto& record : tb1->get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// updating
		tb1->update_record({ {"name", "a"} }, { {"hobby", "jogging"} });

		std::cout << "<<updating>>" <<"\n\n";
		for (auto& record : tb1->get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// removing
		tb1->remove_record({ {"hobby", "jogging"} });

		std::cout << "<<removing>>" <<"\n\n";
		for (auto& record : tb1->get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// joining
		keilo::table tb3{ "tb3", "name" };
		tb3.insert_record({ {{"name", "a"}, {"age", "10"}} , tb3.get_key() });
		tb3.insert_record({ {{"name", "b"}, {"age", "12"}} , tb3.get_key() });
		tb3.insert_record({ {{"name", "c"}, {"age", "13"}} , tb3.get_key() });
		tb3.insert_record({ {{"name", "d"}, {"age", "14"}} , tb3.get_key() });

		auto tb1_3 = tb1->join(tb3);

		std::cout << "<<joining>>" <<"\n\n";
		for(auto& record : tb1_3.get_records())
			for(const auto& instance : record()) 
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// sorting
		tb1_3.sort(false);

		std::cout << "<<sorting>> (false = descending)" <<"\n\n";
		for (auto& record : tb1_3.get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		app.export_database("db1", "db1.json");
	}
	catch(std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}