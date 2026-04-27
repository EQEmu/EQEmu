#pragma once

#include "rof2.h"
#include "../struct_strategy.h"

class EQStreamIdentifier;

namespace TOB
{

	//these are the only public member of this namespace.
	extern void Register(EQStreamIdentifier& into);
	extern void Reload();



	//you should not directly access anything below..
	//I just dont feel like making a seperate header for it.

	class Strategy : public StructStrategy {
	public:
		Strategy();

	protected:

		virtual std::string Describe() const;
		virtual const EQ::versions::ClientVersion ClientVersion() const;

		//magic macro to declare our opcode processors
#include "ss_declare.h"
#include "tob_ops.h"

	};

}; /*TOB*/

namespace Message {

class TOB : public RoF2
{
public:
	TOB() = default;
	~TOB() override = default;

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

} // namespace Message

namespace Buff {

class TOB : public RoF2
{
public:
	TOB() = default;
	~TOB() override = default;

	// std::unique_ptr<EQApplicationPacket> MakeLegacyBuffsPacket(Mob* mob, bool for_target,
	// 	bool clear_buffs) const override;

	std::unique_ptr<EQApplicationPacket>
	BuffDefinition(Mob* mob, const Buffs_Struct& buff, int slot, bool fade) const override;
	std::unique_ptr<EQApplicationPacket> RefreshBuffs(EmuOpcode opcode, Mob* mob, int32_t timer, bool remove,
		bool buff_timers_suspended, const std::vector<uint32_t>& slots) const override;
	void SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, Mob* source, Client* target) const override;
};

} // namespace Buff
