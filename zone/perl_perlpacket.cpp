#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/types.h"
#include "zone/embperl.h"
#include "zone/perlpacket.h"

PerlPacket* Perl_PerlPacket_new(const char* class_name)
{
	return new PerlPacket();
}

PerlPacket* Perl_PerlPacket_new(const char* class_name, const char* opcode)
{
	return new PerlPacket(opcode);
}

PerlPacket* Perl_PerlPacket_new(const char* class_name, const char* opcode, uint32_t len)
{
	return new PerlPacket(opcode, len);
}

void Perl_PerlPacket_DESTROY(PerlPacket* self)
{
	delete self;
}

bool Perl_PerlPacket_SetOpcode(PerlPacket* self, const char* opcode)
{
	return self->SetOpcode(opcode);
}

void Perl_PerlPacket_Resize(PerlPacket* self, uint32_t len)
{
	self->Resize(len);
}

void Perl_PerlPacket_SendTo(PerlPacket* self, Client* who)
{
	self->SendTo(who);
}

void Perl_PerlPacket_SendToAll(PerlPacket* self)
{
	self->SendToAll();
}

void Perl_PerlPacket_Zero(PerlPacket* self)
{
	self->Zero();
}

void Perl_PerlPacket_FromArray(PerlPacket* self, perl::reference avref, uint32_t length)
{
	perl::array av_numbers = avref;
	int* numbers = new int[av_numbers.size()];

	for (int i = 0; i < av_numbers.size(); ++i)
	{
		numbers[i] = av_numbers[i];
	}

	self->FromArray(numbers, length);

	delete[] numbers;
}

void Perl_PerlPacket_SetByte(PerlPacket* self, uint32_t pos, uint8_t val)
{
	self->SetByte(pos, val);
}

void Perl_PerlPacket_SetShort(PerlPacket* self, uint32_t pos, uint16_t val)
{
	self->SetShort(pos, val);
}

void Perl_PerlPacket_SetLong(PerlPacket* self, uint32_t pos, uint32_t val)
{
	self->SetLong(pos, val);
}

void Perl_PerlPacket_SetFloat(PerlPacket* self, uint32_t pos, float val)
{
	self->SetFloat(pos, val);
}

void Perl_PerlPacket_SetString(PerlPacket* self, uint32_t pos, char* str)
{
	self->SetString(pos, str);
}

void Perl_PerlPacket_SetEQ1319(PerlPacket* self, uint32_t pos, float part13, float part19)
{
	self->SetEQ1319(pos, part13, part19);
}

void Perl_PerlPacket_SetEQ1913(PerlPacket* self, uint32_t pos, float part19, float part13)
{
	self->SetEQ1913(pos, part19, part13);
}

uint8_t Perl_PerlPacket_GetByte(PerlPacket* self, uint32_t pos)
{
	return self->GetByte(pos);
}

uint16_t Perl_PerlPacket_GetShort(PerlPacket* self, uint32_t pos)
{
	return self->GetShort(pos);
}

uint32_t Perl_PerlPacket_GetLong(PerlPacket* self, uint32_t pos)
{
	return self->GetLong(pos);
}

float Perl_PerlPacket_GetFloat(PerlPacket* self, uint32_t pos)
{
	return self->GetFloat(pos);
}

void perl_register_perlpacket()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<PerlPacket>("PerlPacket");
	package.add("DESTROY", &Perl_PerlPacket_DESTROY);
	package.add("FromArray", &Perl_PerlPacket_FromArray);
	package.add("GetByte", &Perl_PerlPacket_GetByte);
	package.add("GetFloat", &Perl_PerlPacket_GetFloat);
	package.add("GetLong", &Perl_PerlPacket_GetLong);
	package.add("GetShort", &Perl_PerlPacket_GetShort);
	package.add("Resize", &Perl_PerlPacket_Resize);
	package.add("SendTo", &Perl_PerlPacket_SendTo);
	package.add("SendToAll", &Perl_PerlPacket_SendToAll);
	package.add("SetByte", &Perl_PerlPacket_SetByte);
	package.add("SetEQ1319", &Perl_PerlPacket_SetEQ1319);
	package.add("SetEQ1913", &Perl_PerlPacket_SetEQ1913);
	package.add("SetFloat", &Perl_PerlPacket_SetFloat);
	package.add("SetLong", &Perl_PerlPacket_SetLong);
	package.add("SetOpcode", &Perl_PerlPacket_SetOpcode);
	package.add("SetShort", &Perl_PerlPacket_SetShort);
	package.add("SetString", &Perl_PerlPacket_SetString);
	package.add("Zero", &Perl_PerlPacket_Zero);
	package.add("new", (PerlPacket*(*)(const char*))&Perl_PerlPacket_new);
	package.add("new", (PerlPacket*(*)(const char*, const char*))&Perl_PerlPacket_new);
	package.add("new", (PerlPacket*(*)(const char*, const char*, uint32_t))&Perl_PerlPacket_new);
}

#endif //EMBPERL_XS_CLASSES

