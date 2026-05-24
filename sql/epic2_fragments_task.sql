
INSERT INTO tasks (
  id, type, duration, duration_code, title, description, reward_text, reward_id_list, cash_reward, exp_reward, reward_method, reward_points, reward_point_type, min_level, max_level, level_spread, min_players, max_players, repeatable, faction_reward, completion_emote, replay_timer_group, replay_timer_seconds, request_timer_group, request_timer_seconds, dz_template_id, lock_activity_id, faction_amount, enabled
) VALUES (
  990100, 0, 0, 0, 'Epic 2.0: Fragments of War', 'Collect five fragments from each of the seven Omens war zones and return them to the Epic Quest NPC in Dranik Scar.', 'Return to the Epic Quest NPC for your reward.', '', 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, '', 0, 0, 0, 0, 0, -1, 0, 1
);

INSERT INTO task_activities (
  taskid, activityid, req_activity_id, step, activitytype, target_name, goalmethod, goalcount, description_override, npc_match_list, item_id_list, item_list, dz_switch_id, min_x, min_y, min_z, max_x, max_y, max_z, skill_list, spell_list, zones, zone_version, optional, list_group
) VALUES
  (990100, 0, -1, 1, 3, 'Scarred Dranik War Fragment', 15, 5, '', '', '990100', 'Scarred Dranik War Fragment', 0, 0, 0, 0, 0, 0, 0, '', '', '', -1, 0, 0),
  (990100, 1, -1, 2, 3, 'Noble Causeway Battle Fragment', 15, 5, '', '', '990101', 'Noble Causeway Battle Fragment', 0, 0, 0, 0, 0, 0, 0, '', '', '', -1, 0, 0),
  (990100, 2, -1, 3, 3, 'Bloodfields War Fragment', 15, 5, '', '', '990102', 'Bloodfields War Fragment', 0, 0, 0, 0, 0, 0, 0, '', '', '', -1, 0, 0),
  (990100, 3, -1, 4, 3, 'Ruined Dranik Relic Fragment', 15, 5, '', '', '990103', 'Ruined Dranik Relic Fragment', 0, 0, 0, 0, 0, 0, 0, '', '', '', -1, 0, 0),
  (990100, 4, -1, 5, 3, 'Harbinger Spire Relic Fragment', 15, 5, '', '', '990104', 'Harbinger Spire Relic Fragment', 0, 0, 0, 0, 0, 0, 0, '', '', '', -1, 0, 0),
  (990100, 5, -1, 6, 3, 'Wall of Slaughter War Fragment', 15, 5, '', '', '990105', 'Wall of Slaughter War Fragment', 0, 0, 0, 0, 0, 0, 0, '', '', '', -1, 0, 0),
  (990100, 6, -1, 7, 3, 'Riftseeker Sanctum Relic Fragment', 15, 5, '', '', '990106', 'Riftseeker Sanctum Relic Fragment', 0, 0, 0, 0, 0, 0, 0, '', '', '', -1, 0, 0);
