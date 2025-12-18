#pragma once

#include "common/types.h"

#include <cassert>
#include <cstddef>

class AggroMeter
{
public:
	enum AggroTypes {
		AT_Player,
		AT_Secondary,
		AT_Group1,
		AT_Group2,
		AT_Group3,
		AT_Group4,
		AT_Group5,
		AT_XTarget1,
		AT_XTarget2,
		AT_XTarget3,
		AT_XTarget4,
		AT_XTarget5,
		AT_XTarget6,
		AT_XTarget7,
		AT_XTarget8,
		AT_XTarget9,
		AT_XTarget10,
		AT_XTarget11,
		AT_XTarget12,
		AT_XTarget13,
		AT_XTarget14,
		AT_XTarget15,
		AT_XTarget16,
		AT_XTarget17,
		AT_XTarget18,
		AT_XTarget19,
		AT_XTarget20,
		AT_Max
	};

private:
	struct AggroData {
		int16 type;
		int16 pct;
	};

	AggroData data[AT_Max];
	int lock_id;			// we set this
	int target_id;			// current target or if PC targeted, their Target
	// so secondary depends on if we have aggro or not
	// When we are the current target, this will be the 2nd person on list
	// When we are not tanking, this will be the current tank
	int secondary_id;

	// so we need some easy way to detect the client changing but still delaying the packet
	bool lock_changed;
public:
	AggroMeter();
	~AggroMeter() {}

	inline void set_lock_id(int in) { lock_id = in; lock_changed = true; }
	inline bool update_lock() { bool ret = lock_changed; lock_changed = false; return ret; }
	inline void set_target_id(int in) { target_id = in; }
	inline void set_secondary_id(int in) { secondary_id = in; }
	// returns true when changed
	inline bool set_pct(AggroTypes t, int pct) { assert(t >= AT_Player && t < AT_Max); if (data[t].pct == pct) return false; data[t].pct = pct; return true; }

	inline int get_lock_id() const { return lock_id; }
	inline int get_target_id() const { return target_id; }
	inline int get_secondary_id() const { return secondary_id; }
	inline int get_pct(AggroTypes t) const { assert(t >= AT_Player && t < AT_Max); return data[t].pct; }
	// the ID of the spawn for player entry depends on lock_id
	inline int get_player_aggro_id() const { return lock_id ? lock_id : target_id; }
	// fuck it, lets just use a buffer the size of the largest to work with
	const inline size_t max_packet_size() const { return sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + (sizeof(uint8) + sizeof(uint16)) * AT_Max; }
};
