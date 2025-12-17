#pragma once

#include "common/item_instance.h"
#include "common/shareddb.h"

#include <vector>

class Bazaar {
public:
	static std::vector<BazaarSearchResultsFromDB_Struct>
	GetSearchResults(Database &content_db, Database &db, BazaarSearchCriteria_Struct search, unsigned int char_zone_id, int char_zone_instance_id);

};
