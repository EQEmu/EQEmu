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

#define EQEMU_PROTOCOL_VERSION "0.3.10"

// Build variables
// these get injected during the build pipeline
#define CURRENT_VERSION "23.10.3-dev" // always append -dev to the current version for custom-builds
#define LOGIN_VERSION "0.8.0"
#define COMPILE_DATE    __DATE__
#define COMPILE_TIME    __TIME__
#ifndef WIN32
#define LAST_MODIFIED    __TIME__
#else
#define LAST_MODIFIED	__TIMESTAMP__
#endif

/**
 * Every time a Database SQL is added to Github increment CURRENT_BINARY_DATABASE_VERSION
 * number and make sure you update the manifest
 *
 * Manifest: https://github.com/EQEmu/Server/blob/master/utils/sql/db_update_manifest.txt
 */

#define CURRENT_BINARY_DATABASE_VERSION 9328
#define CURRENT_BINARY_BOTS_DATABASE_VERSION 9055
#define CUSTOM_BINARY_DATABASE_VERSION 0
