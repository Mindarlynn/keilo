#pragma once

namespace keilo {
	enum class result_t {
		success = 0,
		fail,
		has_no_value,
		cannot_find,
		already_exist,
		key_duplicated,
		key_not_exist,
		file_not_support
	};
}

#include <string>
#include <fstream>
#include <mutex>
#include <list>
#include <algorithm>
#include <filesystem>
#include <sstream>

#include "instance.hpp"
#include "record.hpp"
#include "table.hpp"
#include "database.hpp"
#include "application.hpp"
#include "json.hpp"
