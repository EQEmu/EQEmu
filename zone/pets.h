#pragma once

class Mob;
struct NPCType;

class Pet : public NPC {
public:
	Pet(NPCType *type_data, Mob *owner, uint8 pet_type, uint16 spell_id, int16 power);
	virtual bool CheckSpellLevelRestriction(Mob *caster, uint16 spell_id);
};
