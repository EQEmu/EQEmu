#include "zone/client.h"

void command_invsnapshot(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	if (sep->argnum == 0 || strcmp(sep->arg[1], "help") == 0) {
		std::string window_title = "Inventory Snapshot Argument Help Menu";

		std::string window_text =
						"<table>"
						"<tr>"
						"<td><c \"#FFFFFF\">Usage:</td>"
						"<td></td>"
						"<td>#invsnapshot arguments<br>(<c \"#00FF00\">required <c \"#FFFF00\">optional<c \"#FFFFFF\">)</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#FFFF00\">help</td>"
						"<td></td>"
						"<td><c \"#AAAAAA\">this menu</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#00FF00\">capture</td>"
						"<td></td>"
						"<td><c \"#AAAAAA\">takes snapshot of character inventory</td>"
						"</tr>";

		window_text.append(
			"<tr>"
			"<td><c \"#00FF00\">gcount</td>"
			"<td></td>"
			"<td><c \"#AAAAAA\">returns global snapshot count</td>"
			"</tr>"
			"<tr>"
			"<td><c \"#00FF00\">gclear</td>"
			"<td><c \"#FFFF00\"><br>now</td>"
			"<td><c \"#AAAAAA\">delete all snapshots - rule<br>delete all snapshots - now</td>"
			"</tr>"
			"<tr>"
			"<td><c \"#00FF00\">count</td>"
			"<td></td>"
			"<td><c \"#AAAAAA\">returns character snapshot count</td>"
			"</tr>"
			"<tr>"
			"<td><c \"#00FF00\">clear</td>"
			"<td><c \"#FFFF00\"><br>now</td>"
			"<td><c \"#AAAAAA\">delete character snapshots - rule<br>delete character snapshots - now</td>"
			"</tr>"
			"<tr>"
			"<td><c \"#00FF00\">list</td>"
			"<td><br><c \"#FFFF00\">count</td>"
			"<td><c \"#AAAAAA\">lists entry ids for current character<br>limits to count</td>"
			"</tr>"
			"<tr>"
			"<td><c \"#00FF00\">parse</td>"
			"<td><c \"#00FF00\">tstmp</td>"
			"<td><c \"#AAAAAA\">displays slots and items in snapshot</td>"
			"</tr>"
			"<tr>"
			"<td><c \"#00FF00\">compare</td>"
			"<td><c \"#00FF00\">tstmp</td>"
			"<td><c \"#AAAAAA\">compares inventory against snapshot</td>"
			"</tr>"
			"<tr>"
			"<td><c \"#00FF00\">restore</td>"
			"<td><c \"#00FF00\">tstmp</td>"
			"<td><c \"#AAAAAA\">restores slots and items in snapshot</td>"
			"</tr>"
		);

		window_text.append(
			"</table>"
		);

		c->SendPopupToClient(window_title.c_str(), window_text.c_str());

		return;
	}

	if (strcmp(sep->arg[1], "gcount") == 0) {
		auto is_count = database.CountInvSnapshots();
		c->Message(
			Chat::White,
			"There %s %i inventory snapshot%s.",
			(is_count == 1 ? "is" : "are"),
			is_count,
			(is_count == 1 ? "" : "s"));

		return;
	}

	if (strcmp(sep->arg[1], "gclear") == 0) {
		if (strcmp(sep->arg[2], "now") == 0) {
			database.ClearInvSnapshots(true);
			c->Message(Chat::White, "Inventory snapshots cleared using current time.");
		} else {
			database.ClearInvSnapshots();
			c->Message(
				Chat::White, "Inventory snapshots cleared using RuleI(Character, InvSnapshotHistoryD) (%i day%s).",
				RuleI(Character, InvSnapshotHistoryD), (RuleI(Character, InvSnapshotHistoryD) == 1 ? "" : "s"));
		}

		return;
	}

	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "Target must be a client.");
		return;
	}

	auto tc = (Client *) c->GetTarget();

	if (strcmp(sep->arg[1], "capture") == 0) {
		if (database.SaveCharacterInvSnapshot(tc->CharacterID())) {
			tc->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
			c->Message(
				Chat::White,
				"Successful inventory snapshot taken of %s - setting next interval for %i minute%s.",
				tc->GetName(),
				RuleI(Character, InvSnapshotMinIntervalM),
				(RuleI(Character, InvSnapshotMinIntervalM) == 1 ? "" : "s"));
		} else {
			tc->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinRetryM));
			c->Message(
				Chat::White,
				"Failed to take inventory snapshot of %s - retrying in %i minute%s.",
				tc->GetName(),
				RuleI(Character, InvSnapshotMinRetryM),
				(RuleI(Character, InvSnapshotMinRetryM) == 1 ? "" : "s"));
		}

		return;
	}

	if (strcmp(sep->arg[1], "count") == 0) {
		auto is_count = database.CountCharacterInvSnapshots(tc->CharacterID());
		c->Message(
			Chat::White,
			"%s (id: %u) has %i inventory snapshot%s.",
			tc->GetName(),
			tc->CharacterID(),
			is_count,
			(is_count == 1 ? "" : "s"));

		return;
	}

	if (strcmp(sep->arg[1], "clear") == 0) {
		if (strcmp(sep->arg[2], "now") == 0) {
			database.ClearCharacterInvSnapshots(tc->CharacterID(), true);
			c->Message(
				Chat::White,
				"%s\'s (id: %u) inventory snapshots cleared using current time.",
				tc->GetName(),
				tc->CharacterID());
		} else {
			database.ClearCharacterInvSnapshots(tc->CharacterID());
			c->Message(
				Chat::White,
				"%s\'s (id: %u) inventory snapshots cleared using RuleI(Character, InvSnapshotHistoryD) (%i day%s).",
				tc->GetName(),
				tc->CharacterID(),
				RuleI(Character, InvSnapshotHistoryD),
				(RuleI(Character, InvSnapshotHistoryD) == 1 ? "" : "s"));
		}

		return;
	}

	if (strcmp(sep->arg[1], "list") == 0) {
		std::list<std::pair<uint32, int>> is_list;
		database.ListCharacterInvSnapshots(tc->CharacterID(), is_list);

		if (is_list.empty()) {
			c->Message(Chat::White, "No inventory snapshots for %s (id: %u)", tc->GetName(), tc->CharacterID());
			return;
		}

		auto list_count = 0;
		if (sep->IsNumber(2)) {
			list_count = Strings::ToInt(sep->arg[2]);
		}
		if (list_count < 1 || list_count > is_list.size()) {
			list_count = is_list.size();
		}

		std::string window_title = StringFormat("Snapshots for %s", tc->GetName());

		std::string window_text =
						"<table>"
						"<tr>"
						"<td>Timestamp</td>"
						"<td>Entry Count</td>"
						"</tr>";

		for (auto iter: is_list) {
			if (!list_count) {
				break;
			}

			window_text.append(
				StringFormat(
					"<tr>"
					"<td>%u</td>"
					"<td>%i</td>"
					"</tr>",
					iter.first,
					iter.second
				));

			--list_count;
		}

		window_text.append(
			"</table>"
		);

		c->SendPopupToClient(window_title.c_str(), window_text.c_str());

		return;
	}

	if (strcmp(sep->arg[1], "parse") == 0) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "A timestamp is required to use this option.");
			return;
		}

		uint32 timestamp = Strings::ToUnsignedInt(sep->arg[2]);

		if (!database.ValidateCharacterInvSnapshotTimestamp(tc->CharacterID(), timestamp)) {
			c->Message(
				Chat::White,
				"No inventory snapshots for %s (id: %u) exist at %u.",
				tc->GetName(),
				tc->CharacterID(),
				timestamp
			);
			return;
		}

		std::list<std::pair<int16, uint32>> parse_list;
		database.ParseCharacterInvSnapshot(tc->CharacterID(), timestamp, parse_list);

		std::string window_title = StringFormat("Snapshot Parse for %s @ %u", tc->GetName(), timestamp);

		std::string window_text = "Slot: ItemID - Description<br>";

		for (auto iter: parse_list) {
			auto        item_data   = database.GetItem(iter.second);
			std::string window_line = StringFormat(
				"%i: %u - %s<br>",
				iter.first,
				iter.second,
				(item_data ? item_data->Name : "[error]"));

			if (window_text.length() + window_line.length() < 4095) {
				window_text.append(window_line);
			} else {
				c->Message(Chat::White, "Too many snapshot entries to list...");
				break;
			}
		}

		c->SendPopupToClient(window_title.c_str(), window_text.c_str());

		return;
	}

	if (strcmp(sep->arg[1], "compare") == 0) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "A timestamp is required to use this option.");
			return;
		}

		uint32 timestamp = Strings::ToUnsignedInt(sep->arg[2]);

		if (!database.ValidateCharacterInvSnapshotTimestamp(tc->CharacterID(), timestamp)) {
			c->Message(
				Chat::White,
				"No inventory snapshots for %s (id: %u) exist at %u.",
				tc->GetName(),
				tc->CharacterID(),
				timestamp
			);
			return;
		}

		std::list<std::pair<int16, uint32>> inv_compare_list;
		database.DivergeCharacterInventoryFromInvSnapshot(tc->CharacterID(), timestamp, inv_compare_list);

		std::list<std::pair<int16, uint32>> iss_compare_list;
		database.DivergeCharacterInvSnapshotFromInventory(tc->CharacterID(), timestamp, iss_compare_list);

		std::string window_title = StringFormat("Snapshot Comparison for %s @ %u", tc->GetName(), timestamp);

		std::string window_text = "Slot: (action) Snapshot -&gt; Inventory<br>";

		auto inv_iter = inv_compare_list.begin();
		auto iss_iter = iss_compare_list.begin();

		while (true) {
			std::string window_line;

			if (inv_iter == inv_compare_list.end() && iss_iter == iss_compare_list.end()) {
				break;
			} else if (inv_iter != inv_compare_list.end() && iss_iter == iss_compare_list.end()) {
				window_line = StringFormat("%i: (delete) [empty] -&gt; %u<br>", inv_iter->first, inv_iter->second);
				++inv_iter;
			} else if (inv_iter == inv_compare_list.end() && iss_iter != iss_compare_list.end()) {
				window_line = StringFormat("%i: (insert) %u -&gt; [empty]<br>", iss_iter->first, iss_iter->second);
				++iss_iter;
			} else {
				if (inv_iter->first < iss_iter->first) {
					window_line = StringFormat(
						"%i: (delete) [empty] -&gt; %u<br>",
						inv_iter->first,
						inv_iter->second
					);
					++inv_iter;
				} else if (inv_iter->first > iss_iter->first) {
					window_line = StringFormat(
						"%i: (insert) %u -&gt; [empty]<br>",
						iss_iter->first,
						iss_iter->second
					);
					++iss_iter;
				} else {
					window_line = StringFormat(
						"%i: (replace) %u -&gt; %u<br>",
						iss_iter->first,
						iss_iter->second,
						inv_iter->second
					);
					++inv_iter;
					++iss_iter;
				}
			}

			if (window_text.length() + window_line.length() < 4095) {
				window_text.append(window_line);
			} else {
				c->Message(Chat::White, "Too many comparison entries to list...");
				break;
			}
		}

		c->SendPopupToClient(window_title.c_str(), window_text.c_str());

		return;
	}

	if (strcmp(sep->arg[1], "restore") == 0) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "A timestamp is required to use this option.");
			return;
		}

		uint32 timestamp = Strings::ToUnsignedInt(sep->arg[2]);

		if (!database.ValidateCharacterInvSnapshotTimestamp(tc->CharacterID(), timestamp)) {
			c->Message(
				Chat::White,
				"No inventory snapshots for %s (id: %u) exist at %u.",
				tc->GetName(),
				tc->CharacterID(),
				timestamp
			);
			return;
		}

		if (database.SaveCharacterInvSnapshot(tc->CharacterID())) {
			tc->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
		} else {
			c->Message(
				Chat::Red, "Failed to take pre-restore inventory snapshot of %s (id: %u).",
				tc->GetName(), tc->CharacterID());
			return;
		}

		if (database.RestoreCharacterInvSnapshot(tc->CharacterID(), timestamp)) {
			// cannot delete all valid item slots from client..so, we worldkick
			tc->WorldKick(); // self restores update before the 'kick' is processed

			c->Message(
				Chat::White, "Successfully applied snapshot %u to %s's (id: %u) inventory.",
				timestamp, tc->GetName(), tc->CharacterID());
		} else {
			c->Message(
				Chat::Red, "Failed to apply snapshot %u to %s's (id: %u) inventory.",
				timestamp, tc->GetName(), tc->CharacterID());
		}

		return;
	}
}

