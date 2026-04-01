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
#include "json_config.h"

#include <fstream>
#include <iostream>

EQ::JsonConfigFile::JsonConfigFile() = default;

EQ::JsonConfigFile::JsonConfigFile(const Json::Value &value)
{
	m_root = value;
}

EQ::JsonConfigFile::~JsonConfigFile() = default;

/**
 * @param file_name
 * @return
 */
EQ::JsonConfigFile EQ::JsonConfigFile::Load(
	const std::string &file_name
)
{
	JsonConfigFile ret;
	ret.m_root = Json::Value();

	std::ifstream ifs;
	ifs.open(file_name, std::ifstream::in);

	if (!ifs.good()) {
		return ret;
	}

	try {
		ifs >> ret.m_root;
	}
	catch (std::exception &) {
		return ret;
	}

	return ret;
}

/**
 * @param file_name
 * @return
 */
void EQ::JsonConfigFile::Save(
	const std::string &file_name
)
{
	std::ofstream opened_config_file;
	opened_config_file.open(file_name);

	/**
	 * Grab and build config contents
	 */
	Json::StreamWriterBuilder write_builder;
	write_builder["indentation"] = "  ";
	std::string document = Json::writeString(write_builder, m_root);

	/**
	 * Write current contents and close
	 */
	opened_config_file << document;
	opened_config_file.close();
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
std::string EQ::JsonConfigFile::GetVariableString(
	const std::string &title,
	const std::string &parameter,
	const std::string &default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asString();
		}
	}
	catch (std::exception &) {
		return default_value;
	}

	return default_value;
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
int EQ::JsonConfigFile::GetVariableInt(
	const std::string &title,
	const std::string &parameter,
	const int default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asInt();
		}
	}
	catch (std::exception &) {
		return default_value;
	}

	return default_value;
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
bool EQ::JsonConfigFile::GetVariableBool(
	const std::string &title,
	const std::string &parameter,
	const bool default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asBool();
		}
	}
	catch (std::exception &) {
		return default_value;
	}

	return default_value;
}

/**
 * @param title
 * @param parameter
 * @param default_value
 * @return
 */
double EQ::JsonConfigFile::GetVariableDouble(
	const std::string &title,
	const std::string &parameter,
	const double default_value
)
{
	try {
		if (m_root.isMember(title) && m_root[title].isMember(parameter)) {
			return m_root[title][parameter].asDouble();
		}
	}
	catch (std::exception &) {
		return default_value;
	}

	return default_value;
}
