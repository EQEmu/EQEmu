#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_logs(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"#logs list - Shows current log settings and categories loaded into the current process' memory for the first 50 log categories"
		);
		c->Message(
			Chat::White,
			"#logs list [Start Category ID] - Shows current log settings and categories loaded into the current process' memory, only shows 50 at a time starting at specified Category ID"
		);
		c->Message(
			Chat::White,
			"#logs reload - Reload all settings in world and all zone processes with what is defined in the database"
		);
		c->Message(
			Chat::White,
			"#logs set [console|file|gmsay] [Category ID] [Debug Level (1-3)] - Sets log settings during the lifetime of the zone"
		);
		return;
	}

	bool is_list   = !strcasecmp(sep->arg[1], "list");
	bool is_reload = !strcasecmp(sep->arg[1], "reload");
	bool is_set    = !strcasecmp(sep->arg[1], "set");

	if (!is_list && !is_reload && !is_set) {
		c->Message(
			Chat::White,
			"#logs list - Shows current log settings and categories loaded into the current process' memory for the first 50 log categories"
		);
		c->Message(
			Chat::White,
			"#logs list [Start Category ID] - Shows current log settings and categories loaded into the current process' memory, only shows 50 at a time starting at specified Category ID"
		);
		c->Message(
			Chat::White,
			"#logs reload - Reload all settings in world and all zone processes with what is defined in the database"
		);
		c->Message(
			Chat::White,
			"#logs set [console|file|gmsay] [Category ID] [Debug Level (1-3)] - Sets log settings during the lifetime of the zone"
		);
		return;
	}

	if (is_list || (is_set && !sep->IsNumber(3))) {
		uint32 start_category_id = 1;
		if (sep->IsNumber(2)) {
			start_category_id = Strings::ToUnsignedInt(sep->arg[2]);
		}

		uint32 max_category_id = (start_category_id + 49);

		c->Message(Chat::White, "------------------------------------------------");

		for (int index = start_category_id; index <= max_category_id; index++) {
			if (index >= Logs::LogCategory::MaxCategoryID) {
				max_category_id = (Logs::LogCategory::MaxCategoryID - 1);
				break;
			}

			bool is_deprecated_category = Strings::Contains(
				fmt::format("{}", Logs::LogCategoryName[index]),
				"Deprecated"
			);
			if (is_deprecated_category) {
				continue;
			}

			std::vector<std::string> gmsay;
			for (int                 i = 0; i <= 2; i++) {
				if (EQEmuLogSys::Instance()->log_settings[index].log_to_gmsay == i) {
					gmsay.emplace_back(std::to_string(i));
					continue;
				}

				gmsay.emplace_back(
					Saylink::Create(
						fmt::format("#logs set gmsay {} {}", index, i), false, std::to_string(i)
					)
				);
			}

			std::vector<std::string> file;
			for (int                 i = 0; i <= 2; i++) {
				if (EQEmuLogSys::Instance()->log_settings[index].log_to_file == i) {
					file.emplace_back(std::to_string(i));
					continue;
				}

				file.emplace_back(
					Saylink::Create(
						fmt::format("#logs set file {} {}", index, i), false, std::to_string(i)
					)
				);
			}

			std::vector<std::string> console;
			for (int                 i = 0; i <= 2; i++) {
				if (EQEmuLogSys::Instance()->log_settings[index].log_to_console == i) {
					console.emplace_back(std::to_string(i));
					continue;
				}

				console.emplace_back(
					Saylink::Create(
						fmt::format("#logs set console {} {}", index, i), false, std::to_string(i)
					)
				);
			}

			std::vector<std::string> discord;
			for (int                 i = 0; i <= 2; i++) {
				if (EQEmuLogSys::Instance()->log_settings[index].log_to_discord == i) {
					discord.emplace_back(std::to_string(i));
					continue;
				}

				discord.emplace_back(
					Saylink::Create(
						fmt::format("#logs set discord {} {}", index, i), false, std::to_string(i)
					)
				);
			}

			std::string gmsay_string   = Strings::Join(gmsay, "-");
			std::string console_string = Strings::Join(console, "-");
			std::string file_string    = Strings::Join(file, "-");
			std::string discord_string = Strings::Join(discord, "-");

			c->Message(
				0,
				fmt::format(
					"[{}] GM [{}] Console [{}] File [{}] Discord [{}] [{}] ",
					index,
					Strings::RTrim(gmsay_string, "-"),
					Strings::RTrim(console_string, "-"),
					Strings::RTrim(file_string, "-"),
					Strings::RTrim(discord_string, "-"),
					Logs::LogCategoryName[index]
				).c_str()
			);
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"Viewing log category settings from {} ({}) to {} ({}).",
				Logs::LogCategoryName[start_category_id],
				start_category_id,
				Logs::LogCategoryName[max_category_id],
				max_category_id
			).c_str()
		);
		c->Message(Chat::White, "------------------------------------------------");

		int next_category_id = (max_category_id + 1);
		if (next_category_id < Logs::LogCategory::MaxCategoryID) {
			auto next_list_string = fmt::format(
				"#logs list {}",
				next_category_id
			);

			auto next_list_link = Saylink::Silent(next_list_string, next_list_string);

			c->Message(
				Chat::White,
				fmt::format(
					"To view the next 50 log settings, you can use {}.",
					next_list_link
				).c_str()
			);
		}
	}
	else if (is_reload) {
		c->Message(Chat::White, "Attempting to reload Log Settings globally.");
		worldserver.SendReload(ServerReload::Type::Logs);
	}
	else if (is_set && sep->IsNumber(3)) {
		auto logs_set   = false;
		bool is_console = !strcasecmp(sep->arg[2], "console");
		bool is_file    = !strcasecmp(sep->arg[2], "file");
		bool is_gmsay   = !strcasecmp(sep->arg[2], "gmsay");
		bool is_discord = !strcasecmp(sep->arg[2], "discord");

		if (!sep->IsNumber(4) || (!is_console && !is_file && !is_gmsay && !is_discord)) {
			c->Message(
				Chat::White,
				"#logs set [console|file|gmsay] [Category ID] [Debug Level (1-3)] - Sets log settings during the lifetime of the zone"
			);
			c->Message(Chat::White, "Example: #logs set gmsay 20 1 - Would output Quest errors to gmsay");
			return;
		}

		logs_set = true;

		auto category_id = Strings::ToUnsignedInt(sep->arg[3]);
		auto setting     = Strings::ToUnsignedInt(sep->arg[4]);

		if (is_console) {
			EQEmuLogSys::Instance()->log_settings[category_id].log_to_console = setting;
		}
		else if (is_file) {
			EQEmuLogSys::Instance()->log_settings[category_id].log_to_file = setting;
		}
		else if (is_gmsay) {
			EQEmuLogSys::Instance()->log_settings[category_id].log_to_gmsay = setting;
		}
		else if (is_discord) {
			EQEmuLogSys::Instance()->log_settings[category_id].log_to_discord = setting;
		}

		if (logs_set) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) is now set to Debug Level {} for {}. This is temporary and only in memory for the current zone.",
					Logs::LogCategoryName[category_id],
					category_id,
					setting,
					sep->arg[2]
				).c_str()
			);
		}

		EQEmuLogSys::Instance()->log_settings[category_id].is_category_enabled = setting ? 1 : 0;
	}
}

