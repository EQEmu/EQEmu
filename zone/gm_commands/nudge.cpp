#include "zone/client.h"

void command_nudge(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #nudge [x=float] [y=float] [z=float] [h=float]");
		c->Message(
			Chat::White,
			fmt::format(
				"Note: Partial or mixed arguments allowed, example {}.",
				Saylink::Silent("#nudge x=5.0")
			).c_str()
		);
		return;
	}

	auto target = c->GetTarget();
	if (!target) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	if (target->IsMoving()) {
		c->Message(Chat::White, "This command requires a stationary target.");
		return;
	}

	glm::vec4 position_offset(0.0f, 0.0f, 0.0f, 0.0f);
	for (auto index = 1; index <= 4; ++index) {
		if (!sep->arg[index]) {
			continue;
		}

		Seperator argsep(sep->arg[index], '=');
		if (!argsep.arg[1][0]) {
			continue;
		}

		switch (argsep.arg[0][0]) {
			case 'x':
				position_offset.x = Strings::ToFloat(argsep.arg[1]);
				break;
			case 'y':
				position_offset.y = Strings::ToFloat(argsep.arg[1]);
				break;
			case 'z':
				position_offset.z = Strings::ToFloat(argsep.arg[1]);
				break;
			case 'h':
				position_offset.w = Strings::ToFloat(argsep.arg[1]);
				break;
			default:
				break;
		}
	}

	const auto& current_position = target->GetPosition();
	glm::vec4 new_position(
		(current_position.x + position_offset.x),
		(current_position.y + position_offset.y),
		(current_position.z + position_offset.z),
		(current_position.w + position_offset.w)
	);

	target->GMMove(new_position.x, new_position.y, new_position.z, new_position.w);

	c->Message(
		Chat::White,
		fmt::format(
			"Nudging {} to {:.2f}, {:.2f}, {:.2f}, {:.2f} with offsets of {:.2f}, {:.2f}, {:.2f}, {:.2f}.",
			c->GetTargetDescription(target),
			new_position.x,
			new_position.y,
			new_position.z,
			new_position.w,
			position_offset.x,
			position_offset.y,
			position_offset.z,
			position_offset.w
		).c_str()
	);
}

