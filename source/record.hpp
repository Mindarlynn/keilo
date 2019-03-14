#pragma once
#include "core.hpp"

namespace keilo {
	class record {
		friend class table;
		friend class database;
	public:
		record() = default;
		record(const std::list<instance>& instances, const std::string& key);
		record(const record& other);
		std::list<instance> operator()();
	private:
		std::mutex mutex;
		std::list<instance> instances;
		instance key;
	};
}
