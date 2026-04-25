/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "IMessage.h"
#include "common/struct_strategy.h"

class EQStreamIdentifier;

namespace Titanium
{

	//these are the only public member of this namespace.
	extern void Register(EQStreamIdentifier &into);
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
		#include "titanium_ops.h"
	};

} /*Titanium*/

// out-going message packets
namespace Message {

class Titanium : public IMessage
{
public:
	Titanium() = default;
	~Titanium() override = default;

	std::unique_ptr<EQApplicationPacket> Simple(uint32_t color, uint32_t id) const override;
	std::unique_ptr<EQApplicationPacket> Formatted(uint32_t color, uint32_t id,
		const std::array<const char*, 9>& args) const override;

	std::unique_ptr<EQApplicationPacket> InterruptSpell(uint32_t message, uint32_t spawn_id,
		const char* spell_link) const override;
	std::unique_ptr<EQApplicationPacket> InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id,
		const char* name,
		const char* spell_link) const override;

protected:
	[[nodiscard]] virtual uint32_t ResolveID(uint32_t id) const;
	virtual void ResolveArguments(uint32_t id, std::array<const char*, 9>& args) const;
};

} // namespace Message

