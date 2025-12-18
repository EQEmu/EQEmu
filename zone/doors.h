#pragma once

#include "common/repositories/doors_repository.h"
#include "zone/mob.h"

class Client;
class Mob;
class NPC;
struct Door;

class Doors : public Entity
{
public:
	~Doors();

	Doors(const char *model, const glm::vec4& position, uint8 open_type = 58, uint16 size = 100);
	Doors(const DoorsRepository::Doors& door);

	bool GetDisableTimer() { return m_disable_timer; }
	bool IsDoor() const { return true; }
	bool IsDoorOpen() { return m_is_open; }
	bool Process();
	bool triggered;
	char *GetDoorName() { return m_door_name; }
	const glm::vec4 &GetPosition() const { return m_position; }
	int GetDzSwitchID() const { return m_dz_switch_id; }
	int GetIncline() { return m_incline; }
	int GetInvertState() { return m_invert_state; }
	uint8 GetDoorID() { return m_door_id; }
	uint8 GetNoKeyring() { return m_no_key_ring; }
	uint8 GetOpenType() { return m_open_type; }
	uint8 GetTriggerDoorID() { return m_trigger_door; }
	uint8 GetTriggerType() { return m_trigger_type; }
	uint8 IsLDoNDoor() { return m_is_ldon_door; }
	uint16 GetLockpick() { return m_lockpick; }
	uint16 GetSize() { return m_size; }
	uint32 GetClientVersionMask() { return m_client_version_mask; }
	uint32 GetDoorDBID() { return m_database_id; }
	int32 GetDoorParam() { return m_door_param; }
	uint32 GetEntityID() { return m_entity_id; }
	uint32 GetGuildID() { return m_guild_id; }
	uint32 GetKeyItem() { return m_key_item_id; }
	void CreateDatabaseEntry();
	void ForceClose(Mob *sender, bool alt_mode = false);
	void ForceOpen(Mob *sender, bool alt_mode = false);
	void HandleClick(Client *sender, uint8 trigger);
	void Open(Mob *sender, bool alt_mode = false);
	void SetDisableTimer(bool flag);
	void SetDoorName(const char *name);
	void SetEntityID(uint32 entity) { m_entity_id = entity; }
	void SetIncline(int in);
	void SetInvertState(int in);
	void SetKeyItem(uint32 in) { m_key_item_id = in; }
	void SetLocation(float x, float y, float z);
	void SetLockpick(uint16 in) { m_lockpick = in; }
	void SetNoKeyring(uint8 in) { m_no_key_ring = in; }
	void SetOpenState(bool st) { m_is_open = st; }
	void SetOpenType(uint8 in);
	void SetPosition(const glm::vec4 &position);
	void SetSize(uint16 size);
	void ToggleState(Mob *sender);
	inline std::string GetDestinationZoneName() { return m_destination_zone_name; }
	inline int GetDestinationInstanceID() { return m_destination_instance_id; }
	inline float GetDestinationX() { return m_destination.x; }
	inline float GetDestinationY() { return m_destination.y; }
	inline float GetDestinationZ() { return m_destination.z; }
	inline float GetDestinationHeading() { return m_destination.w; }

	float GetX();
	float GetY();
	float GetZ();
	float GetHeading();

	bool HasDestinationZone() const;
	bool IsDestinationZoneSame() const;

	bool IsDoorBlacklisted();
	bool IsDoorBetween(glm::vec4 loc_a, glm::vec4 loc_c, uint16 door_size = 15, float door_depth = 5.0f, bool draw_box = false);

	const char* GetDoorZone() const { return m_zone_name; }

private:
	bool GetIsDoorBlacklisted();

	bool      m_has_destination_zone = false;
	bool      m_same_destination_zone = false;
	uint32    m_database_id;
	uint8     m_door_id;
	char      m_zone_name[32];
	char      m_door_name[32];
	glm::vec4 m_position;
	int       m_incline;
	uint8     m_open_type;
	uint32    m_guild_id;
	uint16    m_lockpick;
	uint32    m_key_item_id;
	uint8     m_no_key_ring;
	uint8     m_trigger_door;
	uint8     m_trigger_type;
	int32     m_door_param;
	uint16    m_size;
	int       m_invert_state;
	uint32    m_entity_id;
	bool      m_disable_timer;
	bool      m_is_open;
	Timer     m_close_timer;
	char      m_destination_zone_name[16];
	int       m_destination_instance_id;
	glm::vec4 m_destination;
	uint8     m_is_ldon_door;
	int       m_dz_switch_id = 0;
	uint32    m_client_version_mask;
	bool      m_is_blacklisted_to_open = false; // is door blacklisted to open by npcs
};
