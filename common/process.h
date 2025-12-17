#pragma once

class Process {
public:
	static std::string execute(const std::string &cmd, bool return_result = true);
};
