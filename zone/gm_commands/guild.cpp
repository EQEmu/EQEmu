#include "zone/client.h"
#include "zone/doors.h"
#include "zone/guild_mgr.h"
#include "zone/queryserv.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;
extern QueryServ  *QServ;

void SendGuildSubCommands(Client* c)
{
	c->Message(Chat::White, "#guild create [Character ID|Character Name] [Guild Name]");
	c->Message(Chat::White, "#guild delete [Guild ID]");
	c->Message(Chat::White, "#guild details [Guild ID]");
	c->Message(Chat::White, "#guild help");
	c->Message(Chat::White, "#guild info [Guild ID]");
	c->Message(Chat::White, "#guild list");
	c->Message(Chat::White, "#guild rename [Guild ID] [New Name]");
	c->Message(Chat::White, "#guild search [Search Criteria]");
	c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
	c->Message(Chat::White, "#guild setleader [Guild ID] [Character ID|Character Name]");
	c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
	c->Message(Chat::White, "#guild status [Character ID|Character Name]");
}

void command_guild(Client* c, const Seperator* sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		SendGuildSubCommands(c);
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	bool is_create = !strcasecmp(sep->arg[1], "create");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_help = !strcasecmp(sep->arg[1], "help");
	bool is_info = !strcasecmp(sep->arg[1], "info");
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_rename = !strcasecmp(sep->arg[1], "rename");
	bool is_search = !strcasecmp(sep->arg[1], "search");
	bool is_set = !strcasecmp(sep->arg[1], "set");
	bool is_set_leader = !strcasecmp(sep->arg[1], "setleader");
	bool is_set_rank = !strcasecmp(sep->arg[1], "setrank");
	bool is_status = !strcasecmp(sep->arg[1], "status");
	bool is_details = !strcasecmp(sep->arg[1], "details");
	bool is_test = !strcasecmp(sep->arg[1], "test");
	if (
		!is_create &&
		!is_delete &&
		!is_help &&
		!is_info &&
		!is_list &&
		!is_rename &&
		!is_search &&
		!is_set &&
		!is_set_leader &&
		!is_set_rank &&
		!is_status &&
		!is_details &&
		!is_test
		) {
		SendGuildSubCommands(c);
		return;
	}

	if (is_create) {
		if (arguments < 3) {
			c->Message(Chat::White, "Usage: #guild create [Character ID|Character Name] [Guild Name]");
		}
		else {
			auto leader_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto leader_name = database.GetCharNameByID(leader_id);
			if (!leader_id || leader_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						leader_id
					).c_str()
				);
				return;
			}

			auto guild_id = guild_mgr.FindGuildByLeader(leader_id);
			if (guild_id != GUILD_NONE) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is already the leader of {} ({}).",
						leader_name,
						leader_id,
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			}
			else {
				auto guild_name = sep->argplus[3];
				auto guild_id = guild_mgr.CreateGuild(sep->argplus[3], leader_id);
				auto leader = entity_list.GetClientByCharID(leader_id);


				LogGuilds(
					"[{}]: Creating guild [{}] with leader [{}] with GM command. It was given id [{}]",
					c->GetName(),
					guild_name,
					leader_id,
					guild_id
				);

				if (guild_id == GUILD_NONE) {
					c->Message(Chat::White, "Guild creation failed.");
				}
				else {
					c->Message(
						Chat::White,
						fmt::format(
							"Guild Created | Name: {} ({}) Leader: {} ({})",
							guild_name,
							guild_id,
							leader_name,
							leader_id
						).c_str()
					);

					if (!guild_mgr.SetGuild(leader, guild_id, GUILD_LEADER)) {
						c->Message(
							Chat::White,
							fmt::format(
								"Unable to put {} ({}) in to {} ({}) in the database.",
								leader_name,
								leader_id,
								guild_name,
								guild_id
							).c_str()
						);

						c->Message(Chat::White, "Note: Run #guild set to resolve this.");
					}
				}
			}
		}
	}
	else if (is_delete) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild delete [Guild ID]");
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Guild  ID {} could not be found.",
						guild_id
					).c_str()
				);
				return;
			}

			LogGuilds(
				"[{}]: Deleting guild [{}] ([{}]) with GM command",
				c->GetName(),
				guild_mgr.GetGuildNameByID(guild_id),
				guild_id
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Guild {} Deleted | Name: {} ({})",
					guild_mgr.DeleteGuild(guild_id) ? "Successfully" : "Unsuccessfully",
					guild_mgr.GetGuildNameByID(guild_id),
					guild_id
				).c_str()
			);
		}
	}
	else if (is_help) {
		SendGuildSubCommands(c);
	}
	else if (is_info) {
		if (arguments != 2 && c->IsInAGuild()) {
			c->Message(Chat::White, "#guild info [Guild ID]");
		}
		else {
			auto guild_id = GUILD_NONE;
			if (sep->IsNumber(2)) {
				guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			}

			if (guild_id != GUILD_NONE) {
				guild_mgr.DescribeGuild(c, guild_id);
			}
		}
	}
	else if (is_list) {
		guild_mgr.ListGuilds(c, std::string());
	}
	else if (is_rename) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild rename [Guild ID] [New Guild Name]");
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Guild ID {} could not be found.",
						guild_id
					).c_str()
				);
				return;
			}

			auto new_guild_name = sep->argplus[3];
			LogGuilds(
				"[{}]: Renaming guild [{}] ([{}]) to [{}] with GM command",
				c->GetName(),
				guild_mgr.GetGuildNameByID(guild_id),
				guild_id,
				new_guild_name
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Guild {} Renamed | Name: {} ({})",
					guild_mgr.RenameGuild(guild_id, new_guild_name) ? "Successfully" : "Unsuccessfully",
					new_guild_name,
					guild_id
				).c_str()
			);
		}
	}
	else if (is_search) {
		if (Strings::IsNumber(sep->arg[2])) {
			const auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);

			guild_mgr.ListGuilds(c, guild_id);
		}
		else {
			const std::string search_criteria = sep->argplus[2];

			guild_mgr.ListGuilds(c, search_criteria);
		}
	}
	else if (is_set) {
		if (
			arguments != 3 ||
			!sep->IsNumber(3)
			) {
			c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
			return;
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[3]);
			if (!guild_id) {
				guild_id = GUILD_NONE;
			}
			else if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Guild ID {} could not be found.",
						guild_id
					).c_str()
				);
				return;
			}

			auto character_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto character_name = database.GetCharNameByID(character_id);
			auto client = entity_list.GetClientByCharID(character_id);
			if (!character_id || character_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						character_id
					).c_str()
				);
				return;
			}

			if (!guild_id || guild_id == GUILD_NONE) {
				LogGuilds(
					"[{}]: Removing [{}] ([{}]) from guild with GM command",
					c->GetName(),
					character_name,
					character_id
				);
			}
			else {
				LogGuilds(
					"[{}]: Putting [{}] ([{}]) into guild [{}] ([{}]) with GM command",
					c->GetName(),
					character_name,
					character_id,
					guild_mgr.GetGuildNameByID(guild_id),
					guild_id
				);
			}

			if (guild_id && guild_id != GUILD_NONE) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) has {} put into {} ({}).",
						character_name,
						character_id,
						guild_mgr.SetGuild(client, guild_id, GUILD_MEMBER) ? "been" : "failed to be",
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			}
			else {
				guild_mgr.SetGuild(client, GUILD_NONE, 0);
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is now Guildless.",
						character_name,
						character_id
					).c_str()
				);
			}
		}
	}
	else if (is_set_leader) {
		if (
			arguments != 3 ||
			!sep->IsNumber(2)
			) {
			c->Message(Chat::White, "Usage: #guild setleader [Guild ID] [Character ID|Character Name]");
		}
		else {
			auto leader_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto leader_name = database.GetCharNameByID(leader_id);
			if (!leader_id || leader_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						leader_id
					).c_str()
				);
				return;
			}

			auto guild_id = guild_mgr.FindGuildByLeader(leader_id);
			if (guild_id != 0) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is already the leader of {} ({}).",
						leader_name,
						leader_id,
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			}
			else {
				auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
				if (!guild_mgr.GuildExists(guild_id)) {
					c->Message(
						Chat::White,
						fmt::format(
							"Guild ID {} could not be found.",
							guild_id
						).c_str()
					);
					return;
				}

				LogGuilds(
					"[{}]: Setting leader of guild [{}] ([{}]) to [{}] with GM command",
					c->GetName(),
					guild_mgr.GetGuildNameByID(guild_id),
					guild_id,
					leader_id
				);

				c->Message(
					Chat::White,
					fmt::format(
						"Guild Leader {} Changed | Name: {} ({})",
						guild_mgr.SetGuildLeader(guild_id, leader_id) ? "Successfully" : "Unsuccessfully",
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			}
		}
	}
	else if (is_set_rank) {
		auto rank = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[3]));
		if (!sep->IsNumber(3)) {
			c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
		}
		else if (rank < 0 || rank > GUILD_MAX_RANK) {
			c->Message(
				Chat::White,
				fmt::format(
					"Guild Ranks are from 0 to {}.",
					GUILD_MAX_RANK
				).c_str()
			);
		}
		else {
			auto character_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto character_name = database.GetCharNameByID(character_id);
			if (!character_id || character_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						character_id
					).c_str()
				);
				return;
			}

			if (!guild_mgr.IsCharacterInGuild(character_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is not in a guild.",
						character_name,
						character_id
					).c_str()
				);
				return;
			}

			LogGuilds(
				"[{}]: Setting [{}] ([{}])'s guild rank to [{}] with GM command",
				c->GetName(),
				sep->arg[2],
				character_id,
				rank
			);

			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) has {} to rank {} ({}).",
					character_name,
					character_id,
					guild_mgr.SetGuildRank(character_id, rank) ? "been set" : "failed to be set",
					!guild_mgr.GetGuildRankName(guild_mgr.GetGuildIDByCharacterID(character_id), rank).empty() ? guild_mgr.GetGuildRankName(guild_mgr.GetGuildIDByCharacterID(character_id), rank) : "Nameless",
					rank
				).c_str()
			);
		}
	}
	else if (is_status) {
		auto client = (
			t ?
			t :
			(
				arguments == 2 ?
				entity_list.GetClientByName(sep->arg[2]) :
				c
				)
			);
		if (!client) {
			c->Message(Chat::White, "You must target someone or specify a character name.");
		}
		else {
			if (!client->IsInAGuild()) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} not in a guild.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == t ? "are" : "is"
					).c_str()
				);
			}
			else if (guild_mgr.IsGuildLeader(client->GuildID(), client->CharacterID())) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} the leader of {}.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == t ? "are" : "is",
						guild_mgr.GetGuildNameByID(client->GuildID())
					).c_str()
				);
			}
			else {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} a(n) {} of {}.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == t ? "are" : "is",
						guild_mgr.GetRankName(client->GuildID(), client->GuildRank()),
						guild_mgr.GetGuildNameByID(client->GuildID())
					).c_str()
				);
			}
		}
	}
	else if (is_details) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild details [Guild ID]");
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Guild ID {} could not be found.",
						guild_id
					).c_str()
				);
				return;
			}

			auto guild = guild_mgr.GetGuildByGuildID(guild_id);
			if (!guild) {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Guild {} not found.  #guild list can be used to get guild ids.",
						guild_id
					).c_str());
				return;
			}

			if (guild) {
				c->Message(Chat::Yellow, fmt::format("Guild ID:         {}.", sep->arg[2]).c_str());
				c->Message(Chat::Yellow, fmt::format("Guild Name:       {}.", guild->name.c_str()).c_str());
				c->Message(Chat::Yellow, fmt::format("Guild Leader ID:  {}.", guild->leader).c_str());
				c->Message(Chat::Yellow, fmt::format("Guild MinStatus:  {}.", guild->minstatus).c_str());
				c->Message(Chat::Yellow, fmt::format("Guild MOTD:       {}.", guild->motd.c_str()).c_str());
				c->Message(Chat::Yellow, fmt::format("Guild MOTD Setter:{}.", guild->motd_setter.c_str()).c_str());
				c->Message(Chat::Yellow, fmt::format("Guild Channel:    {}.", guild->channel.c_str()).c_str());
				c->Message(Chat::Yellow, fmt::format("Guild URL:        {}.", guild->url.c_str()).c_str());

				for (int i = 1; i <= GUILD_MAX_RANK; i++) {
					c->Message(
						Chat::Yellow,
						fmt::format("Guild Rank:       {} - {}.", i, guild->rank_names[i].c_str()).c_str());
				}

				c->Message(Chat::Yellow, "Guild Functions:   {db_id} - {guild_id} - {perm_id} - {perm_value}.");
				for (int i = 1; i <= GUILD_MAX_FUNCTIONS; i++) {
					c->Message(
						Chat::Yellow, fmt::format(
							"Guild Function:   {} - {} - {} - {}.",
							guild->functions[i].id,
							guild->functions[i].guild_id,
							guild->functions[i].perm_id,
							guild->functions[i].perm_value
						).c_str());
				}
				c->Message(Chat::Yellow, fmt::format("Guild Tribute:  Favor     {}", guild->tribute.favor).c_str());
				c->Message(
					Chat::Yellow, fmt::format(
						"Guild Tribute:  Tribute 1 {}/{}   - Tribute 2 {}/{}",
						guild->tribute.id_1,
						guild->tribute.id_1_tier,
						guild->tribute.id_2,
						guild->tribute.id_2_tier
					).c_str());
				c->Message(
					Chat::Yellow, fmt::format(
						"Guild Tribute:  Time Remaining {} - Enabled {}",
						guild->tribute.time_remaining,
						guild->tribute.enabled
					).c_str());
			}
			for (auto &c1: entity_list.GetClientList()) {
				if (c1.second->GuildID() == guild_id) {
					c->Message(
						Chat::Yellow, fmt::format(
							"PlayerName: {} ID: {} Rank: {} OptIn: {} DirtyList: {}.",
							c1.second->GetCleanName(),
							c1.second->GuildID(),
							c1.second->GuildRank(),
							c1.second->GuildTributeOptIn(),
							c1.second->GetGuildListDirty()
						).c_str());
				}
			}
		}
	}
	else if (is_test) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild test [Guild ID]");
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			auto guild    = guild_mgr.GetGuildByGuildID(guild_id);

			// PlayerEvent::LootItemEvent e{};
			// e.charges      = -1;
			// e.corpse_name  = "Test Corpse Name";
			// e.item_id      = 123456789;
			// e.item_name    = "Test Item Name";
			// e.npc_id       = 987654321;
			// e.augment_1_id = 11;
			// e.augment_2_id = 0;
			// e.augment_3_id = 0;
			// e.augment_4_id = 44;
			// e.augment_5_id = 55;
			// e.augment_6_id = 66;
			//
			// RecordPlayerEventLogWithClient(c, PlayerEvent::LOOT_ITEM, e);
			//
			// PlayerEvent::DestroyItemEvent e2{};
			// e2.charges     = -1;
			// e2.attuned     = true;
			// e.augment_1_id = 11;
			// e.augment_2_id = 0;
			// e.augment_3_id = 0;
			// e.augment_4_id = 44;
			// e.augment_5_id = 55;
			// e.augment_6_id = 66;
			// e2.item_id     = 123456789;
			// e2.item_name   = "Test Item Destroy Name";
			// e2.reason      = "Test Item Destroy Reason";
			//
			// RecordPlayerEventLogWithClient(c, PlayerEvent::ITEM_DESTROY, e2);
			// auto id       = Strings::ToUnsignedInt(sep->arg[3]);
			// //auto guild    = guild_mgr.GetGuildByGuildID(guild_id);
			// // c->SendGuildMembersList();
			// auto bank = GuildBanks->GetGuildBank(guild_id);
			// if (id == 1) {
			// 	for (auto &[key, item]: bank->items.main_area) {
			// 		auto i = ItemsRepository::FindOne(content_db, item.item_id);
			// 		c->Message(Chat::Yellow, fmt::format("key:{:02} item:{:05} Name:{:40} Qty:{:40} Slot:{}",
			// 			key, item.item_id, i.Name, item.quantity, item.slot).c_str());
			// 	}
			// 	return;
			// }
			// if (id == 2) {
			// 	for (auto &[key, item]: bank->items.deposit_area) {
			// 		auto i = ItemsRepository::FindOne(content_db, item.item_id);
			// 		c->Message(Chat::Yellow, fmt::format("key:{:02} item:{:05} Name:{} Qty:{:40} Slot:{}",
			// 			key, item.item_id, i.Name, item.quantity, item.slot).c_str());
			// 	}
			// 	return;
			// }
			// if (id == 3) {
			// 	for (auto &[key, item]: bank->items.main_area) {
			// 		if (item.item_id == 30416) {
			// 			auto i = ItemsRepository::FindOne(content_db, item.item_id);
			// 			c->Message(Chat::Yellow, fmt::format("key:{:02} item:{:05} Name:{:40} Qty:{:40} Slot:{}",
			// 				key, item.item_id, i.Name, item.quantity, item.slot).c_str());
			// 		}
			// 	}
			// 	return;
			// }
			// if (id == 4) {
			// 	c->Message(Chat::Yellow, "Guild Test 4");
			// 	auto inst = database.CreateItem(30416, 30);
			// 	database.UpdateInventorySlot(c->CharacterID(), inst, -1);
			// 	safe_delete(inst);
			//
			// 	return;
			// }
		}
	}
}
