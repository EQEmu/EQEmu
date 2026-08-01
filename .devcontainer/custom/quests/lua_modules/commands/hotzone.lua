-- #hotzone / #hotzones -- show today's Hot Zones: one per region, 1.5x EXP.
-- Single source of truth: the same hotzones.lua that the XP-apply (event_enter_zone) and the connect
-- welcome use, so the popup can never disagree with what's actually boosted.
local hotzones = require("hotzones")

local function hotzone(e)
	local rows = eq.popup_table_row(
		eq.popup_table_cell(eq.popup_color_message("cyan", "Zone Name")) ..
		eq.popup_table_cell(eq.popup_color_message("cyan", "Region")) ..
		eq.popup_table_cell(eq.popup_color_message("lime_green", "Bonus"))
	)

	for _, z in ipairs(hotzones.today()) do
		rows = rows .. eq.popup_table_row(
			eq.popup_table_cell(eq.popup_color_message("white", z.l)) ..
			eq.popup_table_cell(eq.popup_color_message("white", z.region or "?")) ..
			eq.popup_table_cell(eq.popup_color_message("lime_green", "1.5x EXP"))
		)
	end

	eq.popup("Today's Hot Zones (1.5x EXP)", eq.popup_table(rows))
end

return hotzone;
