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

#include "json/json.h"

#include <string>

namespace EQ
{
	class JsonConfigFile
	{
	public:
		JsonConfigFile();
		JsonConfigFile(const Json::Value &value);
		~JsonConfigFile();

		static JsonConfigFile Load(const std::string &file_name);
		void Save(const std::string &file_name);

		std::string GetVariableString(const std::string &title, const std::string &parameter, const std::string &default_value);
		int GetVariableInt(const std::string &title, const std::string &parameter, const int default_value);
		bool GetVariableBool(const std::string &title, const std::string &parameter, const bool default_value);
		double GetVariableDouble(const std::string &title, const std::string &parameter, const double default_value);
		
		Json::Value& RawHandle() { return m_root; }
	private:
		Json::Value m_root;
	};

}