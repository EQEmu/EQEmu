#pragma once
void InitializeAutoLoot();
void ShutdownAutoLoot();
void AutoLootPulse();
void AoT_MemorizeGaugePulse();
bool HandleAutoLootPacket(DWORD* con, uint16_t opcode, const char* buf, size_t size);
void CaptureLootSendContext(DWORD* con, unsigned __int32 unk, unsigned __int32 channel, DWORD a6, DWORD a7);
void SniffOutgoingPacket(uint16_t opcode, const char* buf, size_t size);
void AoT_SendRawPacket(uint16_t opcode, const void* data, size_t dataSize);
