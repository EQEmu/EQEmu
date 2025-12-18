#include "common/repositories/bug_reports_repository.h"
#include "zone/client.h"

void command_bugs(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #bugs close [Bug ID] - Close a Bug Report by ID");
		c->Message(Chat::White, "Usage: #bugs delete [Bug ID] - Delete a Bug Report by ID");
		c->Message(Chat::White, "Usage: #bugs review [Bug ID] [Review] - Review a Bug Report by ID");
		c->Message(Chat::White, "Usage: #bugs search [Search Criteria] - Search for Bug Reports");
		c->Message(Chat::White, "Usage: #bugs view [Bug ID] - View a Bug Report by ID");
		return;
	}

	bool is_close = !strcasecmp(sep->arg[1], "close");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_review = !strcasecmp(sep->arg[1], "review");
	bool is_search = !strcasecmp(sep->arg[1], "search");
	bool is_view = !strcasecmp(sep->arg[1], "view");
	if (
		!is_close &&
		!is_delete &&
		!is_review &&
		!is_search &&
		!is_view
	) {
		c->Message(Chat::White, "Usage: #bugs close [Bug ID] - Close a Bug Report by ID");
		c->Message(Chat::White, "Usage: #bugs delete [Bug ID] - Delete a Bug Report by ID");
		c->Message(Chat::White, "Usage: #bugs review [Bug ID] [Review] - Review a Bug Report by ID");
		c->Message(Chat::White, "Usage: #bugs search [Search Criteria] - Search for Bug Reports");
		c->Message(Chat::White, "Usage: #bugs view [Bug ID] - View a Bug Report by ID");
		return;
	}

	if (is_close) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #bugs close [Bug ID] - Close a Bug Report by ID");
			return;
		}

		auto bug_id = Strings::ToUnsignedInt(sep->arg[2]);

		auto r = BugReportsRepository::FindOne(database, bug_id);
		if (!r.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug ID {} does not exist or is invalid.",
					bug_id
				).c_str()
			);
			return;
		}

		r.bug_status = 1;

		if (!BugReportsRepository::UpdateOne(database, r)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to close Bug ID {}.",
					bug_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Successfully closed Bug ID {}.",
				bug_id
			).c_str()
		);

	} else if (is_delete) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #bugs delete [Bug ID] - Delete a Bug Report by ID");
			return;
		}

		auto bug_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto deleted_count = BugReportsRepository::DeleteOne(database, bug_id);
		if (!deleted_count) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug ID {} does not exist or is invalid.",
					bug_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} successfully deleted.",
				bug_id
			).c_str()
		);
	} else if (is_review) {
		if (
			arguments < 3 ||
			!sep->IsNumber(2)
		) {
			c->Message(Chat::White, "Usage: #bugs review [Bug ID] [Review] - Review a Bug Report by ID");
			return;
		}

		auto bug_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto bug_review = sep->argplus[3];

		auto r = BugReportsRepository::FindOne(database, bug_id);
		if (!r.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug ID {} does not exist or is invalid.",
					bug_id
				).c_str()
			);
			return;
		}

		r.last_review = std::time(nullptr);
		r.last_reviewer = c->GetCleanName();
		r.reviewer_notes = bug_review;

		if (!BugReportsRepository::UpdateOne(database, r)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to add a review on Bug ID {}.",
					bug_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Successfully added a review on Bug ID {}.",
				bug_id
			).c_str()
		);
	} else if (is_search) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #bugs search [Search Criteria] - Search for Bug Reports");
			return;
		}

		auto search_criteria = sep->argplus[2];
		auto l = BugReportsRepository::GetWhere(
			database,
			fmt::format(
				"bug_status = 0 AND (character_name LIKE '%%{}%%' OR bug_report LIKE '%%{}%%')",
				Strings::Escape(search_criteria),
				Strings::Escape(search_criteria)
			)
		);
		if (l.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"No Bug Reports were found matching '{}'.",
					search_criteria
				).c_str()
			);
			return;
		}

		for (const auto& r : l) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug ID {} | Character: {} Report: {} | {} | {}",
					r.id,
					r.character_name,
					r.bug_report,
					Saylink::Silent(
						fmt::format(
							"#bugs view {}",
							r.id
						),
						"View"
					),
					Saylink::Silent(
						fmt::format(
							"#bugs close {}",
							r.id
						),
						"Close"
					)
				).c_str()
			);
		}
	} else if (is_view) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #bugs view [Bug ID] - View a Bug Report by ID");
			return;
		}

		auto bug_id = Strings::ToUnsignedInt(sep->arg[2]);

		auto r = BugReportsRepository::FindOne(database, bug_id);
		if (!r.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug ID {} does not exist or is invalid.",
					bug_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | Character: {} ({}) Category: {} ({})",
				r.id,
				r.character_name,
				r.character_id,
				r.category_name,
				r.category_id
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | Zone: {} ({})",
				r.id,
				ZoneLongName(ZoneID(r.zone)),
				r.zone
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | Position: {:.2f}, {:.2f}, {:.2f}, {}",
				r.id,
				r.pos_x,
				r.pos_y,
				r.pos_z,
				r.heading
			).c_str()
		);

		if (r._target_info) {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug ID {} | Target: {} ({})",
					r.id,
					r.target_name,
					r.target_id
				).c_str()
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | Can Duplicate: {} Crash Bug: {}",
				r.id,
				r._can_duplicate ? "Yes" : "No",
				r._crash_bug ? "Yes" : "No"
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | Character Flags: {} Unknown Value: {}",
				r.id,
				r._character_flags ? "Yes" : "No",
				r._unknown_value ? "Yes" : "No"
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | Report: {} Reported: {}",
				r.id,
				r.bug_report,
				r.report_datetime
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | Client: {} ({}) UI Path: {}",
				r.id,
				r.client_version_name,
				r.client_version_id,
				r.ui_path
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Bug ID {} | System Info: {}",
				r.id,
				r.system_info
			).c_str()
		);

		if (r.last_reviewer != "None") {
			c->Message(
				Chat::White,
				fmt::format(
					"Bug ID {} | Last Reviewer: {} Last Reviewed: {} Reviewer Notes: {}",
					r.id,
					r.last_reviewer,
					r.last_review,
					r.reviewer_notes
				).c_str()
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Would you like to {} this bug?",
				Saylink::Silent(
					fmt::format(
						"#bugs close {}",
						r.id
					),
					"close"
				)
			).c_str()
		);
	}
}
