/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "common/cli/argh.h"

#include <map>
#include <string>
#include <vector>

namespace EQEmuCommand {

	extern std::map<std::string, void (*)(
		int argc,
		char **argv,
		argh::parser &cmd,
		std::string &description
	)> function_map;

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
	);

	/**
	 * @param cmd
	 */
	void DisplayDebug(argh::parser &cmd);

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
	);
};
