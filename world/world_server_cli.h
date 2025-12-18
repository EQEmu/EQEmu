
#pragma once

#include "common/cli/eqemu_command_handler.h"

class WorldserverCLI {
public:
	static void CommandHandler(int argc, char **argv);
	static void BotsEnable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void BotsDisable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void MercsEnable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void MercsDisable(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void Version(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void CopyCharacter(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseVersion(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseSetAccountStatus(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseGetSchema(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseDump(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseUpdates(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestColors(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void ExpansionTestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestRepository(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestRepository2(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestDatabaseConcurrency(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestStringBenchmarkCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void EtlGetSettings(int argc, char **argv, argh::parser &cmd, std::string &description);
};
