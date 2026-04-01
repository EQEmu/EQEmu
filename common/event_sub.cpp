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
#include "event_sub.h"

void EventSubscriptionWatcher::Subscribe(const std::string &event_name)
{
	m_subs[event_name] = 1;
}

void EventSubscriptionWatcher::Unsubscribe(const std::string &event_name)
{
	m_subs[event_name] = 0;
}

bool EventSubscriptionWatcher::IsSubscribed(const std::string &event_name) const
{
	auto iter = m_subs.find(event_name);
	if (iter != m_subs.end()) {
		return iter->second;
	}

	return false;
}
