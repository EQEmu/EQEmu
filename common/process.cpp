#include "process.h"

#include <algorithm>
#include <fstream>
#include <string>

inline std::string random_string(size_t length)
{
	auto        randchar = []() -> char {
		const char   charset[] = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[static_cast<size_t>(std::rand()) % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

std::string Process::execute(const std::string &cmd, bool return_result)
{
	std::string random     = "/tmp/" + random_string(25);
	const char  *file_name = random.c_str();

	if (return_result) {
#ifdef _WINDOWS
		std::system((cmd + " > " + file_name + " 2>&1").c_str());
#else
		std::system((cmd + " > " + file_name + " 2>&1").c_str());
#endif
	}
	else {
		std::system((cmd).c_str());
	}

	std::string result;

	if (return_result) {
		std::ifstream file(file_name);
		result = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
		std::remove(file_name);

	}

	return result;
}
