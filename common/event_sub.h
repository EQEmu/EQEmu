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

#include <string>
#include <unordered_map>

class EventSubscriptionWatcher
{
public:
	~EventSubscriptionWatcher();
	
	void Subscribe(const std::string &event_name);
	void Unsubscribe(const std::string &event_name);
	bool IsSubscribed(const std::string &event_name) const;

	static EventSubscriptionWatcher *Get() {
		static EventSubscriptionWatcher* inst = nullptr;
		if(!inst) {
			inst = new EventSubscriptionWatcher();
		}
		
		return inst;
	}
private:
	EventSubscriptionWatcher() { }
	EventSubscriptionWatcher(const EventSubscriptionWatcher&);
	EventSubscriptionWatcher& operator=(const EventSubscriptionWatcher&);
	
	std::unordered_map<std::string, bool> m_subs;
};
