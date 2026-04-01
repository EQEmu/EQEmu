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
#include "dialogue_window.h"

void DialogueWindow::Render(Client *c, std::string markdown)
{
	std::string output = markdown;

	if (!c->ClientDataLoaded()) {
		return;
	}

	// this is the NPC that the client is interacting with if there is dialogue going on
	Mob *target = c->GetTarget() ? c->GetTarget() : c;

	// zero this out
	c->SetEntityVariable(DIAWIND_RESPONSE_ONE_KEY, "");
	c->SetEntityVariable(DIAWIND_RESPONSE_TWO_KEY, "");

	// simple find and replace for the markdown
	Strings::FindReplace(output, "~", "</c>");
	Strings::FindReplace(output, "{y}", "<c \"#CCFF33\">");
	Strings::FindReplace(output, "{lb}", "<c \"#00FFFF\">");
	Strings::FindReplace(output, "{r}", "<c \"#FF0000\">");
	Strings::FindReplace(output, "{g}", "<c \"#00FF00\">");
	Strings::FindReplace(output, "{gold}", "<c \"#FFFF66\">");
	Strings::FindReplace(output, "{orange}", "<c \"#FFA500\">");
	Strings::FindReplace(output, "{gray}", "<c \"#808080\">");
	Strings::FindReplace(output, "{tan}", "<c \"#daa520\">");
	Strings::FindReplace(output, "{bullet}", "•");
	Strings::FindReplace(output, "{name}", fmt::format("{}", c->GetCleanName()));
	Strings::FindReplace(output, "{linebreak}", "--------------------------------------------------------------------");
	Strings::FindReplace(output, "{rowpad}", R"(<tr><td>{tdpad}<"td><td>{tdpad}<"td><"tr>)");
	Strings::FindReplace(output, "{tdpad}", "----------------------");
	Strings::FindReplace(output, "{in}", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");

	// mysterious voice
	bool render_mysterious_voice = false;
	if (markdown.find("{mysterious}") != std::string::npos) {
		render_mysterious_voice = true;
		LogDiaWind("Client [{}] Rendering mysterious voice", c->GetCleanName());
		Strings::FindReplace(output, "{mysterious}", "");
	}

	// noquotes
	bool render_noquotes = false;
	if (markdown.find("noquotes") != std::string::npos) {
		render_noquotes = true;
		LogDiaWind("Client [{}] Rendering noquotes", c->GetCleanName());
		Strings::FindReplace(output, "noquotes", "");
	}

	// nobracket
	bool render_nobracket = false;
	if (markdown.find("nobracket") != std::string::npos) {
		render_nobracket = true;
		LogDiaWind("Client [{}] Rendering nobracket", c->GetCleanName());
		Strings::FindReplace(output, "nobracket", "");
	}

	bool render_hiddenresponse = false;
	if (markdown.find("hiddenresponse") != std::string::npos) {
		render_hiddenresponse = true;
		LogDiaWind("Client [{}] Rendering hiddenresponse", c->GetCleanName());
		Strings::FindReplace(output, "hiddenresponse", "");
	}

	// animations
	std::string animation = Strings::GetBetween(output, "+", "+");
	if (!animation.empty()) {
		LogDiaWind("Client [{}] Animation is not empty, contents are [{}]", c->GetCleanName(), animation);

		// we treat the animation field differently if it is a number
		bool found_animation = false;
		if (Strings::IsNumber(animation)) {
			LogDiaWindDetail("Client [{}] Animation is a number, firing animation [{}]", c->GetCleanName(), animation);
			target->DoAnim(Strings::ToInt(animation));
			found_animation = true;
		}
		else {
			for (auto &a: animations) {
				if (a.first.find(Strings::ToLower(animation)) != std::string::npos) {
					LogDiaWindDetail(
						"Client [{}] Animation is a string, firing animation [{}] [{}]",
						c->GetCleanName(),
						a.second,
						a.first
					);
					target->DoAnim(a.second);
					found_animation = true;
				}
			}
		}

		if (found_animation) {
			Strings::FindReplace(output, fmt::format("+{}+", animation), "");
		}
	}

	if (animation.empty() && RuleB(Chat, DialogueWindowAnimatesNPCsIfNoneSet)) {
		std::vector<int> greet_animations = {
			29, // wave
			48, // nodyes
			64, // point
			67, // salute
			69, // tapfoot
			70, // bowto
		};

		int random_animation = rand() % (greet_animations.size() - 1) + 0;

		target->DoAnim(greet_animations[random_animation]);
	}

	// window expire time
	std::string expire_time           = Strings::GetBetween(output, "=", "=");
	uint32      window_expire_seconds = 0;
	if (!expire_time.empty()) {
		LogDiaWind("Client [{}] Window expire time is not empty, contents are [{}]", c->GetCleanName(), expire_time);
		Strings::FindReplace(output, fmt::format("={}=", expire_time), "");

		// we treat the animation field differently if it is a number
		if (Strings::IsNumber(expire_time)) {
			LogDiaWindDetail(
				"Client [{}] Window expire time is a number, setting expiration to [{}]",
				c->GetCleanName(),
				expire_time
			);
			window_expire_seconds = Strings::ToInt(expire_time);
		}
	}

	uint32      popup_id       = POPUPID_DIAWIND_ONE;
	uint32      negative_id    = POPUPID_DIAWIND_TWO;
	std::string button_one_name;
	std::string button_two_name;
	uint32      sound_controls = 0;

	// window type
	std::string wintype;
	if (markdown.find("wintype:") != std::string::npos) {
		LogDiaWind("Client [{}] Rendering wintype option", c->GetCleanName());

		auto first_split = Strings::Split(output, "wintype:");
		if (!first_split.empty()) {

			// assumed that there is more after the wintype declaration
			// wintype:0 +animation+ etc.
			auto second_split = Strings::Split(first_split[1], " ");
			if (!second_split.empty()) {
				wintype = second_split[0];
				LogDiaWindDetail("Client [{}] Rendering wintype option wintype [{}]", c->GetCleanName(), wintype);
			}

			// if we're dealing with a string that is at the end
			// example wintype:0");
			if (first_split[1].length() == 1) {
				wintype = first_split[1];
				LogDiaWindDetail(
					"Client [{}] Rendering wintype (end) option wintype [{}]",
					c->GetCleanName(),
					wintype
				);
			}

			Strings::FindReplace(output, fmt::format("wintype:{}", wintype), "");
		}
	}

	// popupid
	std::string popupid;
	if (markdown.find("popupid:") != std::string::npos) {
		LogDiaWind("Client [{}] Rendering popupid option", c->GetCleanName());

		auto first_split = Strings::Split(output, "popupid:");
		if (!first_split.empty()) {

			// assumed that there is more after the popupid declaration
			// popupid:0 +animation+ etc.
			auto second_split = Strings::Split(first_split[1], " ");
			if (!second_split.empty()) {
				popupid = second_split[0];
				LogDiaWindDetail("Client [{}] Rendering popupid option popupid [{}]", c->GetCleanName(), popupid);
			}

			// if we're dealing with a string that is at the end
			// example popupid:0");
			if (first_split[1].length() == 1) {
				popupid = first_split[1];
				LogDiaWindDetail(
					"Client [{}] Rendering popupid (end) option popupid [{}]",
					c->GetCleanName(),
					popupid
				);
			}

			Strings::FindReplace(output, fmt::format("popupid:{}", popupid), "");

			// set the popup id
			if (!popupid.empty()) {
				popup_id = (Strings::IsNumber(popupid) ? Strings::ToInt(popupid) : 0);
			}
		}
	}

	// secondresponseid
	std::string secondresponseid;
	if (markdown.find("secondresponseid:") != std::string::npos) {
		LogDiaWind("Client [{}] Rendering secondresponseid option", c->GetCleanName());

		auto first_split = Strings::Split(output, "secondresponseid:");
		if (!first_split.empty()) {
			auto second_split = Strings::Split(first_split[1], " ");
			if (!second_split.empty()) {
				secondresponseid = second_split[0];
				LogDiaWindDetail("Client [{}] Rendering secondresponseid option secondresponseid [{}]",
								 c->GetCleanName(),
								 secondresponseid);
			}

			if (first_split[1].length() == 1) {
				secondresponseid = first_split[1];
				LogDiaWindDetail(
					"Client [{}] Rendering secondresponseid (end) option secondresponseid [{}]",
					c->GetCleanName(),
					secondresponseid
				);
			}

			Strings::FindReplace(output, fmt::format("secondresponseid:{}", secondresponseid), "");

			if (!secondresponseid.empty()) {
				negative_id = (Strings::IsNumber(secondresponseid) ? Strings::ToInt(secondresponseid) : 0);
			}
		}
	}

	// Buttons Text
	std::string button_one;
	std::string button_two;
	if (
		markdown.find("{button_one:") != std::string::npos &&
		markdown.find("{button_two:") != std::string::npos
		) {

		LogDiaWind("Client [{}] Rendering button_one option.", c->GetCleanName());

		button_one = Strings::GetBetween(output, "{button_one:", "}");
		LogDiaWind("Client [{}] button_one [{}]", c->GetCleanName(), button_one);

		if (!button_one.empty()) {
			Strings::FindReplace(output, fmt::format("{{button_one:{}}}", button_one), "");
			button_one_name = Strings::Trim(button_one);
		}

		button_two = Strings::GetBetween(output, "{button_two:", "}");
		LogDiaWind("Client [{}] button_two [{}]", c->GetCleanName(), button_two);

		if (!button_two.empty()) {
			Strings::FindReplace(output, fmt::format("{{button_two:{}}}", button_two), "");
			button_two_name = Strings::Trim(button_two);
		}

		LogDiaWind(
			"Client [{}] Rendering buttons button_one [{}] button_two [{}]",
			c->GetCleanName(),
			button_one,
			button_two
		);
	}

	// bracket responses
	std::vector<std::string> responses;
	std::vector<std::string> bracket_responses;
	if (markdown.find('[') != std::string::npos && markdown.find(']') != std::string::npos) {

		// record any saylinks that may be in saylink form
		std::string                        strip_saylinks = output;
		std::map<std::string, std::string> replacements   = {};
		while (strip_saylinks.find('[') != std::string::npos && strip_saylinks.find(']') != std::string::npos) {
			std::string bracket_message = Strings::GetBetween(strip_saylinks, "[", "]");

			// strip saylinks and normalize to [regular message]
			size_t link_open  = bracket_message.find('\x12');
			size_t link_close = bracket_message.find_last_of('\x12');
			if (link_open != link_close && (bracket_message.length() - link_open) > EQ::constants::SAY_LINK_BODY_SIZE) {
				replacements.emplace(
					std::pair<std::string, std::string>(
						bracket_message,
						bracket_message.substr(EQ::constants::SAY_LINK_BODY_SIZE + 1)
					)
				);
			}

			Strings::FindReplace(strip_saylinks, fmt::format("[{}]", bracket_message), "");
		}

		// write replacement strips
		for (auto &replacement: replacements) {
			Strings::FindReplace(output, replacement.first, replacement.second.substr(0, replacement.second.size() - 1));
		}

		// copy
		std::string content = output;

		// while brackets still exist
		int response_index = 0;
		while (content.find('[') != std::string::npos && content.find(']') != std::string::npos) {
			std::string bracket_message = Strings::GetBetween(content, "[", "]");

			LogDiaWindDetail(
				"Client [{}] Rendering responses ({}) [{}]",
				c->GetCleanName(),
				response_index,
				bracket_message
			);

			// pop message onto responses
			responses.emplace_back(bracket_message);

			// pop the response off of the message
			Strings::FindReplace(content, fmt::format("[{}]", bracket_message), "");

			// too many iterations / safety net
			if (response_index > 100) {
				break;
			}

			response_index++;
		}
	}

	// build saylinks
	if (responses.size() > 1) {
		for (auto &r: responses) {
			bracket_responses.emplace_back(
				fmt::format("[{}]", Saylink::Create(r))
			);
		}
	}

	// Placed here to allow silent message or other message to override default for custom values.
	if (!button_one_name.empty() && !button_two_name.empty()) {
		c->SetEntityVariable(DIAWIND_RESPONSE_ONE_KEY, button_one_name);
		c->SetEntityVariable(DIAWIND_RESPONSE_TWO_KEY, button_two_name);
	}

	// handle silent prompts from the [> silent syntax
	std::string silent_message;
	if (responses.empty() && markdown.find('[') != std::string::npos && markdown.find('>') != std::string::npos) {
		silent_message = Strings::GetBetween(output, "[", ">");

		// temporary and used during the response
		c->SetEntityVariable(DIAWIND_RESPONSE_ONE_KEY, silent_message);

		// pop the silent message off
		Strings::FindReplace(output, fmt::format("[{}>", silent_message), "");
	}
	else if (!responses.empty()) {
		// handle silent prompts from the single respond bracket syntax []
		silent_message = responses[0];

		// temporary and used during the response
		c->SetEntityVariable(DIAWIND_RESPONSE_ONE_KEY, silent_message);

		// pop the silent message off
		Strings::FindReplace(output, fmt::format("[{}]", silent_message), "");
	}

	// strip brackets
	if (render_nobracket) {
		Strings::FindReplace(output, "[", "");
		Strings::FindReplace(output, "]", "");
	}

	// render title
	std::string title;
	std::string speaking;
	if (target) {
		speaking = fmt::format("{} says", target->GetCleanName());
	}

	if (render_mysterious_voice) {
		speaking = "A Mysterious Voice says";
	}

	// title
	std::string popup_title;
	if (markdown.find("{title:") != std::string::npos) {
		popup_title = Strings::GetBetween(output, "{title:", "}");

		LogDiaWind("Client [{}] Rendering title option title [{}]", c->GetCleanName(), popup_title);

		if (!popup_title.empty()) {
			Strings::FindReplace(output, fmt::format("{{title:{}}}", popup_title), "");
			title = Strings::Trim(popup_title);
		}
	}

	if (title.empty()) {
		title = fmt::format("Dialogue [{}]", speaking);
	}

	// render quotes
	std::string quote_string = "'";
	if (render_noquotes) {
		quote_string = "";
	}

	// click response
	// window type response
	uint32      window_type           = (Strings::IsNumber(wintype) ? Strings::ToInt(wintype) : 0);
	std::string click_response_button = (window_type == 1 ? "Yes" : "OK");
	std::string click_response        = fmt::format(
		"<c \"#F07F00\">Click [{}] to continue...</c>",
		click_response_button
	);

	// different response when a timer is set
	if (window_expire_seconds > 0) {
		click_response = fmt::format(
			"<c \"#F07F00\">This message will disappear in {} second(s)...</c>",
			window_expire_seconds
		);
	}

	// respond with silent message
	if (!silent_message.empty()) {
		click_response = fmt::format(
			"<c \"#F07F00\">Click [{}] to respond with [{}]...</c>",
			click_response_button,
			silent_message
		);
	}

	if (!button_one_name.empty() && !button_two_name.empty()) {
		click_response = fmt::format(
			"<c \"#F07F00\">Click [{}] to respond with [{}]...<br>"
			"Click [{}] to respond with [{}]...</c>",
			button_one_name,
			button_one_name,
			button_two_name,
			button_two_name
		);
	}

	// post processing of color markdowns
	// {spring_green_1} = <c "#5EFB6E">
	if (markdown.find('{') != std::string::npos && markdown.find('}') != std::string::npos) {

		// while brackets still exist
		int tag_index = 0;
		while (output.find('{') != std::string::npos && output.find('}') != std::string::npos) {
			std::string color_tag = Strings::GetBetween(output, "{", "}");

			LogDiaWindDetail(
				"Client [{}] Rendering color tags ({}) [{}]",
				c->GetCleanName(),
				tag_index,
				color_tag
			);

			std::string     html_tag;
			for (const auto &color : html_colors) {
				if (color_tag.find(color.first) != std::string::npos) {
					// build html tag
					html_tag = fmt::format("<c \"{}\">", color.second);
					// pop the response off of the message
					Strings::FindReplace(output, fmt::format("{{{}}}", color.first), html_tag);
				}
			}

			// too many iterations / safety net
			if (tag_index > 100) {
				break;
			}

			tag_index++;
		}
	}

	// build the final output string
	std::string final_output;
	final_output = fmt::format("{}{}{} <br><br> {}", quote_string, Strings::Trim(output), quote_string, click_response);
	if (render_hiddenresponse) {
		final_output = fmt::format("{}{}{}", quote_string, Strings::Trim(output), quote_string);
	}

	// send popup
	c->SendFullPopup(
		title.c_str(),
		final_output.c_str(),
		popup_id,
		negative_id,
		window_type,
		window_expire_seconds,
		button_one_name.c_str(),
		button_two_name.c_str(),
		sound_controls
	);

	// if multiple brackets are presented, send message
	if (!bracket_responses.empty()) {
		c->Message(Chat::White, " --- Select Response from Options --- ");
		c->Message(Chat::White, Strings::Implode(" ", bracket_responses).c_str());
	}
}

std::string DialogueWindow::Break(uint32 break_count)
{
	if (!break_count) {
		return std::string();
	}

	std::string break_message;
	auto count = break_count;

	while (count) {
		break_message.append("<br>");
		count--;
	}

	return break_message;
}

std::string DialogueWindow::CenterMessage(std::string message)
{
	if (message.empty()) {
		return std::string();
	}

	std::string cleaned_message;
	cleaned_message.reserve(message.size());

	// Strip HTML-like tags
	bool in_tag = false;
	for (char c : message) {
		if (c == '<') {
			in_tag = true;
		} else if (c == '>' && in_tag) {
			in_tag = false;
		} else if (!in_tag) {
			cleaned_message += c;
		}
	}

	auto message_len = cleaned_message.length();
	auto initial_index = (53 - (message_len * .80));
	auto index = 0;
	std::string buffer;
	while (index < initial_index) {
		buffer.append("&nbsp;");
		index++;
	}

	return fmt::format("{} {}", buffer, message);
}

std::string DialogueWindow::ColorMessage(std::string color, std::string message)
{
	if (message.empty()) {
		return std::string();
	}

	if (!color.empty()) {
		const auto &c = html_colors.find(color);
		if (c != html_colors.end()) {
			return fmt::format(
				"<c \"{}\">{}</c>",
				c->second,
				message
			);
		}
	}

	return message;
}

std::string DialogueWindow::Indent(uint32 indent_count)
{
	if (!indent_count) {
		return std::string();
	}

	std::string indent_message;
	auto count = indent_count;

	while (count) {
		indent_message.append("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
		count--;
	}

	return indent_message;
}

std::string DialogueWindow::Link(std::string link, std::string message)
{
	if (link.empty()) {
		return std::string();
	}

	if (!link.empty()) {
		return fmt::format(
			"<a href=\"{}\">{}</a>",
			link,
			!message.empty() ? message : link
		);
	}

	return message;
}

std::string DialogueWindow::Table(std::string message)
{
	if (message.empty()) {
		return std::string();
	}

	return fmt::format("<table>{}</table>", message);
}

std::string DialogueWindow::TableCell(std::string message)
{
	if (message.empty()) {
		return "<td></td>";
	}

	return fmt::format("<td>{}</td>", message);
}

std::string DialogueWindow::TableRow(std::string message)
{
	if (message.empty()) {
		return std::string();
	}

	return fmt::format("<tr>{}</tr>", message);
}
