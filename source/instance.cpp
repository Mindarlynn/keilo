#include "keilo.hpp"

namespace keilo {
	instance::instance(const std::string& identifier, const std::string& value) : identifier(identifier), value(value) {
		if (identifier.empty() || value.empty())
			throw std::runtime_error("[instance] has no value");
	}

	bool instance::operator==(const instance& other) const {
		return this->identifier == other.identifier && this->value == other.value;
	}

	bool instance::operator==(const std::pair<std::string, std::string>& other) const {
		return this->identifier == other.first && this->value == other.second;
	}

	std::string instance::get_identifier() const { return this->identifier; }
	std::string instance::get_value() const { return this->value; }

}
