#include "zone/client.h"
#include "zone/dialogue_window.h"

void command_doanim(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #doanim [Name|Number] [Speed]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	int animation_id = -1;
	std::string animation_name;
	auto animation_speed = 0;

	if (sep->IsNumber(1)) {
		animation_id = Strings::ToInt(sep->arg[1]);

		const auto& a = animation_names_map.find(animation_id);
		if (a != animation_names_map.end()) {
			animation_name = a->second;
		}

		if (animation_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Animation ID {} does not exist or is invalid.",
					animation_id
				).c_str()
			);
			return;
		}
	} else {
		const std::string search_criteria = sep->arg[1];

		const auto& a = animations.find(Strings::ToLower(search_criteria));
		if (a != animations.end()) {
			animation_id = a->second;
			const auto& b = animation_names_map.find(animation_id);
			if (b != animation_names_map.end()) {
				animation_name = b->second;
			}
		} else {
			for (const auto& b : animation_names_map) {
				if (Strings::ToLower(b.second).find(Strings::ToLower(search_criteria)) != std::string::npos) {
					animation_id = b.first;
					animation_name = b.second;
					break;
				}
			}

			if (animation_id == -1) {
				c->Message(
					Chat::White,
					fmt::format(
						"No Animation could be found matching '{}'.",
						search_criteria
					).c_str()
				);
				return;
			}
		}
	}

	if (sep->IsNumber(2)) {
		animation_speed = Strings::ToInt(sep->arg[2]);
	}

	const auto speed_message = (
		animation_speed ?
		fmt::format(
			" at speed {}",
			animation_speed
		) :
		""
	);

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} now performing the {} ({}) animation{}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "are" : "is",
			animation_name,
			animation_id,
			speed_message
		).c_str()
	);

	t->DoAnim(animation_id, animation_speed);
}

