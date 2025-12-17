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
