#include <string>
#include <memory>
#include "process.h"
#include <fmt/format.h>
#include <fmt/ranges.h>

std::string Process::execute(const std::string &cmd)
{
	std::string           command = fmt::format("{} 2>&1", cmd);
	std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
	if (!pipe) { return "ERROR"; }
	char        buffer[128];
	std::string result;
	while (!feof(pipe.get())) {
		if (fgets(buffer, 128, pipe.get()) != nullptr) {
			result += buffer;
		}
	}

	return result;
}
