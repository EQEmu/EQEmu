#pragma once

#include "rof2.h"
#include "../struct_strategy.h"

class EQStreamIdentifier;

namespace TOB {

extern void Register(EQStreamIdentifier& into);
extern void Reload();

class Strategy : public StructStrategy
{
public:
	Strategy();

protected:
	virtual std::string Describe() const;
	virtual const EQ::versions::ClientVersion ClientVersion() const;

//magic macro to declare our opcode processors
#include "ss_declare.h"
#include "tob_ops.h"
};

class MessageComponent : public Titanium::MessageComponent
{
public:
	MessageComponent() = default;
	~MessageComponent() override = default;

	std::unique_ptr<EQApplicationPacket> Formatted(uint32_t color, uint32_t id,
		const std::array<const char*, 9>& args) const override;

	std::unique_ptr<EQApplicationPacket> InterruptSpell(uint32_t message, uint32_t spawn_id,
		const char* spell_link) const override;
	std::unique_ptr<EQApplicationPacket> InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id,
		const char* name, const char* spell_link) const override;

protected:
	[[nodiscard]] uint32_t ResolveID(uint32_t id) const override;
	void ResolveArguments(uint32_t id, std::array<const char*, 9>& args) const override;
};

class BuffComponent : public UF::BuffComponent
{
public:
	BuffComponent() = default;
	~BuffComponent() override = default;

	std::unique_ptr<EQApplicationPacket>
	BuffDefinition(Mob* mob, const Buffs_Struct& buff, int slot, bool fade) const override;
	std::unique_ptr<EQApplicationPacket> RefreshBuffs(EmuOpcode opcode, Mob* mob, bool remove,
		bool buff_timers_suspended, const std::vector<uint32_t>& slots) const override;
	void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, Mob* source, Client* target) const override;
};

}; /*TOB*/
