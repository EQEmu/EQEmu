#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/client.h"
#include "zone/embperl.h"
#include "zone/hate_list.h"

int64_t Perl_HateEntry_GetDamage(struct_HateList* self) // @categories Script Utility, Hate and Aggro
{
	return self->hatelist_damage;
}

Mob* Perl_HateEntry_GetEnt(struct_HateList* self) // @categories Script Utility, Hate and Aggro
{
	return self->entity_on_hatelist;
}

bool Perl_HateEntry_GetFrenzy(struct_HateList* self) // @categories Script Utility, Hate and Aggro
{
	return self->is_entity_frenzy;
}

int64_t Perl_HateEntry_GetHate(struct_HateList* self) // @categories Script Utility, Hate and Aggro
{
	return self->stored_hate_amount;
}

void Perl_HateEntry_SetDamage(struct_HateList* self, int64 value) // @categories Script Utility, Hate and Aggro
{
	self->hatelist_damage = value;
}

void Perl_HateEntry_SetEnt(struct_HateList* self, Mob* mob) // @categories Script Utility, Hate and Aggro
{
	self->entity_on_hatelist = mob;
}

void Perl_HateEntry_SetFrenzy(struct_HateList* self, bool is_frenzy) // @categories Script Utility, Hate and Aggro
{
	self->is_entity_frenzy = is_frenzy;
}

void Perl_HateEntry_SetHate(struct_HateList* self, int64 value) // @categories Script Utility, Hate and Aggro
{
	self->stored_hate_amount = value;
}

void perl_register_hateentry()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<struct_HateList>("HateEntry");
	package.add("GetDamage", &Perl_HateEntry_GetDamage);
	package.add("GetEnt", &Perl_HateEntry_GetEnt);
	package.add("GetFrenzy", &Perl_HateEntry_GetFrenzy);
	package.add("GetHate", &Perl_HateEntry_GetHate);
	package.add("SetDamage", &Perl_HateEntry_SetDamage);
	package.add("SetEnt", &Perl_HateEntry_SetEnt);
	package.add("SetFrenzy", &Perl_HateEntry_SetFrenzy);
	package.add("SetHate", &Perl_HateEntry_SetHate);
}

#endif //EMBPERL_XS_CLASSES

