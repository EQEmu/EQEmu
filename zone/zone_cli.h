
#pragma once

#include "common/cli/argh.h"

#include <string>

class ZoneCLI {
public:
	static void CommandHandler(int argc, char **argv);
	static void BenchmarkDatabuckets(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void SidecarServeHttp(int argc, char **argv, argh::parser &cmd, std::string &description);
	static bool RanConsoleCommand(int argc, char **argv);
	static bool RanSidecarCommand(int argc, char **argv);
	static bool RanTestCommand(int argc, char **argv);
	static void TestDataBuckets(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestNpcHandins(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestNpcHandinsMultiQuest(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestZoneState(int argc, char **argv, argh::parser &cmd, std::string &description);
};
