#include "MQ2Main.h"
#include "AoT_AutoLoot.h"

// Opcodes confirmed by packet sniff against this server build
#define OP_LootRequest    0x0adf   // confirmed outgoing (size 6, payload = DWORD corpse entity ID)
#define OP_EndLootRequest 0x30f7   // confirmed outgoing (size 6), also client close-loot
#define OP_LootItem       0x4dc9   // confirmed outgoing (size 22)
#define OP_MoneyOnCorpse  0x5f44   // confirmed incoming (size 20: BYTE resp, BYTE[3] pad, DWORD plat/gold/silv/cop)
#define OP_ItemPacket     0x368e   // confirmed incoming (one per item in loot window, variable size)
#define OP_LootComplete     0x55c4   // confirmed incoming We want to ignore this packet.

// Loot slots on this server start at 0x17 (23) — mirrors EQ inventory bag numbering
#define LOOT_SLOT_FIRST  23
#define LOOT_SLOT_LAST   52   // 30 slots covers all likely drops

#define CORPSE_QUEUE_MAX 16
#define MAX_LOOT_ITEMS   32

#pragma pack(push, 1)
struct LootingItem_Struct {
    DWORD lootee;
    DWORD looter;
    WORD  slot_id;
    BYTE  pad[2];
    DWORD auto_loot;
    DWORD unknown;
}; // 20 bytes
#pragma pack(pop)

// Per-item data parsed from OP_ItemPacket (0x368e)
struct LootItemInfo {
    WORD  slot;       // loot window slot (23-52)
    DWORD itemID;
    char  name[64];
    int   noDrop;     // 1 = no-drop
    int   augType;    // > 0 = augmentation
    int   stackable;  // 1 = stackable
    int   priceCop;   // NPC buy price, copper
};


// Most of this is unneccessary for this project but could be useful later
// stolen from rof2.cpp
struct RoF2_Header {
    char    unknown000[16];
    uint32_t  stacksize;
    uint32_t  unknown004;
    uint16_t  slot_type;
    uint16_t  main_slot;
    uint16_t  sub_slot;
    uint16_t  aug_slot;
    uint32_t  price;
    uint32_t  merchant_slot;
    uint32_t  scaled_value;
    uint32_t  instance_id;
    uint32_t  unknown028;
    uint32_t  last_cast_time;
    uint32_t  charges;
    uint8_t   inst_nodrop;
    uint32_t  unknown044;
    uint32_t  unknown048;
    uint32_t  unknown052;
    uint8_t   isEvolving;
};

struct RoF2_SecondaryBody {
    uint32_t augtype;      // This is what you need (Field 93 equivalent)
    int32_t  augrestrict2;
    uint32_t augrestrict;
    // ... followed by augslots[6], bag data, and book data ...
};

struct RoF2_FinishHeader {
    uint32_t ornamentIcon;
    uint32_t unknowna1;
    uint32_t ornamentHeroModel;
    uint8_t  unknown063;
    uint8_t  Copied;
    uint32_t unknowna4;
    uint32_t unknowna5;
    uint8_t  ItemClass;
};


enum AutoLootState { ALS_IDLE, ALS_WAITING_MONEY, ALS_LOOTING };

extern unsigned char __fastcall SendMessage_Trampoline(
    DWORD*, unsigned __int32, unsigned __int32, char*, size_t, DWORD, DWORD);

static bool          gbAutoLoot   = false;
static bool          gbDebug      = false;
static bool          gbSniff      = false;
static AutoLootState gState       = ALS_IDLE;
static DWORD         gStateTick   = 0;
static DWORD*        gLootCon     = nullptr;
static DWORD         gSendUnk     = 0;
static DWORD         gSendChannel = 0;
static DWORD         gSendA6      = 0;
static DWORD         gSendA7      = 0;
static bool          gHaveSendCtx = false;

// Active loot session
static DWORD gCorpseId   = 0;
static DWORD gLooterID   = 0;
static WORD  gNextSlot   = LOOT_SLOT_FIRST;

// Corpse queue: filled during IDLE scan, consumed one at a time
static DWORD gCorpseQueue[CORPSE_QUEUE_MAX];
static int   gCorpseQueueCount = 0;
static int   gCorpseQueueHead  = 0;

// Item list for current loot session (populated from 0x368e packets)
static LootItemInfo gPendingItems[MAX_LOOT_ITEMS];
static int          gPendingCount = 0;


#define DBGF(fmt, ...) do { if (gbDebug) WriteChatf("\agAutoLoot: " fmt, ##__VA_ARGS__); } while(0)

// ── item packet parser ────────────────────────────────────────────────────────


const char* ReadString(const char* src, char* dest, size_t destSize, const char* end) {
    size_t len = 0;
    const char* start = src;
    while (src < end && *src != 0) {
        src++;
        len++;
    }
    if (dest && destSize > 0) {
        size_t toCopy = (len < destSize - 1) ? len : destSize - 1;
        memcpy(dest, start, toCopy);
        dest[toCopy] = '\0';
    }
    WriteChatf("%s", src);
    return (src < end) ? src + 1 : end; // Skip the null terminator
}


static bool ParseRoF2ItemPacket(const char* buf, size_t size, LootItemInfo& out)
{
    const char* p = buf + 4;
    const char* pEnd = buf + size;

    // 1. Skip Header & Evolving Data
    const RoF2_Header* hdr = (const RoF2_Header*)p;
    p += sizeof(RoF2_Header);
    if (hdr->isEvolving > 0) p += 32;

    // 2. Skip Ornamentation Strings
    p = ReadString(p, nullptr, 0, pEnd);
    p = ReadString(p, nullptr, 0, pEnd);

    // 3. Skip Finish Header
    p += sizeof(RoF2_FinishHeader);

    // 4. Read Name (Field 12) and skip Lore/IDFile
    p = ReadString(p, out.name, 64, pEnd);
    p = ReadString(p, nullptr, 0, pEnd);
    p = ReadString(p, nullptr, 0, pEnd);
    p += 1; // Extra null terminator

    // 5. Read Item Body to get ItemID (Field 15)
    // Note: Ensure your 'Body' struct size matches your server's ItemBodyStruct exactly!
    const uint32_t* itemIDPtr = (const uint32_t*)p;
    out.itemID = *itemIDPtr;

    // Skip the rest of the fixed ItemBodyStruct (usually 224 bytes in RoF2)
    p += 224;

    // 6. Skip Charm Text string
    p = ReadString(p, nullptr, 0, pEnd);

    // 7. Read Secondary Body for AugType (Field 93)
    if (p + sizeof(RoF2_SecondaryBody) <= pEnd) {
        const RoF2_SecondaryBody* isbs = (const RoF2_SecondaryBody*)p;
        out.augType = (BYTE)isbs->augtype; // If > 0, it's an augment
    }

    // 8. Map basic flags from the primary header
    out.slot = hdr->main_slot;
    out.noDrop = hdr->inst_nodrop;
    out.priceCop = hdr->price;
    // Stackable check: RoF2 often stores this in the Tertiary body, 
    // but hdr->stacksize is a reliable fallback for loots.
    out.stackable = (hdr->stacksize > 1);

    WriteChatf("  slot=%u id=%u %s%s%s price=%dcp \"%s\"",
        out.slot, out.itemID,
        out.noDrop ? "\arND\ax " : "",
        out.augType ? "\ayAUG\ax " : "",
        out.stackable ? "\agSTK\ax " : "",
        out.priceCop,
        out.name[0] ? out.name : "?");

    return (out.itemID != 0);
}

// ── outgoing packet sniff ─────────────────────────────────────────────────────

void SniffOutgoingPacket(uint16_t opcode, const char* buf, size_t size)
{
    if (!gbSniff) return;
    if (size != 6 && size != 18 && size != 22) return;
    char hexbuf[64] = {};
    size_t payloadSize = size > 2 ? size - 2 : 0;
    for (size_t i = 0; i < payloadSize && i < 20; i++)
        sprintf(hexbuf + i * 3, "%02x ", (unsigned char)buf[2 + i]);
    WriteChatf("\ayAutoLoot OUT: op=0x%04x size=%zu payload=[%s]", opcode, size, hexbuf);
}

// ── send context capture ──────────────────────────────────────────────────────

void CaptureLootSendContext(DWORD* con, unsigned __int32 unk, unsigned __int32 channel, DWORD a6, DWORD a7)
{
    bool wasFirst = !gHaveSendCtx;
    gLootCon     = con;
    gSendUnk     = unk;
    gSendChannel = channel;
    gSendA6      = a6;
    gSendA7      = a7;
    gHaveSendCtx = true;
    if (wasFirst)
        WriteChatf("\agAutoLoot: send context captured (con=%p unk=%u ch=%u)", con, unk, channel);
}

// ── internal helpers ──────────────────────────────────────────────────────────

static void SendLootPkt(WORD opcode, const void* data, size_t dataSize)
{
    if (!gLootCon || !gHaveSendCtx) {
        DBGF("SendLootPkt: no send context");
        return;
    }
    size_t total = 2 + dataSize;
    char* buf = (char*)_alloca(total);
    memcpy(buf, &opcode, 2);
    if (data && dataSize) memcpy(buf + 2, data, dataSize);
    SendMessage_Trampoline(gLootCon, gSendUnk, gSendChannel, buf, total, gSendA6, gSendA7);
}

void AoT_SendRawPacket(uint16_t opcode, const void* data, size_t dataSize)
{
    SendLootPkt(opcode, data, dataSize);
}

static void ResetState()
{
    gState    = ALS_IDLE;
    gCorpseId = 0;
    gLooterID = 0;
    gNextSlot = LOOT_SLOT_FIRST;
    gStateTick = 0;
    gPendingCount = 0;
}

static void StartNextCorpse()
{
    if (gCorpseQueueHead >= gCorpseQueueCount) {
        gCorpseQueueCount = 0;
        gCorpseQueueHead  = 0;
        ResetState();
        return;
    }
    gCorpseId  = gCorpseQueue[gCorpseQueueHead++];
    gStateTick = GetTickCount();
    gState     = ALS_WAITING_MONEY;
    gPendingCount = 0;
    SendLootPkt(OP_LootRequest, &gCorpseId, 4);
    DBGF("LootRequest -> corpse %u (%d/%d)", gCorpseId, gCorpseQueueHead, gCorpseQueueCount);
}

static const char* StateName(AutoLootState s)
{
    switch (s) {
    case ALS_IDLE:          return "IDLE";
    case ALS_WAITING_MONEY: return "WAITING_MONEY";
    case ALS_LOOTING:       return "LOOTING";
    }
    return "?";
}

// ── command ───────────────────────────────────────────────────────────────────

static void Cmd_AutoLoot(PSPAWNINFO pChar, PCHAR szLine)
{
    if (szLine && !_stricmp(szLine, "debug")) {
        gbDebug = !gbDebug;
        WriteChatf("AutoLoot debug: \a%c%s", gbDebug ? 'g' : 'r', gbDebug ? "ON" : "OFF");
        return;
    }
    if (szLine && !_stricmp(szLine, "sniff")) {
        gbSniff = !gbSniff;
        WriteChatf("AutoLoot sniff: \a%c%s", gbSniff ? 'g' : 'r', gbSniff ? "ON" : "OFF");
        return;
    }
    if (szLine && !_stricmp(szLine, "status")) {
        WriteChatf("AutoLoot: %s  state=%s  corpse=%u  queue=%d  slot=%u  sendctx=%s  items=%d",
            gbAutoLoot ? "\agON" : "\arOFF",
            StateName(gState), gCorpseId,
            gCorpseQueueCount - gCorpseQueueHead,
            gNextSlot,
            gHaveSendCtx ? "\agyes" : "\arno",
            gPendingCount);
        return;
    }
    if (szLine && !_stricmp(szLine, "items")) {
        WriteChatf("AutoLoot: %d item(s) in pending list:", gPendingCount);
        for (int i = 0; i < gPendingCount; i++) {
            LootItemInfo& it = gPendingItems[i];
            
        }
        return;
    }
    gbAutoLoot = !gbAutoLoot;
    if (!gbAutoLoot) { ResetState(); gCorpseQueueCount = 0; gCorpseQueueHead = 0; }
    WriteChatf("AutoLoot: \a%c%s", gbAutoLoot ? 'g' : 'r', gbAutoLoot ? "ON" : "OFF");
    

}

// ── pulse (game loop) ─────────────────────────────────────────────────────────

void AutoLootPulse()
{
    if (!gbAutoLoot) return;
    if (!ppCharSpawn || !pCharSpawn) return;

    DWORD now = GetTickCount();

    // Timeout guard for stuck states
    if (gState != ALS_IDLE && now - gStateTick > 5000) {
        WriteChatf("\arAutoLoot: timeout in %s — moving on", StateName(gState));
        StartNextCorpse();
        return;
    }

    // ALS_LOOTING: send one slot per pulse tick, then EndLootRequest when done
    if (gState == ALS_LOOTING) {
        if (gNextSlot <= LOOT_SLOT_LAST) {
            LootingItem_Struct li = {};
            li.lootee    = gCorpseId;
            li.looter    = gLooterID;
            li.slot_id   = gNextSlot++;
            li.auto_loot = 1;
            SendLootPkt(OP_LootItem, &li, sizeof(li));
        } else {
            if (gPendingCount > 0 && gbDebug) {
                WriteChatf("\agAutoLoot: looted %d item(s) from corpse %u:", gPendingCount, gCorpseId);
                for (int i = 0; i < gPendingCount; i++) {
                    LootItemInfo& it = gPendingItems[i];
                    WriteChatf("  %s%s%s\"%s\" (id=%u)",
                        it.noDrop  ? "[ND] " : "",
                        it.augType ? "[AUG] " : "",
                        it.stackable ? "[STK] " : "",
                        it.name[0] ? it.name : "?", it.itemID);
                }
            }
            SendLootPkt(OP_EndLootRequest, &gCorpseId, 4);
            DBGF("EndLootRequest sent for corpse %u — moving on", gCorpseId);
            StartNextCorpse();
        }
        return;
    }

    // ALS_WAITING_MONEY: handled by HandleAutoLootPacket, nothing to do here
    if (gState == ALS_WAITING_MONEY) return;

    // ALS_IDLE: scan once per second, fill corpse queue
    static DWORD dwLastScan = 0;
    if (now - dwLastScan < 1000) return;
    dwLastScan = now;

    if (!gHaveSendCtx) return;

    PSPAWNINFO pChar = (PSPAWNINFO)pCharSpawn;
    int nFound = 0;
    gCorpseQueueCount = 0;
    gCorpseQueueHead  = 0;

    for (PSPAWNINFO p = (PSPAWNINFO)pSpawnList; p; p = p->pNext) {
        if (p->Type != SPAWN_CORPSE) continue;
        float dx = pChar->X - p->X;
        float dy = pChar->Y - p->Y;
        if (dx * dx + dy * dy > 625.0f) continue;   // 25 unit radius
        if (gCorpseQueueCount < CORPSE_QUEUE_MAX)
            gCorpseQueue[gCorpseQueueCount++] = p->SpawnID;
        nFound++;
    }

    if (gCorpseQueueCount == 0) return;

    DBGF("Found %d corpse(s) in range — starting queue", gCorpseQueueCount);
    StartNextCorpse();
}

// ── incoming packet handler ───────────────────────────────────────────────────

bool HandleAutoLootPacket(DWORD* con, uint16_t opcode, const char* buf, size_t size)
{
    // TODO: If not looting, ignore itemdata packets, merchants and item links need em too.

    if (gbSniff && opcode != 0x7dfc) {
        if (opcode == OP_ItemPacket && size > 4) {
            // 16-byte rows with ASCII column for easy field identification
            WriteChatf("\ayAutoLoot  IN: op=0x%04x size=%zu [%s]", opcode, size, StateName(gState));
            for (size_t row = 0; row < size && row < 512; row += 16) {
                char hexbuf[64] = {};
                char ascbuf[20] = {};
                for (size_t i = 0; i < 16 && (row + i) < size; i++) {
                    unsigned char c = (unsigned char)buf[row + i];
                    sprintf(hexbuf + i * 3, "%02x ", c);
                    ascbuf[i] = (c >= 0x20 && c < 0x7f) ? (char)c : '.';
                }
                ascbuf[16] = '\0';
                WriteChatf("\ay  [%03zu] %-48s  %s", row, hexbuf, ascbuf);
            }
        } else {
            WriteChatf("\ayAutoLoot  IN: op=0x%04x size=%zu [%s]", opcode, size, StateName(gState));
        }
    }

    if (!gbAutoLoot) return false;

    // if we dont ignore this opcode it removes our target, we dont want that and
    // the client doesnt actually know we are looting to begin with so it doesnt matter.
    if (opcode == OP_LootComplete)  return true;
    

    // MoneyOnCorpse: suppress loot window, begin looting
    if (opcode == OP_MoneyOnCorpse && gState == ALS_WAITING_MONEY) {
        BYTE resp = (size >= 1) ? (BYTE)buf[0] : 0xFF;
        DBGF("MoneyOnCorpse resp=0x%02x (%s)",
            resp, resp == 1 ? "OK" : resp == 0 ? "DENY/BUSY" : resp == 2 ? "TOO FAR" : "UNKNOWN");
        if (resp == 1) {
            PCHARINFO pCI = GetCharInfo();
            gLooterID  = (pCI && pCI->pSpawn) ? pCI->pSpawn->SpawnID : 0;
            gNextSlot  = LOOT_SLOT_FIRST;
            gState     = ALS_LOOTING;
            gStateTick = GetTickCount();
        } else {
            WriteChatf("\arAutoLoot: loot denied (resp=0x%02x) — next corpse", resp);
            StartNextCorpse();
        }
        return true;   // suppress loot window from opening
    }

    // ItemPacket: parse item data, suppress window population
    if (opcode == OP_ItemPacket) {
        if (gPendingCount < MAX_LOOT_ITEMS) {
            LootItemInfo info = {};
            if (ParseRoF2ItemPacket(buf, size, info)) {
                gPendingItems[gPendingCount++] = info;
            } else {
                DBGF("Item parse failed (size=%zu, hdr=0x%02x)", size, size > 0 ? (unsigned char)buf[0] : 0);
            }
        }
        return true;   // suppress loot window population
    }

    return false;
}

// ── init / shutdown ───────────────────────────────────────────────────────────

void InitializeAutoLoot()
{
    AddCommand("/autoloot", Cmd_AutoLoot, 0, 0, 1);
}

void ShutdownAutoLoot()
{
    RemoveCommand("/autoloot");
    ResetState();
}
