#pragma once

#include "common/repositories/base/base_buyer_repository.h"

#include "common/database.h"
#include "common/strings.h"
#include "base/base_buyer_trade_items_repository.h"
#include "base/base_buyer_buy_lines_repository.h"

class BuyerRepository: public BaseBuyerRepository {
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
     * BuyerRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * BuyerRepository::GetWhereNeverExpires()
     * BuyerRepository::GetWhereXAndY()
     * BuyerRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static bool UpdateWelcomeMessage(Database& db, uint32 char_id, const char *message) {

		auto const b = GetWhere(db, fmt::format("`char_id` = '{}';", char_id));

		if (b.empty()) {
			return false;
		}

		auto buyer = b.front();
		buyer.welcome_message = message;
		return UpdateOne(db, buyer);
	}

	static std::string GetWelcomeMessage(Database& db, uint32 char_id) {

		auto const b = GetWhere(db, fmt::format("`char_id` = '{}' LIMIT 1;", char_id));
		if (b.empty()) {
			return std::string();
		}

		return b.front().welcome_message;
	}

	static int UpdateTransactionDate(Database& db, uint32 char_id, time_t transaction_date) {
		auto b = GetWhere(db, fmt::format("`char_id` = '{}' LIMIT 1;", char_id));
		if (b.empty()) {
			return 0;
		}

		auto e = b.front();
		e.transaction_date = transaction_date;

		return UpdateOne(db, e);
	}

	static time_t GetTransactionDate(Database& db, uint32 char_id) {
		auto b = GetWhere(db, fmt::format("`char_id` = '{}' LIMIT 1;", char_id));
		if (b.empty()) {
			return 0;
		}

		auto e = b.front();

		return e.transaction_date;
	}

	static bool DeleteBuyer(Database &db, uint32 char_id)
	{
		if (char_id == 0) {
			Truncate(db);
			BaseBuyerBuyLinesRepository::Truncate(db);
			BaseBuyerTradeItemsRepository::Truncate(db);
		}
		else {
			auto buyer = GetWhere(db, fmt::format("`char_id` = '{}' LIMIT 1;", char_id));
			if (buyer.empty()) {
				return false;
			}

			auto buy_lines =
				BaseBuyerBuyLinesRepository::GetWhere(db, fmt::format("`buyer_id` = {}", buyer.front().id));

			std::vector<std::string> buy_line_ids{};
			for (auto const &bl: buy_lines) {
				buy_line_ids.push_back(std::to_string(bl.id));
			}

			DeleteWhere(db, fmt::format("`char_id` = '{}';", char_id));
			if (buy_line_ids.empty()) {
				return true;
			}

			BaseBuyerBuyLinesRepository::DeleteWhere(
				db, fmt::format("`id` IN({})", Strings::Implode(", ", buy_line_ids))
			);
			BaseBuyerTradeItemsRepository::DeleteWhere(
				db, fmt::format("`buyer_buy_lines_id` IN({})", Strings::Implode(", ", buy_line_ids))
			);
		}

		return true;
	}

	static bool DeleteBuyers(Database &db, uint32 char_zone_id, uint32 char_zone_instance_id)
	{
		auto buyers = GetWhere(
			db,
			fmt::format(
				"`char_zone_id` = {} AND `char_zone_instance_id` = {}", char_zone_id, char_zone_instance_id
			)
		);
		if (buyers.empty()) {
			return false;
		}

		std::vector<std::string> buyer_ids{};
		std::vector<std::string> buy_line_ids{};

		for (auto const &b: buyers) {
			buyer_ids.push_back(std::to_string(b.id));
		}

		auto buy_lines = BaseBuyerBuyLinesRepository::GetWhere(
			db, fmt::format("`buyer_id` IN({})", Strings::Implode(", ", buyer_ids))
		);

		if (!buy_lines.empty()) {
			for (auto const &bl: buy_lines) {
				buy_line_ids.push_back(std::to_string(bl.id));
			}
		}

		DeleteWhere(db, fmt::format("`id` IN({});", Strings::Implode(", ", buyer_ids)));
		if (buy_line_ids.empty()) {
			return true;
		}

		BaseBuyerBuyLinesRepository::DeleteWhere(
			db,
			fmt::format("`id` IN({})", Strings::Implode(", ", buy_line_ids))
		);
		BaseBuyerTradeItemsRepository::DeleteWhere(
			db,
			fmt::format("`buyer_buy_lines_id` IN({})", Strings::Implode(", ", buy_line_ids))
		);

		return true;
	}
};
