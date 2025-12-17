#pragma once

#include "common/repositories/base/base_buyer_trade_items_repository.h"

#include "common/database.h"
#include "common/strings.h"

class BuyerTradeItemsRepository: public BaseBuyerTradeItemsRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * BuyerTradeItemsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * BuyerTradeItemsRepository::GetWhereNeverExpires()
     * BuyerTradeItemsRepository::GetWhereXAndY()
     * BuyerTradeItemsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static std::vector<BuyerTradeItems> GetTradeItems(Database& db, const uint32 char_id)
	{
		std::vector<BuyerTradeItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT bti.* "
				"FROM buyer_trade_items AS bti "
				"INNER JOIN buyer_buy_lines AS bbl ON bti.buyer_buy_lines_id = bbl.id "
				"WHERE bbl.char_id = '{}';",
				char_id
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BuyerTradeItems e{};

			e.id                 = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.buyer_buy_lines_id = row[1] ? strtoull(row[1], nullptr, 10) : 0;
			e.item_id            = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.item_qty           = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.item_icon          = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.item_name          = row[5] ? row[5] : "0";

			all_entries.push_back(e);
		}

		return all_entries;
	}
};
