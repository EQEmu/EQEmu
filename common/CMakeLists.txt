cmake_minimum_required(VERSION 3.20.0)

set(common_sources
    base_packet.cpp
    bazaar.cpp
    bodytypes.cpp
    classes.cpp
    cli/eqemu_command_handler.cpp
    compression.cpp
    condition.cpp
    content/world_content_service.cpp
    crash.cpp
    crc16.cpp
    crc32.cpp
    data_bucket.cpp
    database.cpp
    database/database_dump_service.cpp
    database/database_update.cpp
    database_instances.cpp
    dbcore.cpp
    deity.cpp
    discord/discord.cpp
    discord/discord_manager.cpp
    dynamic_zone_base.cpp
    dynamic_zone_lockout.cpp
    emu_constants.cpp
    emu_limits.cpp
    emu_opcodes.cpp
    emu_versions.cpp
    eq_limits.cpp
    eq_packet.cpp
    eq_stream_ident.cpp
    eq_stream_proxy.cpp
    eqdb.cpp
    eqdb_res.cpp
    eqemu_config.cpp
    eqemu_exception.cpp
    eqemu_logsys.cpp
    eqtime.cpp
    event_sub.cpp
    events/player_event_discord_formatter.cpp
    events/player_event_logs.cpp
    evolving_items.cpp
    extprofile.cpp
    faction.cpp
    file.cpp
    guild_base.cpp
    guilds.cpp
    inventory_profile.cpp
    inventory_slot.cpp
    ip_util.cpp
    ipc_mutex.cpp
    item_data.cpp
    item_instance.cpp
    json/json.hpp
    json/jsoncpp.cpp
    json_config.cpp
    light_source.cpp
    md5.cpp
    memory/ksm.hpp
    memory_buffer.cpp
    memory_mapped_file.cpp
    misc.cpp
    misc_functions.cpp
    mutex.cpp
    mysql_request_result.cpp
    mysql_request_row.cpp
    mysql_stmt.cpp
    net/console_server.cpp
    net/console_server_connection.cpp
    net/crc32.cpp
    net/eqstream.cpp
    net/packet.cpp
    net/reliable_stream_connection.cpp
    net/servertalk_client_connection.cpp
    net/servertalk_legacy_client_connection.cpp
    net/servertalk_server.cpp
    net/servertalk_server_connection.cpp
    net/tcp_connection.cpp
    net/tcp_server.cpp
    net/websocket_server.cpp
    net/websocket_server_connection.cpp
    opcode_map.cpp
    opcodemgr.cpp
    packet_dump.cpp
    packet_dump_file.cpp
    packet_functions.cpp
    patches/patches.cpp
    patches/rof.cpp
    patches/rof2.cpp
    patches/rof2_limits.cpp
    patches/rof_limits.cpp
    patches/sod.cpp
    patches/sod_limits.cpp
    patches/sof.cpp
    patches/sof_limits.cpp
    patches/titanium.cpp
    patches/titanium_limits.cpp
    patches/uf.cpp
    patches/uf_limits.cpp
    path_manager.cpp
    perl_eqdb.cpp
    perl_eqdb_res.cpp
    platform.cpp
    proc_launcher.cpp
    process.cpp
    process/process.cpp
    profanity_manager.cpp
    ptimer.cpp
    races.cpp
    rdtsc.cpp
    rulesys.cpp
    say_link.cpp
    serialize_buffer.cpp
    server_event_scheduler.cpp
    serverinfo.cpp
    shared_tasks.cpp
    shareddb.cpp
    skill_caps.cpp
    skills.cpp
    spdat.cpp
    spdat_bot.cpp
    StackWalker/StackWalker.cpp
    strings.cpp
    strings_legacy.cpp
    strings_misc.cpp
    struct_strategy.cpp
    textures.cpp
    timer.cpp
    unix.cpp
    util/directory.cpp
    util/uuid.cpp
    zone_store.cpp
)

set(repositories
    # Criteria
    repositories/criteria/content_filter_criteria.h

    # Base Repositories
    repositories/base/base_aa_ability_repository.h
    repositories/base/base_aa_rank_effects_repository.h
    repositories/base/base_aa_rank_prereqs_repository.h
    repositories/base/base_aa_ranks_repository.h
    repositories/base/base_account_flags_repository.h
    repositories/base/base_account_ip_repository.h
    repositories/base/base_account_repository.h
    repositories/base/base_account_rewards_repository.h
    repositories/base/base_adventure_details_repository.h
    repositories/base/base_adventure_members_repository.h
    repositories/base/base_adventure_stats_repository.h
    repositories/base/base_adventure_template_entry_flavor_repository.h
    repositories/base/base_adventure_template_entry_repository.h
    repositories/base/base_adventure_template_repository.h
    repositories/base/base_alternate_currency_repository.h
    repositories/base/base_auras_repository.h
    repositories/base/base_base_data_repository.h
    repositories/base/base_blocked_spells_repository.h
    repositories/base/base_books_repository.h
    repositories/base/base_bug_reports_repository.h
    repositories/base/base_bugs_repository.h
    repositories/base/base_buyer_repository.h
    repositories/base/base_buyer_trade_items_repository.h
    repositories/base/base_char_create_combinations_repository.h
    repositories/base/base_char_create_point_allocations_repository.h
    repositories/base/base_char_recipe_list_repository.h
    repositories/base/base_character_activities_repository.h
    repositories/base/base_character_alt_currency_repository.h
    repositories/base/base_character_alternate_abilities_repository.h
    repositories/base/base_character_auras_repository.h
    repositories/base/base_character_bandolier_repository.h
    repositories/base/base_character_bind_repository.h
    repositories/base/base_character_buffs_repository.h
    repositories/base/base_character_corpse_items_repository.h
    repositories/base/base_character_corpses_repository.h
    repositories/base/base_character_currency_repository.h
    repositories/base/base_character_data_repository.h
    repositories/base/base_character_disciplines_repository.h
    repositories/base/base_character_evolving_items_repository.h
    repositories/base/base_character_exp_modifiers_repository.h
    repositories/base/base_character_expedition_lockouts_repository.h
    repositories/base/base_character_inspect_messages_repository.h
    repositories/base/base_character_instance_safereturns_repository.h
    repositories/base/base_character_item_recast_repository.h
    repositories/base/base_character_languages_repository.h
    repositories/base/base_character_leadership_abilities_repository.h
    repositories/base/base_character_material_repository.h
    repositories/base/base_character_memmed_spells_repository.h
    repositories/base/base_character_parcels_containers_repository.h
    repositories/base/base_character_parcels_repository.h
    repositories/base/base_character_peqzone_flags_repository.h
    repositories/base/base_character_pet_buffs_repository.h
    repositories/base/base_character_pet_info_repository.h
    repositories/base/base_character_pet_inventory_repository.h
    repositories/base/base_character_potionbelt_repository.h
    repositories/base/base_character_skills_repository.h
    repositories/base/base_character_spells_repository.h
    repositories/base/base_character_task_timers_repository.h
    repositories/base/base_character_tasks_repository.h
    repositories/base/base_chatchannel_reserved_names_repository.h
    repositories/base/base_chatchannels_repository.h
    repositories/base/base_completed_shared_task_activity_state_repository.h
    repositories/base/base_completed_shared_task_members_repository.h
    repositories/base/base_completed_shared_tasks_repository.h
    repositories/base/base_completed_tasks_repository.h
    repositories/base/base_content_flags_repository.h
    repositories/base/base_damageshieldtypes_repository.h
    repositories/base/base_data_buckets_repository.h
    repositories/base/base_db_str_repository.h
    repositories/base/base_discord_webhooks_repository.h
    repositories/base/base_discovered_items_repository.h
    repositories/base/base_doors_repository.h
    repositories/base/base_dynamic_zone_lockouts_repository.h
    repositories/base/base_dynamic_zone_members_repository.h
    repositories/base/base_dynamic_zone_templates_repository.h
    repositories/base/base_dynamic_zones_repository.h
    repositories/base/base_faction_association_repository.h
    repositories/base/base_faction_base_data_repository.h
    repositories/base/base_faction_list_mod_repository.h
    repositories/base/base_faction_list_repository.h
    repositories/base/base_faction_values_repository.h
    repositories/base/base_fishing_repository.h
    repositories/base/base_forage_repository.h
    repositories/base/base_friends_repository.h
    repositories/base/base_global_loot_repository.h
    repositories/base/base_gm_ips_repository.h
    repositories/base/base_graveyard_repository.h
    repositories/base/base_grid_entries_repository.h
    repositories/base/base_grid_repository.h
    repositories/base/base_ground_spawns_repository.h
    repositories/base/base_group_id_repository.h
    repositories/base/base_group_leaders_repository.h
    repositories/base/base_guild_bank_repository.h
    repositories/base/base_guild_members_repository.h
    repositories/base/base_guild_permissions_repository.h
    repositories/base/base_guild_ranks_repository.h
    repositories/base/base_guild_relations_repository.h
    repositories/base/base_guilds_repository.h
    repositories/base/base_horses_repository.h
    repositories/base/base_instance_list_player_repository.h
    repositories/base/base_instance_list_repository.h
    repositories/base/base_inventory_repository.h
    repositories/base/base_inventory_snapshots_repository.h
    repositories/base/base_ip_exemptions_repository.h
    repositories/base/base_items_evolving_details_repository.h
    repositories/base/base_items_repository.h
    repositories/base/base_ldon_trap_entries_repository.h
    repositories/base/base_ldon_trap_templates_repository.h
    repositories/base/base_level_exp_mods_repository.h
    repositories/base/base_lfguild_repository.h
    repositories/base/base_login_accounts_repository.h
    repositories/base/base_login_api_tokens_repository.h
    repositories/base/base_login_server_admins_repository.h
    repositories/base/base_login_server_list_types_repository.h
    repositories/base/base_login_world_servers_repository.h
    repositories/base/base_logsys_categories_repository.h
    repositories/base/base_lootdrop_entries_repository.h
    repositories/base/base_lootdrop_repository.h
    repositories/base/base_loottable_entries_repository.h
    repositories/base/base_loottable_repository.h
    repositories/base/base_mail_repository.h
    repositories/base/base_merchantlist_repository.h
    repositories/base/base_merchantlist_temp_repository.h
    repositories/base/base_name_filter_repository.h
    repositories/base/base_npc_emotes_repository.h
    repositories/base/base_npc_faction_entries_repository.h
    repositories/base/base_npc_faction_repository.h
    repositories/base/base_npc_scale_global_base_repository.h
    repositories/base/base_npc_spells_effects_entries_repository.h
    repositories/base/base_npc_spells_effects_repository.h
    repositories/base/base_npc_spells_entries_repository.h
    repositories/base/base_npc_spells_repository.h
    repositories/base/base_npc_types_repository.h
    repositories/base/base_npc_types_tint_repository.h
    repositories/base/base_object_contents_repository.h
    repositories/base/base_object_repository.h
    repositories/base/base_perl_event_export_settings_repository.h
    repositories/base/base_petitions_repository.h
    repositories/base/base_pets_beastlord_data_repository.h
    repositories/base/base_pets_equipmentset_entries_repository.h
    repositories/base/base_pets_equipmentset_repository.h
    repositories/base/base_pets_repository.h
    repositories/base/base_player_event_aa_purchase_repository.h
    repositories/base/base_player_event_killed_named_npc_repository.h
    repositories/base/base_player_event_killed_npc_repository.h
    repositories/base/base_player_event_killed_raid_npc_repository.h
    repositories/base/base_player_event_log_settings_repository.h
    repositories/base/base_player_event_logs_repository.h
    repositories/base/base_player_event_loot_items_repository.h
    repositories/base/base_player_event_merchant_purchase_repository.h
    repositories/base/base_player_event_merchant_sell_repository.h
    repositories/base/base_player_event_npc_handin_entries_repository.h
    repositories/base/base_player_event_npc_handin_repository.h
    repositories/base/base_player_event_speech_repository.h
    repositories/base/base_player_event_trade_entries_repository.h
    repositories/base/base_player_event_trade_repository.h
    repositories/base/base_player_titlesets_repository.h
    repositories/base/base_quest_globals_repository.h
    repositories/base/base_raid_details_repository.h
    repositories/base/base_raid_members_repository.h
    repositories/base/base_reports_repository.h
    repositories/base/base_respawn_times_repository.h
    repositories/base/base_rule_sets_repository.h
    repositories/base/base_rule_values_repository.h
    repositories/base/base_saylink_repository.h
    repositories/base/base_server_scheduled_events_repository.h
    repositories/base/base_shared_task_activity_state_repository.h
    repositories/base/base_shared_task_dynamic_zones_repository.h
    repositories/base/base_shared_task_members_repository.h
    repositories/base/base_shared_tasks_repository.h
    repositories/base/base_skill_caps_repository.h
    repositories/base/base_spawn2_repository.h
    repositories/base/base_spawn_condition_values_repository.h
    repositories/base/base_spawn_conditions_repository.h
    repositories/base/base_spawn_events_repository.h
    repositories/base/base_spawnentry_repository.h
    repositories/base/base_spawngroup_repository.h
    repositories/base/base_spell_buckets_repository.h
    repositories/base/base_spell_globals_repository.h
    repositories/base/base_spells_new_repository.h
    repositories/base/base_start_zones_repository.h
    repositories/base/base_starting_items_repository.h
    repositories/base/base_task_activities_repository.h
    repositories/base/base_tasks_repository.h
    repositories/base/base_tasksets_repository.h
    repositories/base/base_timers_repository.h
    repositories/base/base_titles_repository.h
    repositories/base/base_trader_repository.h
    repositories/base/base_tradeskill_recipe_entries_repository.h
    repositories/base/base_tradeskill_recipe_repository.h
    repositories/base/base_traps_repository.h
    repositories/base/base_tribute_levels_repository.h
    repositories/base/base_tributes_repository.h
    repositories/base/base_veteran_reward_templates_repository.h
    repositories/base/base_zone_points_repository.h
    repositories/base/base_zone_repository.h

    # Extended Repositories
    repositories/aa_ability_repository.h
    repositories/aa_rank_effects_repository.h
    repositories/aa_rank_prereqs_repository.h
    repositories/aa_ranks_repository.h
    repositories/account_flags_repository.h
    repositories/account_ip_repository.h
    repositories/account_repository.h
    repositories/account_rewards_repository.h
    repositories/adventure_details_repository.h
    repositories/adventure_members_repository.h
    repositories/adventure_stats_repository.h
    repositories/adventure_template_entry_flavor_repository.h
    repositories/adventure_template_entry_repository.h
    repositories/adventure_template_repository.h
    repositories/alternate_currency_repository.h
    repositories/auras_repository.h
    repositories/base_data_repository.h
    repositories/blocked_spells_repository.h
    repositories/books_repository.h
    repositories/bug_reports_repository.h
    repositories/bugs_repository.h
    repositories/buyer_buy_lines_repository.h
    repositories/buyer_trade_items_repository.h
    repositories/char_create_combinations_repository.h
    repositories/char_create_point_allocations_repository.h
    repositories/char_recipe_list_repository.h
    repositories/character_activities_repository.h
    repositories/character_alt_currency_repository.h
    repositories/character_alternate_abilities_repository.h
    repositories/character_auras_repository.h
    repositories/character_bandolier_repository.h
    repositories/character_bind_repository.h
    repositories/character_buffs_repository.h
    repositories/character_corpse_items_repository.h
    repositories/character_corpses_repository.h
    repositories/character_currency_repository.h
    repositories/character_data_repository.h
    repositories/character_disciplines_repository.h
    repositories/character_evolving_items_repository.h
    repositories/character_exp_modifiers_repository.h
    repositories/character_expedition_lockouts_repository.h
    repositories/character_inspect_messages_repository.h
    repositories/character_instance_safereturns_repository.h
    repositories/character_item_recast_repository.h
    repositories/character_languages_repository.h
    repositories/character_leadership_abilities_repository.h
    repositories/character_material_repository.h
    repositories/character_memmed_spells_repository.h
    repositories/character_parcels_containers_repository.h
    repositories/character_parcels_repository.h
    repositories/character_peqzone_flags_repository.h
    repositories/character_pet_buffs_repository.h
    repositories/character_pet_info_repository.h
    repositories/character_pet_inventory_repository.h
    repositories/character_potionbelt_repository.h
    repositories/character_skills_repository.h
    repositories/character_spells_repository.h
    repositories/character_task_timers_repository.h
    repositories/character_tasks_repository.h
    repositories/chatchannel_reserved_names_repository.h
    repositories/chatchannels_repository.h
    repositories/completed_shared_task_activity_state_repository.h
    repositories/completed_shared_task_members_repository.h
    repositories/completed_shared_tasks_repository.h
    repositories/completed_tasks_repository.h
    repositories/content_flags_repository.h
    repositories/damageshieldtypes_repository.h
    repositories/data_buckets_repository.h
    repositories/db_str_repository.h
    repositories/discord_webhooks_repository.h
    repositories/discovered_items_repository.h
    repositories/doors_repository.h
    repositories/dynamic_zone_lockouts_repository.h
    repositories/dynamic_zone_members_repository.h
    repositories/dynamic_zone_templates_repository.h
    repositories/dynamic_zones_repository.h
    repositories/faction_association_repository.h
    repositories/faction_base_data_repository.h
    repositories/faction_list_mod_repository.h
    repositories/faction_list_repository.h
    repositories/faction_values_repository.h
    repositories/fishing_repository.h
    repositories/forage_repository.h
    repositories/friends_repository.h
    repositories/global_loot_repository.h
    repositories/gm_ips_repository.h
    repositories/graveyard_repository.h
    repositories/ground_spawns_repository.h
    repositories/group_id_repository.h
    repositories/group_leaders_repository.h
    repositories/guild_bank_repository.h
    repositories/guild_members_repository.h
    repositories/guild_permissions_repository.h
    repositories/guild_ranks_repository.h
    repositories/guild_relations_repository.h
    repositories/guilds_repository.h
    repositories/horses_repository.h
    repositories/instance_list_player_repository.h
    repositories/instance_list_repository.h
    repositories/inventory_repository.h
    repositories/inventory_snapshots_repository.h
    repositories/ip_exemptions_repository.h
    repositories/items_evolving_details_repository.h
    repositories/items_repository.h
    repositories/ldon_trap_entries_repository.h
    repositories/ldon_trap_templates_repository.h
    repositories/level_exp_mods_repository.h
    repositories/lfguild_repository.h
    repositories/login_accounts_repository.h
    repositories/login_api_tokens_repository.h
    repositories/login_server_admins_repository.h
    repositories/login_server_list_types_repository.h
    repositories/login_world_servers_repository.h
    repositories/logsys_categories_repository.h
    repositories/lootdrop_entries_repository.h
    repositories/lootdrop_repository.h
    repositories/loottable_entries_repository.h
    repositories/loottable_repository.h
    repositories/mail_repository.h
    repositories/merchantlist_repository.h
    repositories/merchantlist_temp_repository.h
    repositories/name_filter_repository.h
    repositories/npc_emotes_repository.h
    repositories/npc_faction_entries_repository.h
    repositories/npc_faction_repository.h
    repositories/npc_scale_global_base_repository.h
    repositories/npc_spells_effects_entries_repository.h
    repositories/npc_spells_effects_repository.h
    repositories/npc_spells_entries_repository.h
    repositories/npc_spells_repository.h
    repositories/npc_types_repository.h
    repositories/npc_types_tint_repository.h
    repositories/object_contents_repository.h
    repositories/object_repository.h
    repositories/perl_event_export_settings_repository.h
    repositories/petitions_repository.h
    repositories/pets_beastlord_data_repository.h
    repositories/pets_equipmentset_entries_repository.h
    repositories/pets_equipmentset_repository.h
    repositories/pets_repository.h
    repositories/player_event_aa_purchase_repository.h
    repositories/player_event_killed_named_npc_repository.h
    repositories/player_event_killed_npc_repository.h
    repositories/player_event_killed_raid_npc_repository.h
    repositories/player_event_log_settings_repository.h
    repositories/player_event_logs_repository.h
    repositories/player_event_loot_items_repository.h
    repositories/player_event_merchant_purchase_repository.h
    repositories/player_event_merchant_sell_repository.h
    repositories/player_event_npc_handin_entries_repository.h
    repositories/player_event_npc_handin_repository.h
    repositories/player_event_speech_repository.h
    repositories/player_event_trade_entries_repository.h
    repositories/player_event_trade_repository.h
    repositories/player_titlesets_repository.h
    repositories/quest_globals_repository.h
    repositories/raid_details_repository.h
    repositories/raid_members_repository.h
    repositories/reports_repository.h
    repositories/respawn_times_repository.h
    repositories/rule_sets_repository.h
    repositories/rule_values_repository.h
    repositories/saylink_repository.h
    repositories/server_scheduled_events_repository.h
    repositories/shared_task_activity_state_repository.h
    repositories/shared_task_dynamic_zones_repository.h
    repositories/shared_task_members_repository.h
    repositories/shared_tasks_repository.h
    repositories/skill_caps_repository.h
    repositories/spawn2_repository.h
    repositories/spawn_condition_values_repository.h
    repositories/spawn_conditions_repository.h
    repositories/spawn_events_repository.h
    repositories/spawnentry_repository.h
    repositories/spawngroup_repository.h
    repositories/spell_buckets_repository.h
    repositories/spell_globals_repository.h
    repositories/spells_new_repository.h
    repositories/start_zones_repository.h
    repositories/starting_items_repository.h
    repositories/task_activities_repository.h
    repositories/tasks_repository.h
    repositories/tasksets_repository.h
    repositories/timers_repository.h
    repositories/titles_repository.h
    repositories/trader_repository.h
    repositories/tradeskill_recipe_entries_repository.h
    repositories/tradeskill_recipe_repository.h
    repositories/traps_repository.h
    repositories/tribute_levels_repository.h
    repositories/tributes_repository.h
    repositories/veteran_reward_templates_repository.h
    repositories/zone_points_repository.h
    repositories/zone_repository.h
)

set(common_headers
    StackWalker/StackWalker.h
    additive_lagged_fibonacci_engine.h
    base_packet.h
    bazaar.h
    bodytypes.h
    cli/argh.h
    cli/eqemu_command_handler.h
    cli/terminal_color.hpp
    classes.h
    compression.h
    condition.h
    content/world_content_service.h
    crash.h
    crc16.h
    crc32.h
    cron/croncpp.h
    data_bucket.cpp
    data_verification.h
    database.h
    database/database_dump_service.h
    database/database_update.h
    database/database_update_manifest.h
    database/database_update_manifest_bots.h
    database/database_update_manifest_custom.h
    database_schema.h
    dbcore.h
    deity.h
    discord/discord.h
    discord/discord_manager.h
    dynamic_zone_base.h
    dynamic_zone_lockout.h
    emu_constants.h
    emu_limits.h
    emu_opcodes.h
    emu_oplist.h
    emu_versions.h
    eq_constants.h
    eq_limits.h
    eq_packet.h
    eq_packet_structs.h
    eq_stream_ident.h
    eq_stream_intf.h
    eq_stream_locator.h
    eq_stream_proxy.h
    eqdb.h
    eqdb_res.h
    eqemu_config.h
    eqemu_config_elements.h
    eqemu_exception.h
    eqemu_logsys.h
    eqemu_logsys_log_aliases.h
    eqtime.h
    event/event_loop.h
    event/task.h
    event/timer.h
    event_sub.h
    events/player_event_discord_formatter.h
    events/player_event_logs.h
    events/player_events.h
    evolving_items.h
    extprofile.h
    faction.h
    features.h
    file.h
    fixed_memory_hash_set.h
    fixed_memory_variable_hash_set.h
    global_define.h
    guild_base.h
    guilds.h
    http/httplib.h
    http/uri.h
    inventory_profile.h
    inventory_slot.h
    ip_util.h
    ipc_mutex.h
    item_data.h
    item_instance.h
    json/json-forwards.h
    json/json.h
    json/json_archive_single_line.h
    json_config.h
    light_source.h
    linked_list.h
    loot.h
    mail_oplist.h
    md5.h
    memory_buffer.h
    memory_mapped_file.h
    misc.h
    misc_functions.h
    mutex.h
    mysql_request_result.h
    mysql_request_row.h
    mysql_stmt.h
    net/console_server.h
    net/console_server_connection.h
    net/crc32.h
    net/dns.h
    net/endian.h
    net/eqstream.h
    net/packet.h
    net/reliable_stream_connection.h
    net/reliable_stream_pooling.h
    net/reliable_stream_structs.h
    net/servertalk_client_connection.h
    net/servertalk_common.h
    net/servertalk_legacy_client_connection.h
    net/servertalk_server.h
    net/servertalk_server_connection.h
    net/tcp_connection.h
    net/tcp_connection_pooling.h
    net/tcp_server.h
    net/websocket_server.h
    net/websocket_server_connection.h
    op_codes.h
    opcode_dispatch.h
    opcodemgr.h
    packet_dump.h
    packet_dump_file.h
    packet_functions.h
    patches/patches.h
    patches/rof.h
    patches/rof2.h
    patches/rof2_limits.h
    patches/rof2_ops.h
    patches/rof2_structs.h
    patches/rof_limits.h
    patches/rof_ops.h
    patches/rof_structs.h
    patches/sod.h
    patches/sod_limits.h
    patches/sod_ops.h
    patches/sod_structs.h
    patches/sof.h
    patches/sof_limits.h
    patches/sof_ops.h
    patches/sof_structs.h
    patches/ss_declare.h
    patches/ss_define.h
    patches/ss_register.h
    patches/titanium.h
    patches/titanium_limits.h
    patches/titanium_ops.h
    patches/titanium_structs.h
    patches/uf.h
    patches/uf_limits.h
    patches/uf_ops.h
    patches/uf_structs.h
    path_manager.cpp
    platform.h
    proc_launcher.h
    process.h
    process/process.h
    profanity_manager.h
    profiler.h
    ptimer.h
    queue.h
    races.h
    raid.h
    random.h
    rdtsc.h
    rulesys.h
    ruletypes.h
    say_link.h
    seperator.h
    serialize_buffer.h
    server_event_scheduler.h
    server_reload_types.h
    serverinfo.h
    servertalk.h
    shared_tasks.h
    shareddb.h
    skill_caps.h
    skills.h
    spdat.h
    stacktrace/backward.hpp
    strings.h
    struct_strategy.h
    tasks.h
    termcolor/rang.hpp
    textures.h
    timer.h
    types.h
    unix.h
    useperl.h
    util/directory.h
    util/memory_stream.h
    util/uuid.h
    version.h
    zone_store.h
)

# Source Groups (Regex based for automatic subdirectory handling)
source_group("CLI" REGULAR_EXPRESSION "^cli/")
source_group("Content" REGULAR_EXPRESSION "^content/")
source_group("Cron" REGULAR_EXPRESSION "^cron/")
source_group("Database" REGULAR_EXPRESSION "^database/")
source_group("Discord" REGULAR_EXPRESSION "^discord/")
source_group("Event" REGULAR_EXPRESSION "^event/")
source_group("Events" REGULAR_EXPRESSION "^events/")
source_group("Http" REGULAR_EXPRESSION "^http/")
source_group("Json" REGULAR_EXPRESSION "^json/")
source_group("Memory" REGULAR_EXPRESSION "^memory/")
source_group("Net" REGULAR_EXPRESSION "^net/")
source_group("Patches" REGULAR_EXPRESSION "^patches/")
source_group("Process" REGULAR_EXPRESSION "^process/")
source_group("Repositories" REGULAR_EXPRESSION "^repositories/")
source_group("StackWalker" REGULAR_EXPRESSION "^StackWalker/")
source_group("Stacktrace" REGULAR_EXPRESSION "^stacktrace/")
source_group("Termcolor" REGULAR_EXPRESSION "^termcolor/")
source_group("Util" REGULAR_EXPRESSION "^util/")

option(EQEMU_ADD_PROFILER "Link with Google perftools profiler" OFF)
#PRNG options
option(EQEMU_ADDITIVE_LFIB_PRNG "Use Additive LFib for PRNG." OFF)
mark_as_advanced(EQEMU_ADDITIVE_LFIB_PRNG)
option(EQEMU_BIASED_INT_DIST "Use biased int dist instead of uniform." OFF)
mark_as_advanced(EQEMU_BIASED_INT_DIST)
set(EQEMU_CUSTOM_PRNG_ENGINE "" CACHE STRING "Custom random engine. (ex. std::default_random_engine)")
mark_as_advanced(EQEMU_CUSTOM_PRNG_ENGINE)

if(CMAKE_COMPILER_IS_GNUCXX)
    option(EQEMU_SFMT19937 "Use GCC's extention for SIMD Fast MT19937." OFF)
    mark_as_advanced(EQEMU_SFMT19937)
endif()

include_directories(Patches SocketLib StackWalker)

add_library(common ${common_sources} ${common_headers} ${repositories})

target_compile_definitions(common PUBLIC BOOST_BIND_GLOBAL_PLACEHOLDERS GLM_FORCE_RADIANS GLM_FORCE_CTOR_INIT GLM_ENABLE_EXPERIMENTAL ENABLE_SECURITY)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../submodules/websocketpp")
target_link_libraries(common PUBLIC cereal::cereal fmt::fmt unofficial::libmariadb $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv> OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB unofficial-sodium::sodium)

# Requires libgoogle-perftools-dev google-perftools packages for linux (debian)
if(EQEMU_ADD_PROFILER)
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed,-lprofiler,--as-needed")
endif()

if(EQEMU_ADDITIVE_LFIB_PRNG)
    target_compile_definitions(common PUBLIC USE_ADDITIVE_LFIB_PRNG)
    if(EQEMU_SFMT19937)
        message(STATUS "SFMT19937 and ADDITITVE_LFIB_PRNG both set, SFMT19937 ignored.")
        set(EQEMU_SFMT19937 OFF)
    endif()
    if(NOT EQEMU_CUSTOM_PRNG_ENGINE STREQUAL "")
        message(STATUS "CUSTOM_PRNG_ENGINE and ADDITITVE_LFIB_PRNG both set, CUSTOM_PRNG_ENGINE ignored.")
        set(EQEMU_CUSTOM_PRNG_ENGINE "")
    endif()
endif()

if(EQEMU_SFMT19937)
    target_compile_definitions(common PUBLIC USE_SFMT19937)
    if(NOT EQEMU_CUSTOM_PRNG_ENGINE STREQUAL "")
        message(STATUS "CUSTOM_PRNG_ENGINE and SFMT19937 both set, CUSTOM_PRNG_ENGINE ignored.")
        set(EQEMU_CUSTOM_PRNG_ENGINE "")
    endif()
endif()

if(NOT EQEMU_CUSTOM_PRNG_ENGINE STREQUAL "")
    target_compile_definitions(common PUBLIC USE_CUSTOM_PRNG_ENGINE=${EQEMU_CUSTOM_PRNG_ENGINE})
endif()

if(EQEMU_BIASED_INT_DIST)
    target_compile_definitions(common PUBLIC BIASED_INT_DIST)
endif()

if(${OPENSSL_VERSION} VERSION_GREATER_EQUAL "1.1.1")
    target_compile_definitions(common PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)
endif()

if(UNIX)
    target_link_libraries(common PUBLIC uuid)
    set_source_files_properties("SocketLib/Mime.cpp" PROPERTY COMPILE_FLAGS -Wno-unused-result)
    set_source_files_properties("patches/sod.cpp" "patches/sof.cpp" "patches/rof.cpp" "patches/rof2.cpp" "patches/uf.cpp" PROPERTIES COMPILE_FLAGS -O0)
endif()

if(EQEMU_BUILD_PCH)
    TARGET_PRECOMPILE_HEADERS(common PRIVATE pch/std-pch.h)
    # Avoid PCH/__OPTIMIZE__ mismatch when compiling certain patch sources with -O0
    # These files are compiled with -O0 on UNIX (see COMPILE_FLAGS above), which
    # disables the __OPTIMIZE__ predefined macro. Disabling PCH for them prevents
    # Clang from erroring due to macro state differences between the PCH and TU.
    set_source_files_properties("patches/sod.cpp" "patches/sof.cpp" "patches/rof.cpp" "patches/rof2.cpp" "patches/uf.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET common PROPERTY FOLDER libraries)
