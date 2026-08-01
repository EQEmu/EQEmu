-- bazaar_broker.lua
-- Player-shop ("offline vendor") backend. A player sets up as an AFK Bazaar trader from ANY city via
-- the /shop window (dll): fill a Trader's Satchel -> /shop -> price each item -> Open My Shop. The
-- server registers a real native trader (client:StartPlayerTrader); buyers find them via the native
-- "/bazaar -> all traders" search + parcel delivery, and the shop PERSISTS while the owner is logged
-- off (trading.cpp: rows kept on logout, sales banked to shop_escrow_<charid>, paid on next login,
-- satchel reconciled). All driven from global_player event_say/event_connect (M.handle_global_say /
-- M.pay_escrow) -- there is no NPC. Chat verbs (dll swallows the echoes): "shopopen" (open window),
-- "vpset id:copper,..." (prices), "vshop" (open shop), "vclose" (close), "getsatchel" (summon a bag).

local M = {}

-- Pay out coin banked from OFFLINE shop sales. The server credits `shop_escrow_<charid>` (copper) in
-- BuyTraderItemOutsideBazaar each time someone buys from a logged-off trader; we hand it over on the
-- seller's next login and clear the bucket. Call from global_player.lua event_connect.
function M.pay_escrow(c)
	local key    = "shop_escrow_" .. c:CharacterID()
	local copper = tonumber(eq.get_data(key) or "0") or 0
	if copper <= 0 then return end
	eq.set_data(key, "0")   -- clear before granting so a hiccup can't double-pay
	local plat   = math.floor(copper / 1000); copper = copper % 1000
	local gold   = math.floor(copper / 100);  copper = copper % 100
	local silver = math.floor(copper / 10);   copper = copper % 10
	c:AddMoneyToPP(copper, silver, gold, plat, true)
	c:Message(MT.Yellow, string.format(
		"Your shop earned %dp %dg %ds %dc while you were away.", plat, gold, silver, copper))
end

-- Push the current shop state to the dll (it swallows both lines):
--   SHOPINVDATA slot|itemid|name|vendor^...   -- droppable items you can ADD (from any bag)
--   MYSHOPDATA  item_sn|itemid|name|cost^...  -- what's already listed in your shop
local function send_shop_data(c)
	c:Message(MT.NPCQuestSay, "SHOPINVDATA "  .. (c:GetSellableInventory() or ""))   -- inventory + projected price
	c:Message(MT.NPCQuestSay, "SHOPBOOKDATA " .. (c:GetPriceBook() or ""))            -- persistent price book
	c:Message(MT.NPCQuestSay, "SHOPLOGDATA "  .. (c:GetPriceLog() or ""))             -- recent price changes
	c:Message(MT.NPCQuestSay, "MYSHOPDATA "   .. (c:GetMyShopListing() or ""))        -- current shop rows
	-- History panels, sent with the rest so the window never has to ask twice:
	--   SHOPLISTLOG  itemid|name|qty|price|when^...        -- what you have listed (List Item tab)
	--   SHOPSOLDLOG  itemid|name|qty|price|buyer|when^...  -- what has sold      (My Shop tab)
	-- Both are newest-first and capped server side, so they cannot outgrow a chat line.
	--
	-- ⚠️ AN EMPTY LOG IS NOT SENT AT ALL. Both are append-only histories, so "empty" only ever means
	-- "nothing has happened yet" and there is no stale panel to clear -- but a dll that does not yet
	-- swallow the line prints the bare header to chat, which is what a player sees every time they
	-- open the shop. Skipping the send costs nothing and keeps a server change from leaking into chat
	-- ahead of the client half that hides it.
	local listlog = c:GetShopListingLog() or ""
	local soldlog = c:GetShopSoldLog()    or ""
	if listlog ~= "" then c:Message(MT.NPCQuestSay, "SHOPLISTLOG " .. listlog) end
	if soldlog ~= "" then c:Message(MT.NPCQuestSay, "SHOPSOLDLOG " .. soldlog) end
end

-- GLOBAL say handler (from global_player event_say). The dll issues these via /say and swallows the
-- echoes:  "shopopen"/"shoprefresh" refresh the window;  "shopadd slot:price,..." escrows those bag
-- items into the shop;  "shoppull <item_sn>" pulls one listing back to the cursor.
function M.handle_global_say(e)
	local c   = e.self                       -- in a PLAYER event, e.self is the client
	local msg = (e.message or ""):lower()

	if msg == "shopopen" or msg == "shoprefresh" then   -- /shop (dll rewrites to /say shopopen)
		send_shop_data(c)
		return true
	end

	local pid, pprice = msg:match("^shopsetprice%s+(%d+):(%d+)$")   -- set the price book: "itemid:price"
	if pid then
		c:SetItemPrice(tonumber(pid), tonumber(pprice))
		send_shop_data(c)
		return true
	end

	local add = msg:match("^shopadd%s+(.+)$")           -- "slot:qty,slot:qty,..." (price comes from the book)
	if add then
		local n = c:AddItemsToShop(add)
		if n > 0 then
			c:Message(MT.Yellow, n .. " item(s) added to your shop. They stay for sale in the Bazaar even " ..
				"while you're logged off; your coin arrives instantly or on your next login.")
		end
		send_shop_data(c)
		return true
	end

	local sn = msg:match("^shoppull%s+(%d+)$")           -- item_sn of the listing to reclaim
	if sn then
		c:PullShopItem(tonumber(sn))
		send_shop_data(c)
		return true
	end

	return false
end

return M
