#include "../../client.h"

void FindSpell(Client *c, const Seperator *sep)
{
	if (SPDAT_RECORDS <= 0) {
		c->Message(Chat::White, "Spells not loaded.");
		return;
	}

	const auto can_cast_spells = c->Admin() >= GetCommandStatus("castspell");

	if (sep->IsNumber(2)) {
		const auto spell_id = Strings::ToUnsignedInt(sep->arg[2]);
		if (!IsValidSpell(spell_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spell ID {} was not found.",
					spell_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spell {} | {}",
				spell_id,
				spells[spell_id].name
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (uint32 spell_id = 0; spell_id < SPDAT_RECORDS; spell_id++) {
		if (IsValidSpell(spell_id)) {
			const auto& current_spell = spells[spell_id];

			const std::string& spell_name       = current_spell.name;
			const auto&        spell_name_lower = Strings::ToLower(spell_name);
			if (!Strings::Contains(spell_name_lower, search_criteria)) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Spell {} | {}{}",
					spell_id,
					spell_name,
					(
						can_cast_spells ?
						fmt::format(
							" | {}",
							Saylink::Silent(
								fmt::format(
									"#castspell {}",
									spell_id
								),
								"Cast"
							)
						) :
						""
					)
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Spells found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Spell{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
