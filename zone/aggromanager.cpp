#include "aggromanager.h"

AggroMeter::AggroMeter() : lock_id(0), target_id(0), secondary_id(0), lock_changed(false)
{
	for (int i = 0; i < AT_Max; ++i) {
		data[i].type = i;
		data[i].pct = 0;
	}
}
