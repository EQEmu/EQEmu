#if !defined(_L__EQDATAB__H)
#define _L__EQDATAB__H

#include "misc_functions.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <mysql.h>
#include <string>
#include <ctime>
#include <iostream>

using namespace std;

#pragma comment ( lib, "libmysql" )

class EQEmuDatabase {
public:
	EQEmuDatabase(string serverName, string databaseName, string dbUsername, string dbPassword);
	~EQEmuDatabase();

	bool Connected() { if(_mysql){ return true; }else{ return false; } }

	void SetDBUsername(string dbUsername) { _dbUsername = dbUsername; };
	void SetDBPassword(string dbPassword) { _dbPassword = dbPassword; };
	void SetDatabaseName(string databaseName) { _databaseName = databaseName; };
	void SetServerName(string serverName) { _serverName = serverName; };
	string GetDBUsername() { return _dbUsername; };
	string GetDBPassword() { return _dbPassword; };
	string GetDatabaseName() { return _databaseName; };
	string GetServerName() { return _serverName; };

	void GetPlayer(std::string name);
	void GetPlayers();
	void StorePlayer(int32 charid, char* data);

private:
	string _dbUsername;
	string _dbPassword;
	string _databaseName;
	string _serverName;
	MYSQL *_mysql;
};
#endif