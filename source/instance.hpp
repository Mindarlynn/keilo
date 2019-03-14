#pragma once
#include "keilo.hpp"

namespace keilo {
	class instance {
		friend class table;
		friend class record;
		friend class application;
	public:
		instance() = default;
		instance(const std::string& identifier, const std::string& value);

		bool operator==(const instance& other) const;
		friend std::ostream& operator<<(std::ostream& os, const instance& p);

		std::string get_identifier() const;
		std::string get_value() const;

	private:
		std::string identifier;
		std::string value;
	};

	inline std::ostream& operator<<(std::ostream& os, const instance& p) {
		os << p.identifier << ':' << p.value;
		return os;
	}

}
