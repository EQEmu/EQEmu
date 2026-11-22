#include "zone/collision_observation.h"
#include <cassert>
int main() {
	using namespace CollisionObservation;
	const glm::vec3 point(1, 2, 3);
	Query("unscoped", point, point, 0);
	{
		Scope outer("outer", 2011, 123);
		Query("first", point, point, 1);
		{
			Scope rejected("rejected", 2012, 456);
			Query("rejected", point, point, 0);
		}
		Query("restored", point, point, 2);
		{
			Scope nested("nested", 2011, 789);
			Query("nested", point, point, 3);
		}
		// Exhaust the real budget and try to overrun it.
		for (unsigned i = 0; i < 4100; ++i) Query("budget", point, point, 4);
	}
	Query("after_scope", point, point, 0);
	assert(caller == nullptr && entity == 0);
}
