#include "zone/client.h"
#include "zone/object.h"

void command_augmentitem(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	auto in_augment = new AugmentItem_Struct[sizeof(AugmentItem_Struct)];
	in_augment->container_slot = 1000; // <watch>
	in_augment->augment_slot   = -1;
	if (c->GetTradeskillObject() != nullptr) {
		Object::HandleAugmentation(c, in_augment, c->GetTradeskillObject());
	}
	safe_delete_array(in_augment);
}

