#include "../world_server_cli.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include "../../common/events/player_events.h"
#include "../../common/timer.h"

void WorldserverCLI::TestStringBenchmarkCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	enum Type {
		StringsToInt = 0,
		StringsToBigInt,
		StringsToUnsignedInt,
		StringsToUnsignedBigInt,
		StringsToFloat,
		StringsIsNumber,
		StringsIsFloat,
		StdStoi,
		StdAtoi,
		StdStoll,
		StdAtoll,
		StdStoul,
		StdStoull,
		StdStof,
	};

	struct Benchmark {
		std::string name;
		Type        type;
	};

	std::vector<Benchmark> benches = {
		Benchmark{.name = "Strings::ToInt", .type = StringsToInt},
		Benchmark{.name = "std::stoi", .type = StdStoi},
		Benchmark{.name = "std::atoi", .type = StdAtoi},
		Benchmark{.name = "Strings::ToBigInt", .type = StringsToBigInt},
		Benchmark{.name = "std::stoll", .type = StdStoll},
		Benchmark{.name = "std::atoll", .type = StdAtoll},
		Benchmark{.name = "Strings::ToUnsignedInt", .type = StringsToUnsignedInt},
		Benchmark{.name = "std::stoul", .type = StdStoul},
		Benchmark{.name = "Strings::ToUnsignedBigInt", .type = StringsToUnsignedBigInt},
		Benchmark{.name = "std::stoull", .type = StdStoull},
		Benchmark{.name = "Strings::ToFloat", .type = StringsToFloat},
		Benchmark{.name = "std::stof", .type = StdStof},
		Benchmark{.name = "Strings::IsNumber", .type = StringsIsNumber},
		Benchmark{.name = "Strings::IsFloat", .type = StringsIsFloat},
	};

	BenchTimer benchmark;

	for (auto &b: benches) {
		int         iterations   = 10000000;
		std::string number       = "1111753784";
		std::string float_number = "1111753784.2345623456345";
		int64       convert      = 0;
		uint64_t    uconvert     = 0;
		float       fconvert     = 0;
		bool        check        = false;

		for (int i = 0; i < iterations; i++) {
			switch (b.type) {
				case StringsToInt:
					convert = Strings::ToInt(number, 0);
					break;
				case StdStoi:
					convert = std::stoi(number);
					break;
				case StdAtoi:
					convert = std::atoi(number.c_str());
					break;
				case StringsToBigInt:
					convert = Strings::ToBigInt(number, 0);
					break;
				case StdStoll:
					convert = std::stoll(number);
					break;
				case StdAtoll:
					convert = std::atoll(number.c_str());
					break;
				case StringsToUnsignedInt:
					uconvert = Strings::ToUnsignedInt(number, 0);
					break;
				case StringsToUnsignedBigInt:
					uconvert = Strings::ToUnsignedBigInt(number, 0);
					break;
				case StringsToFloat:
					fconvert = Strings::ToFloat(number, 0);
					break;
				case StringsIsNumber:
					check = Strings::IsNumber(number);
					break;
				case StringsIsFloat:
					check = Strings::IsFloat(float_number);
					break;
				case StdStoul:
					uconvert = std::stoul(number);
					break;
				case StdStoull:
					uconvert = std::stoull(number);
					break;
				case StdStof:
					fconvert = std::stof(number);
					break;
			}
		}

		LogInfo("{:<30} | [{}] time [{}]", b.name, Strings::Commify(iterations), benchmark.elapsed());
		benchmark.reset();
	}
}
