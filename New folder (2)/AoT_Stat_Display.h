#pragma once
#include <cstdint>

// Mirror of server AoT_StatBlock_Struct (common/eq_packet_structs.h).
// Server header uses #pragma pack(1) globally, so this struct is tightly packed.
#pragma pack(push, 1)
struct AoT_StatBlock_Struct_Client {
    // Defensive
    int32_t ac_mitigation;
    int32_t min_mit_pct_x100;
    int32_t deflect_pct_x100;
    int32_t avoidance_score;
    int32_t avoid_pct_x100;
    uint8_t critable;
    // Speed
    int32_t melee_haste_x100;
    int32_t cast_speed_x100;
    // Primary weapon
    int32_t wpn_max_hit;
    int32_t wpn_offense;
    int32_t wpn_accuracy;
    int32_t wpn_hit_pct_x100;
    int32_t wpn_crit_pct_x1000;
    int32_t wpn_crit_dmg_pct_x100;
    int32_t wpn_delay_x100;
    int32_t wpn_dps_x100;
    uint8_t wpn_valid;
    // Secondary weapon
    int32_t sec_max_hit;
    int32_t sec_offense;
    int32_t sec_accuracy;
    int32_t sec_hit_pct_x100;
    int32_t sec_crit_pct_x1000;
    int32_t sec_crit_dmg_pct_x100;
    int32_t sec_delay_x100;
    int32_t sec_dps_x100;
    uint8_t sec_valid;
    // Ranged weapon
    int32_t rng_max_hit;
    int32_t rng_offense;
    int32_t rng_accuracy;
    int32_t rng_hit_pct_x100;
    int32_t rng_crit_pct_x1000;
    int32_t rng_crit_dmg_pct_x100;
    int32_t rng_delay_x100;
    int32_t rng_dps_x100;
    uint8_t rng_valid;
    // Spell stats ([0]=PSN [1]=MAG [2]=DIS [3]=FIR [4]=CLD [5]=CRP)
    int32_t spell_crit_pct_x1000;
    int32_t spell_dc[6];
    int32_t spell_potency_x100[6];
    // Healing stats
    int32_t heal_crit_pct_x1000;
    int32_t heal_potency_direct_x100;
    int32_t heal_potency_hot_x100;
    int32_t heal_potency_rune_x100;
    // Threat
    int32_t threat_mod_x100;
};
#pragma pack(pop)

void AoT_StatDisplay_Receive(const char* buf, size_t len);
void AoT_StatDisplay_Pulse();
void AoT_StatDisplay_Cleanup();
