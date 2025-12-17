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