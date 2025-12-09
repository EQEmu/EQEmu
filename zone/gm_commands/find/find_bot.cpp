#include "../../client.h"
#include "../../../common/repositories/bot_data_repository.h"

void FindBot(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto bot_id = Strings::ToUnsignedInt(sep->arg[2]);

		const auto& e = BotDataRepository::FindOne(content_db, bot_id);
		if (!e.bot_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bot ID {} does not exist or is invalid.",
					bot_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bot ID {} | {}",
				bot_id,
				e.name
			).c_str()
		);

		return;
	}

	const auto search_criteria = Strings::ToLower(sep->argplus[2]);

	const auto& l = BotDataRepository::GetWhere(
		content_db,
		fmt::format(
			"LOWER(`name`) LIKE '%%{}%%' AND `name` NOT LIKE '%-deleted-%' ORDER BY `bot_id` ASC LIMIT 50",
			search_criteria
		)
	);

	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"No bots found matching '{}'.",
				sep->argplus[2]
			).c_str()
		);
	}

	auto found_count = 0;

	for (const auto& e : l) {
		c->Message(
			Chat::White,
			fmt::format(
				"Bot ID {} | {}",
				Strings::Commify(e.bot_id),
				e.name
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Bots found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Bot{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

