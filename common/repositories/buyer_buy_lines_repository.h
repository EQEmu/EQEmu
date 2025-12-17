#pragma once

#include "common/repositories/base/base_buyer_buy_lines_repository.h"

#include "common/database.h"
#include "common/eq_packet_structs.h"
#include "common/repositories/buyer_repository.h"
#include "common/repositories/buyer_trade_items_repository.h"
#include "common/repositories/character_data_repository.h"
#include "common/strings.h"

class BuyerBuyLinesRepository: public BaseBuyerBuyLinesRepository {
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
	 * BuyerBuyLinesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * BuyerBuyLinesRepository::GetWhereNeverExpires()
	 * BuyerBuyLinesRepository::GetWhereXAndY()
	 * BuyerBuyLinesRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */

	// Custom extended repository methods here

	struct WelcomeData_Struct {
		uint32 count_of_buyers;
		uint32 count_of_items;
	};

	static int CreateBuyLine(Database& db, const BuyerLineItems_Struct b, uint32 char_id)
	{
		auto buyer = BuyerRepository::GetWhere(db, fmt::format("`char_id` = '{}' LIMIT 1", char_id));
		if (buyer.empty()){
			return 0;
		}

		BuyerBuyLinesRepository::BuyerBuyLines buy_lines{};
		buy_lines.id          = 0;
		buy_lines.buyer_id    = buyer.front().id;
		buy_lines.char_id     = char_id;
		buy_lines.buy_slot_id = b.slot;
		buy_lines.item_id     = b.item_id;
		buy_lines.item_name   = b.item_name;
		buy_lines.item_icon   = b.item_icon;
		buy_lines.item_qty    = b.item_quantity;
		buy_lines.item_price  = b.item_cost;
		auto e = InsertOne(db, buy_lines);

		std::vector<BuyerTradeItemsRepository::BuyerTradeItems> queue;

		for (auto const &r: b.trade_items) {
			BuyerTradeItemsRepository::BuyerTradeItems bti{};
			bti.id                 = 0;
			bti.buyer_buy_lines_id = e.id;
			bti.item_id            = r.item_id;
			bti.item_qty           = r.item_quantity;
			bti.item_icon          = r.item_icon;
			bti.item_name          = r.item_name;

			if (bti.item_id) {
				queue.push_back(bti);
			}
		}

		if (!queue.empty()) {
			BuyerTradeItemsRepository::InsertMany(db, queue);
		}

		return e.id;
	}

	static int ModifyBuyLine(Database& db, const BuyerLineItems_Struct b, uint32 char_id)
	{
		auto b_lines = GetWhere(db, fmt::format("`char_id` = '{}' AND `buy_slot_id` = '{}';", char_id, b.slot));
		if (b_lines.empty() || b_lines.size() > 1){
			return 0;
		}

		auto b_line = b_lines.front();

		b_line.item_qty   = b.item_quantity;
		b_line.item_price = b.item_cost;
		b_line.item_icon  = b.item_icon;
		auto e = UpdateOne(db, b_line);

		std::vector<BuyerTradeItemsRepository::BuyerTradeItems> queue;

		BuyerTradeItemsRepository::DeleteWhere(db, fmt::format("`buyer_buy_lines_id` = '{}';", b_line.id));
		for (auto const &r: b.trade_items) {
			BuyerTradeItemsRepository::BuyerTradeItems bti{};
			bti.id                 = 0;
			bti.buyer_buy_lines_id = b_line.id;
			bti.item_id            = r.item_id;
			bti.item_qty           = r.item_quantity;
			bti.item_icon          = r.item_icon;
			bti.item_name          = r.item_name;

			if (bti.item_id) {
				queue.push_back(bti);
			}
		}

		if (!queue.empty()) {
			BuyerTradeItemsRepository::InsertMany(db, queue);
		}

		return 1;
	}

	static bool DeleteBuyLine(Database &db, uint32 char_id, int32 slot_id = 0xffffffff)
	{
		std::vector<BuyerBuyLines> buylines{};
		if (slot_id == 0xffffffff) {
			auto buylines = GetWhere(db, fmt::format("`char_id` = '{}'", char_id));
			DeleteWhere(db, fmt::format("`char_id` = '{}'", char_id));
		}
		else {
			auto buylines = GetWhere(db, fmt::format("`char_id` = '{}' AND `buy_slot_id` = '{}'", char_id, slot_id));
			DeleteWhere(db, fmt::format("`char_id` = '{}' AND `buy_slot_id` = '{}'", char_id, slot_id));
		}

		if (buylines.empty()) {
			return 0;
		}

		std::vector<std::string> buyline_ids{};
		for (auto const          &bl: buylines) {
			buyline_ids.push_back((std::to_string(bl.id)));
		}

		if (!buyline_ids.empty()) {
			BuyerTradeItemsRepository::DeleteWhere(
				db,
				fmt::format(
					"`buyer_buy_lines_id` IN({})",
					Strings::Implode(", ", buyline_ids)
				)
			);
		}

		return 1;
	}

	static std::vector<BuyerLineItems_Struct> GetBuyLines(Database &db, uint32 char_id)
	{
		std::vector<BuyerLineItems_Struct> all_entries{};

		auto  buy_line             = GetWhere(db, fmt::format("`char_id` = '{}';", char_id));
		if (buy_line.empty()){
			return all_entries;
		}

		auto  buy_line_trade_items = BuyerTradeItemsRepository::GetWhere(
			db,
			fmt::format(
				"`buyer_buy_lines_id` IN (SELECT b.id FROM buyer_buy_lines AS b WHERE b.char_id = '{}')",
				char_id
			)
		);

		all_entries.reserve(buy_line.size());

		for (auto const &l: buy_line) {
			BuyerLineItems_Struct bli{};
			bli.item_id       = l.item_id;
			bli.item_cost     = l.item_price;
			bli.item_quantity = l.item_qty;
			bli.slot          = l.buy_slot_id;
			bli.item_name     = l.item_name;

			for (auto const &i: GetSubIDs(buy_line_trade_items, l.id)) {
				BuyerLineTradeItems_Struct blti{};
				blti.item_id       = i.item_id;
				blti.item_icon     = i.item_icon;
				blti.item_quantity = i.item_qty;
				blti.item_id       = i.item_id;
				blti.item_name     = i.item_name;
				bli.trade_items.push_back(blti);
			}
			all_entries.push_back(bli);
		}

		return all_entries;
	}

	static BuyerLineSearch_Struct SearchBuyLines(
		Database &db,
		std::string &search_string,
		uint32 char_id = 0,
		uint32 zone_id = 0,
		uint32 zone_instance_id = 0
	)
	{
		BuyerLineSearch_Struct all_entries{};
		std::string where_clause(fmt::format("`item_name` LIKE \"%{}%\" ", search_string));

		if (char_id) {
			where_clause += fmt::format("AND `char_id` = '{}' ", char_id);
		}

		if (zone_id) {
			auto buyers = BuyerRepository::GetWhere(
				db,
				fmt::format(
					"`char_zone_id` = '{}' AND char_zone_instance_id = '{}'",
					zone_id,
					zone_instance_id
				)
			);

			if (buyers.empty()) {
				return all_entries;
			}

			std::vector<std::string> char_ids{};
			for (auto const &bl : buyers) {
				char_ids.push_back((std::to_string(bl.char_id)));
			}

			where_clause += fmt::format("AND `char_id` IN ({}) ", Strings::Implode(", ", char_ids));
		}

		auto buy_line = GetWhere(db, where_clause);
		if (buy_line.empty()){
			return all_entries;
		}

		std::vector<std::string> ids{};
		std::vector<std::string> char_ids{};
		for (auto const &bl : buy_line) {
			if (std::find(ids.begin(), ids.end(), std::to_string(bl.id)) == std::end(ids)) {
				ids.push_back(std::to_string(bl.id));
			}
			if (std::find(char_ids.begin(), char_ids.end(), std::to_string(bl.char_id)) == std::end(char_ids)) {
				char_ids.push_back((std::to_string(bl.char_id)));
			}
		}

		auto buy_line_trade_items = BuyerTradeItemsRepository::GetWhere(
			db,
			fmt::format(
				"`buyer_buy_lines_id` IN ({});",
				Strings::Implode(", ", ids)
			)
		);

		auto char_names = BuyerRepository::GetWhere(
			db,
			fmt::format(
				"`char_id` IN ({});",
				Strings::Implode(", ", char_ids)
			)
		);

		all_entries.no_items = buy_line.size();
		for (auto const &l: buy_line) {
			BuyerLineItemsSearch_Struct blis{};
			blis.slot          = l.buy_slot_id;
			blis.item_id       = l.item_id;
			blis.item_cost     = l.item_price;
			blis.item_icon     = l.item_icon;
			blis.item_quantity = l.item_qty;
			blis.buyer_id      = l.char_id;
			auto it = std::find_if(
				char_names.cbegin(),
				char_names.cend(),
				[&](BuyerRepository::Buyer e) { return e.char_id == l.char_id; }
			);
			blis.buyer_name             = it != char_names.end() ? it->char_name : std::string("");
			blis.buyer_entity_id        = it != char_names.end() ? it->char_entity_id : 0;
			blis.buyer_zone_id          = it != char_names.end() ? it->char_zone_id : 0;
			blis.buyer_zone_instance_id = it != char_names.end() ? it->char_zone_instance_id : 0;
			strn0cpy(blis.item_name, l.item_name.c_str(), sizeof(blis.item_name));

			for (auto const &i: GetSubIDs(buy_line_trade_items, l.id)) {
				BuyerLineTradeItems_Struct e{};
				e.item_id       = i.item_id;
				e.item_icon     = i.item_icon;
				e.item_quantity = i.item_qty;
				e.item_id       = i.item_id;
				e.item_name     = i.item_name;

				blis.trade_items.push_back(e);
			}
			all_entries.buy_line.push_back(blis);
		}

		return all_entries;
	}

	static std::vector<BuyerTradeItemsRepository::BuyerTradeItems>
	GetSubIDs(std::vector<BuyerTradeItemsRepository::BuyerTradeItems> &in, uint64 id)
	{
		std::vector<BuyerTradeItemsRepository::BuyerTradeItems> results{};
		std::vector<uint64>                                     indices{};

		auto it = in.begin();
		while ((it = std::find_if(
			it,
			in.end(),
			[&](BuyerTradeItemsRepository::BuyerTradeItems const &e) {
				return e.buyer_buy_lines_id == id;
			}
		))
			   != in.end()
			) {
			indices.push_back(std::distance(in.begin(), it));
			results.push_back(*it);
			it++;
		}
		return results;
	}

	static WelcomeData_Struct GetWelcomeData(Database &db)
	{
		WelcomeData_Struct e{};

		auto results = db.QueryDatabase("SELECT COUNT(DISTINCT char_id), COUNT(char_id) FROM buyer;");

		if (!results.RowCount()) {
			return e;
		}

		auto r = results.begin();
		e.count_of_buyers = Strings::ToInt(r[0]);
		e.count_of_items  = Strings::ToInt(r[1]);
		return e;
	}

};
