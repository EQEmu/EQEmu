/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "common/cli/eqemu_command_handler.h"
#include "common/cli/terminal_color.hpp"
#include "common/platform.h"

#include "fmt/format.h"

namespace EQEmuCommand {

	std::map<std::string, void (*)(
		int argc,
		char **argv,
		argh::parser &cmd,
		std::string &description
	)> function_map;

	/**
	 * @param cmd
	 */
	void DisplayDebug(argh::parser &cmd)
	{
		if (cmd[{"-d", "--debug"}]) {
			std::cout << "Positional args:\n";
			for (auto &pos_arg: cmd.pos_args())
				std::cout << '\t' << pos_arg << std::endl;

			std::cout << "\nFlags:\n";
			for (auto &flag: cmd.flags())
				std::cout << '\t' << flag << std::endl;

			std::cout << "\nParameters:\n";
			for (auto &param: cmd.params())
				std::cout << '\t' << param.first << " : " << param.second << std::endl;
		}
	}

	/**
	 * @param arguments
	 * @param options
	 * @param cmd
	 * @param argc
	 * @param argv
	 */
	void ValidateCmdInput(
		std::vector<std::string> &arguments,
		std::vector<std::string> &options,
		argh::parser &cmd,
		int argc,
		char **argv
	)
	{
		bool arguments_filled = true;

		int       index = 2;
		for (auto &arg: arguments) {
			if (cmd(arg).str().empty() && cmd(index).str().empty()) {
				arguments_filled = false;
			}
			index++;
		}

		if (!arguments_filled || (argc == 2 && !cmd[{"-h", "--help"}]) || (argc == 3 && cmd[{"-h", "--help"}])) {
			std::string arguments_string;
			for (auto   &arg: arguments) {
				arguments_string += " " + arg;
			}

			std::string options_string;
			for (auto   &opt: options) {
				options_string += "  " + opt + "\n";
			}

			std::stringstream command_string;

			command_string <<
						   termcolor::colorize <<
						   termcolor::yellow <<
						   "\nCommand" <<
						   termcolor::reset << "\n\n" <<
						   termcolor::green << argv[1] << arguments_string << termcolor::reset << "\n" <<
						   termcolor::yellow << (!options_string.empty() ? "\nOptions\n\n" : "") <<
						   termcolor::reset << termcolor::cyan << options_string << termcolor::reset;

			std::cout << command_string.str() << std::endl;

			exit(0);
		}
	}

	/**
	 * @param in_function_map
	 * @param cmd
	 * @param argc
	 * @param argv
	 */
	void HandleMenu(
		std::map<std::string, void (*)(
			int argc,
			char **argv,
			argh::parser &cmd,
			std::string &description
		)> &in_function_map,
		argh::parser &cmd,
		int argc,
		char **argv
	)
	{
		std::string description;
		if (cmd[{"-h", "--help"}]) {
			std::cout << std::endl;
			std::cout <<
					  "> " <<
					  termcolor::yellow <<
					  "EQEmulator [" + GetPlatformName() + "] CLI Menu" <<
					  termcolor::reset
					  << std::endl
					  << std::endl;

			// Get max command length for padding length
			int max_command_length = 0;

			for (auto &it: in_function_map) {
				std::stringstream command;
				command << termcolor::colorize << termcolor::yellow << it.first << termcolor::reset;
				if (command.str().length() > max_command_length) {
					max_command_length = command.str().length() + 1;
				}
			}

			// Display command menu
			std::string command_section;
			for (auto   &it: in_function_map) {
				description.clear();

				(it.second)(argc, argv, cmd, description);

				// Print section header
				std::string command_prefix = it.first.substr(0, it.first.find(":"));

				if (command_prefix.find("test") != std::string::npos) {
					continue;
				}

				if (command_section != command_prefix) {
					command_section = command_prefix;
					std::cout << termcolor::reset << command_prefix << std::endl;
				}

				// Print commands
				std::stringstream command;
				command << termcolor::colorize << termcolor::yellow << it.first << termcolor::reset;
				printf("  %-*s %s\n", max_command_length, command.str().c_str(), description.c_str());
			}

			std::cout << std::endl;

			std::exit(0);
		}

		bool ran_command = false;

		for (auto &it: in_function_map) {
			if (it.first == argv[1]) {
				(it.second)(argc, argv, cmd, description);
				ran_command = true;
			}
		}

		if (ran_command) {
			std::exit(0);
		}
	}

}
