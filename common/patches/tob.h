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
	TOB() {}
	~TOB() override {}

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
