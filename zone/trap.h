/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "zone/entity.h"

class Mob;
class NPC;

//ID of the NPC type to spawn when a trap is set off, to do the damage
#define TRAP_NPC_TYPE 1586

enum TrapTypes
{
	trapTypeDebuff = 0,
	trapTypeAlarm = 1,
	trapTypeMysticSpawn = 2,
	trapTypeBanditSpawn = 3,
	trapTypeDamage = 4,
};

class Trap: public Entity
{
public:
	Trap();
	virtual ~Trap();
	virtual bool Process();
	virtual bool IsTrap() const { return true; }
	void	Trigger(Mob* trigger);

	void	SpellOnTarget(Mob* trigger, uint32 spell_id);

	NPC * GetHiddenTrigger() { return hiddenTrigger; }
	void SetHiddenTrigger(NPC* n) { hiddenTrigger = n; }
	void CreateHiddenTrigger();
	void DestroyHiddenTrigger() { hiddenTrigger = nullptr; }
	void UpdateTrap(bool respawn = true, bool repopnow = false);
	//Trap data, leave this unprotected
	Timer	respawn_timer; //Respawn Time when Trap's been disarmed
	Timer	chkarea_timer;
	Timer	reset_timer; //How long a trap takes to reset before triggering again.
	uint32	trap_id; //Original ID of the trap from DB. This value never changes.
	uint32	db_id; //The DB ID of the trap that currently is spawned.
	glm::vec3 m_Position;
	float	maxzdiff;	//maximum z diff to be triggerable
	float	radius;		//radius around trap to be triggerable
	uint8	chance;		//%chance that the trap is triggered each 'tick'
	uint8	effect; //Effect ID
	int32	effectvalue; //Value of Effect
	int32	effectvalue2; //Value of Effect
	uint8	skill; //Skill to detect/disarm with rogue.
	uint8	level;
	bool	detected;
	bool	disarmed;
	uint32	respawn_time;
	uint32	respawn_var;
	uint8	triggered_number;
	uint8	times_triggered;
	uint8	group;
	bool	despawn_when_triggered;
	uint32  charid; //ID of character that triggered trap. This is cleared when the trap despawns are resets.
	bool	undetectable;

	std::string message;
protected:
	NPC *hiddenTrigger;
};
