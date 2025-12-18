#include "common/repositories/petitions_repository.h"
#include "zone/client.h"

void ShowPetition(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(2)) {
		const auto& l = PetitionsRepository::All(database);

		uint32 found_count = 0;

		for (const auto& e : l) {
			c->Message(
				Chat::White,
				fmt::format(
					"Petition {} | Name: {} Text: {}",
					e.petid,
					e.charname,
					e.petitiontext
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}

		if (found_count == 50) {
			c->Message(Chat::White, "50 Petitions found, max reached.");
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} Petition{} found.",
				found_count,
				found_count != 1 ? "s" : ""
			).c_str()
		);

		return;
	}

	const uint32 petition_id = Strings::ToUnsignedInt(sep->arg[2]);

	const auto& l = PetitionsRepository::GetWhere(database, fmt::format("petid = {}", petition_id));
	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Petition ID {} was not found.",
				petition_id
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Petition {} | Name: {} Text: {}",
			l[0].petid,
			l[0].charname,
			l[0].petitiontext
		).c_str()
	);
}
