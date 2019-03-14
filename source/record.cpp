#include "core.hpp"

namespace keilo {
	record::record(const std::list<instance>& instances, const std::string& key) {
		if (key.empty())
			throw std::runtime_error("[record] key not exists");

		for (const auto& instance : instances)
			this->instances.emplace_back(instance);

		const auto it = std::find_if(this->instances.begin(), this->instances.end(), [key](const instance& instance) {
			return instance.get_identifier() == key;
		});

		if (it == this->instances.end())
			throw std::runtime_error("[record] key not exists");

		this->key.identifier = it->identifier;
		this->key.value = it->value;
	}

	record::record(const record& other) {
		this->key = other.key;
		for (const auto& instance : other.instances)
			this->instances.emplace_back(instance);
	}

	std::list<instance> record::operator()() {
		std::unique_lock<std::mutex> lock(mutex);
		return instances;
	}
}
