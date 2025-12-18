#include "xtargetautohaters.h"

#include "zone/client.h"
#include "zone/groups.h"
#include "zone/mob.h"
#include "zone/raids.h"

#include <algorithm>

void XTargetAutoHaters::increment_count(Mob *in)
{
	assert(in != nullptr);
	auto it = std::find_if(m_haters.begin(), m_haters.end(),
			       [&in](const HatersCount &c) { return c.spawn_id == in->GetID(); });

	// we are on the list, we just need to increment the count
	if (it != m_haters.end()) {
		it->count++;
		return;
	}
	// We are not on the list
	HatersCount c;
	c.spawn_id = in->GetID();
	c.count = 1;

	m_haters.push_back(c);
	// trigger event on owner
	if (m_client)
		m_client->SetDirtyAutoHaters();
	else if (m_group)
		m_group->SetDirtyAutoHaters();
	else if (m_raid)
		m_raid->SetDirtyAutoHaters();
}

void XTargetAutoHaters::decrement_count(Mob *in)
{
	assert(in != nullptr);
	auto it = std::find_if(m_haters.begin(), m_haters.end(),
			       [&in](const HatersCount &c) { return c.spawn_id == in->GetID(); });

	// we are not on the list ... shouldn't happen
	if (it == m_haters.end())
		return;
	it->count--;
	if (it->count == 0) {
		m_haters.erase(it);
		if (m_client)
			m_client->SetDirtyAutoHaters();
		else if (m_group)
			m_group->SetDirtyAutoHaters();
		else if (m_raid)
			m_raid->SetDirtyAutoHaters();
	}
}

void XTargetAutoHaters::merge(XTargetAutoHaters &other)
{
	bool trigger = false;
	for (auto &e : other.m_haters) {
		auto it = std::find_if(m_haters.begin(), m_haters.end(),
				       [&e](const HatersCount &c) { return e.spawn_id == c.spawn_id; });
		if (it != m_haters.end()) {
			it->count += e.count;
			continue;
		}
		m_haters.push_back(e);
		trigger = true;
	}

	if (trigger) {
		if (m_client)
			m_client->SetDirtyAutoHaters();
		else if (m_group)
			m_group->SetDirtyAutoHaters();
		else if (m_raid)
			m_raid->SetDirtyAutoHaters();
	}
}

// demerge this from other. other belongs to group/raid you just left
void XTargetAutoHaters::demerge(XTargetAutoHaters &other)
{
	bool trigger = false;
	for (auto &e : m_haters) {
		auto it = std::find_if(other.m_haters.begin(), other.m_haters.end(),
				       [&e](const HatersCount &c) { return e.spawn_id == c.spawn_id; });
		if (it != other.m_haters.end()) {
			it->count -= e.count;
			if (it->count == 0) {
				trigger = true;
				other.m_haters.erase(it);
			}
		}
	}

	if (trigger) {
		if (other.m_client)
			other.m_client->SetDirtyAutoHaters();
		else if (other.m_group)
			other.m_group->SetDirtyAutoHaters();
		else if (other.m_raid)
			other.m_raid->SetDirtyAutoHaters();
	}
}

bool XTargetAutoHaters::contains_mob(int spawn_id)
{
	auto it = std::find_if(m_haters.begin(), m_haters.end(),
			       [spawn_id](const HatersCount &c) { return c.spawn_id == spawn_id; });
	return it != m_haters.end();
}
