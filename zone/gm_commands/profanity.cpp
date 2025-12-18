#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

#include "common/profanity_manager.h"

void command_profanity(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #profanity add [Word] - Adds a word to the profanity list");
		c->Message(Chat::White, "Usage: #profanity clear - Deletes all profanity list");
		c->Message(Chat::White, "Usage: #profanity delete [Word] - Deletes a word from the profanity list");
		c->Message(Chat::White, "Usage: #profanity list - Shows the profanity list");
		c->Message(Chat::White, "Usage: #profanity reload - Reloads the profanity list");
		return;
	}
	
	bool is_add = !strcasecmp(sep->arg[1], "add");
	bool is_clear = !strcasecmp(sep->arg[1], "clear");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_reload = !strcasecmp(sep->arg[1], "reload");
	if (
		!is_add &&
		!is_clear &&
		!is_delete &&
		!is_list &&
		!is_reload
	) {
		c->Message(Chat::White, "Usage: #profanity add [Word] - Adds a word to the profanity list");
		c->Message(Chat::White, "Usage: #profanity clear - Deletes all profanity list");
		c->Message(Chat::White, "Usage: #profanity delete [Word] - Deletes a word from the profanity list");
		c->Message(Chat::White, "Usage: #profanity list - Shows the profanity list");
		c->Message(Chat::White, "Usage: #profanity reload - Reloads the profanity list");
		return;
	}

	if (is_add) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #profanity add [Word] - Adds a word to the profanity list");
			return;
		}

		std::string profanity = sep->arg[2];
		if (!EQ::ProfanityManager::AddProfanity(&database, profanity)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Could not add '{}' to the profanity list.",
					profanity
				).c_str()
			);
			return;
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Added '{}' to the profanity list.",
					profanity
				).c_str()
			);
		}

		auto pack = new ServerPacket(ServerOP_RefreshCensorship);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	} else if (is_clear) {
		if (!EQ::ProfanityManager::DeleteProfanityList(&database)) {
			c->Message(Chat::White, "Could not clear the profanity list.");
			return;
		} else {
			c->Message(Chat::White, "Cleared the profanity list.");
		}

		auto pack = new ServerPacket(ServerOP_RefreshCensorship);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	} else if (is_delete) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #profanity delete [Word] - Deletes a word from the profanity list");
			return;
		}

		std::string profanity = sep->arg[2];
		if (!EQ::ProfanityManager::RemoveProfanity(&database, profanity)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Could not delete '{}' from the profanity list.",
					profanity
				).c_str()
			);
			return;
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Deleted '{}' from the profanity list.",
					profanity
				).c_str()
			);
		}

		auto pack = new ServerPacket(ServerOP_RefreshCensorship);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	} else if (is_list) {
		std::string popup_message;
		std::string popup_title;
		if (!EQ::ProfanityManager::IsCensorshipActive()) {
			popup_title = "Profanity List [Empty]";
			popup_message = "The profanity list is empty.";
		} else {
			auto profanity_index = 1;
			auto profanity_list = EQ::ProfanityManager::GetProfanityList();

			popup_title = fmt::format(
				"Profanity List [{} Entr{}]",
				profanity_list.size(),
				profanity_list.size() != 1 ? "ies" : "y"
			);
			
			for (const auto& profanity : profanity_list) {
				popup_message.append(
					fmt::format(
						"{}. {}<br>",
						profanity_index,
						profanity
					)
				);

				profanity_index++;
			}
		}
	
		c->SendPopupToClient(
			popup_title.c_str(),
			popup_message.c_str()
		);
	} else if (is_reload) {
		if (!EQ::ProfanityManager::UpdateProfanityList(&database)) {
			c->Message(Chat::White, "Could not reload the profanity list.");
			return;
		} else {
			c->Message(Chat::White, "Reloaded the profanity list.");
		}

		auto pack = new ServerPacket(ServerOP_RefreshCensorship);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

