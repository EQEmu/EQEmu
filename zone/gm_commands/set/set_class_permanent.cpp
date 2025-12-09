#include "../../client.h"

void SetClassPermanent(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set class_permanent [Class ID]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 class_id = Strings::ToUnsignedInt(sep->arg[2]);

	LogInfo("Class changed by {} for {} to {} ({}).",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		GetClassIDName(class_id),
		class_id
	);

	t->SetBaseClass(class_id);
	t->Save();
	t->Kick("Class was changed.");

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"Class changed for {} to {} ({}).",
				c->GetTargetDescription(t),
				GetClassIDName(class_id),
				class_id
			).c_str()
		);
	}
}
