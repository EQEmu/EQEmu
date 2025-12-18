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
