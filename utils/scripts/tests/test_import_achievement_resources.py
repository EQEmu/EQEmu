import importlib.util
from pathlib import Path
import re
import sys
import unittest
from unittest import mock


SCRIPT_PATH = Path(__file__).parents[1] / "import_achievement_resources.py"
SPEC = importlib.util.spec_from_file_location(
    "import_achievement_resources", SCRIPT_PATH
)
assert SPEC is not None and SPEC.loader is not None
IMPORTER = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = IMPORTER
SPEC.loader.exec_module(IMPORTER)


def category(category_id, parent_id, name):
    return IMPORTER.Category(category_id, parent_id, 0, name, "", "")


def achievement(achievement_id, name, description=""):
    return IMPORTER.Achievement(
        achievement_id, name, description, 0, 0, False, 0
    )


def association(category_id, achievement_id):
    return IMPORTER.CategoryAssociation(category_id, 0, achievement_id)


def component(achievement_id, sequence, component_type, component_id, description):
    return IMPORTER.Component(
        achievement_id,
        sequence,
        component_type,
        component_id,
        description,
    )


def level_components(level):
    return (
        component(level, 1, 1, level, f"Reach Level {level}"),
        component(
            level,
            2,
            3,
            1600 + level,
            f"Meet or Exceed the Level Cap ({level})",
        ),
    )


class EnableSelectionTests(unittest.TestCase):
    def test_npc_name_hash_matches_runtime_golden_vectors(self):
        vectors = {
            "Vishimtar_the_Fallen00": 0x708BEE77,
            "  VISHIMTAR__the   Fallen 99 ": 0x708BEE77,
            "Tunare`s_Guardian00": 0xCF16724E,
            "Tunare's Guardian": 0xCF16724E,
            "#A_Rat_01": 0x40FF2A77,
            " 123_#- ": 0,
            "\N{LATIN CAPITAL LETTER E WITH ACUTE}": 0,
        }
        for name, expected in vectors.items():
            with self.subTest(name=name):
                self.assertEqual(IMPORTER.npc_name_hash(name), expected)

    def setUp(self):
        self.resources = IMPORTER.ResourceSet(
            categories=(
                category(10, 0, "General"),
                category(13, 10, "Level"),
                category(100, 0, "EverQuest"),
                category(101, 100, "General"),
                category(200, 0, "Ruins of Kunark"),
                category(201, 200, "Raids"),
            ),
            achievements=(
                achievement(5, "Level 5"),
                achievement(60, "Level 60"),
                achievement(65, "Level 65"),
                achievement(70, "Level 70"),
                achievement(75, "Level 75"),
                achievement(1000, "Classic content"),
                achievement(2000, "Kunark content"),
                achievement(3000, "Later content"),
            ),
            category_associations=(
                association(13, 5),
                association(13, 60),
                association(13, 65),
                association(13, 70),
                association(13, 75),
                association(101, 1000),
                association(201, 2000),
            ),
            components=(
                level_components(5)
                + level_components(60)
                + level_components(65)
                + level_components(70)
                + level_components(75)
            ),
            component_counts=(),
        )

    def test_expansion_selection_is_cumulative_and_uses_level_cap(self):
        selection = IMPORTER.build_enable_selection(
            self.resources, through_expansion="RoK"
        )

        self.assertEqual(selection.through_expansion.key, "kunark")
        self.assertEqual(selection.max_level, 60)
        self.assertEqual(selection.expansion_achievement_ids, (1000, 2000))
        self.assertEqual(selection.level_achievement_ids, (5, 60))
        self.assertEqual(selection.achievement_ids, {5, 60, 1000, 2000})
        self.assertEqual(
            selection.level_criteria,
            (
                IMPORTER.LevelCriterion(5, 1, 1, 5, 5),
                IMPORTER.LevelCriterion(60, 1, 1, 60, 60),
            ),
        )

    def test_max_level_overrides_expansion_default(self):
        selection = IMPORTER.build_enable_selection(
            self.resources,
            through_expansion="kunark",
            max_level=5,
        )

        self.assertEqual(selection.level_achievement_ids, (5,))

    def test_sql_enables_selected_rows_without_disabling_existing_later_rows(self):
        selection = IMPORTER.build_enable_selection(
            self.resources, through_expansion="kunark"
        )
        sql = IMPORTER.generate_sql(
            self.resources, enable_selection=selection
        )

        self.assertIn(
            "UPDATE `achievements` SET `enabled` = 1 WHERE `id` IN "
            "(5, 60, 1000, 2000);",
            sql,
        )
        self.assertIn(
            "INSERT INTO `achievement_criteria` "
            "(`achievement_id`, `component_type`, `component_sequence`, "
            "`component_id`, `event_type`, `progress_mode`, `behavior`, "
            "`target_id`, `target_id2`, `target_value`, `required_count`, "
            "`enabled`) VALUES",
            sql,
        )
        self.assertIn("(5, 1, 1, 5, 1, 3, 0, 0, 0, 5, 1, 1),", sql)
        self.assertIn("(60, 1, 1, 60, 1, 3, 0, 0, 0, 60, 1, 1)", sql)
        criteria_sql = sql.split(
            "INSERT INTO `achievement_criteria`", maxsplit=1
        )[1].split("COMMIT;", maxsplit=1)[0]
        self.assertNotIn("(1000,", criteria_sql)
        self.assertNotIn("(2000,", criteria_sql)
        self.assertNotIn("DELETE", criteria_sql)
        replace_sql = IMPORTER.generate_sql(
            self.resources,
            replace_existing=True,
            enable_selection=selection,
        )
        self.assertNotIn(
            "DELETE target FROM `achievement_criteria`",
            replace_sql,
        )
        achievement_upsert = sql.split(
            "INSERT INTO `achievements`", maxsplit=1
        )[1].split("INSERT INTO `achievement_category_associations`", maxsplit=1)[0]
        self.assertIn("`has_reward`, `client_flag`", achievement_upsert)
        self.assertNotIn("`enabled` = VALUES(`enabled`)", achievement_upsert)
        self.assertIn("3000", achievement_upsert)
        self.assertIn("`component_id`, `name`, `description`", sql)

    def test_exact_enable_selection_disables_only_unselected_imported_ids(self):
        selection = IMPORTER.build_enable_selection(
            self.resources, through_expansion="kunark"
        )
        sql = IMPORTER.generate_sql(
            self.resources,
            enable_selection=selection,
            exact_enable_selection=True,
        )

        self.assertIn(
            "UPDATE `achievements` SET `enabled` = 0 WHERE `id` IN "
            "(65, 70, 75, 3000);",
            sql,
        )
        self.assertIn(
            "UPDATE `achievements` SET `enabled` = 1 WHERE `id` IN "
            "(5, 60, 1000, 2000);",
            sql,
        )
        self.assertNotIn("NOT IN", sql)
        self.assertIn("Custom IDs are untouched.", sql)
        self.assertNotIn(
            "existing definitions keep their state",
            sql,
        )

    def test_preserve_enable_state_generates_policy_without_enable_updates(self):
        selection = IMPORTER.build_enable_selection(
            self.resources, through_expansion="kunark"
        )
        sql = IMPORTER.generate_sql(
            self.resources,
            enable_selection=selection,
            preserve_enable_state=True,
        )

        self.assertNotIn(
            "UPDATE `achievements` SET `enabled`",
            sql,
        )
        self.assertIn(
            "existing definitions retain their current enabled state",
            sql,
        )
        self.assertIn(
            "(60, 1, 1, 60, 1, 3, 0, 0, 0, 60, 1, 1)",
            sql,
        )

    def test_preserve_enable_state_rejects_exact_selection(self):
        selection = IMPORTER.build_enable_selection(
            self.resources, through_expansion="kunark"
        )
        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            "cannot be combined with exact_enable_selection",
        ):
            IMPORTER.generate_sql(
                self.resources,
                enable_selection=selection,
                exact_enable_selection=True,
                preserve_enable_state=True,
            )

    def test_slayer_policy_warnings_explain_scope_and_activation(self):
        warnings = IMPORTER._selection_policy_warnings(
            enable_slayer=True,
            through_expansion="don",
            preserve_enable_state=True,
        )

        self.assertEqual(len(warnings), 2)
        self.assertIn(
            "--enable-through-expansion don does not constrain Slayer",
            warnings[0],
        )
        self.assertIn(
            "does not enable existing Slayer definitions",
            warnings[1],
        )
        self.assertEqual(
            IMPORTER._selection_policy_warnings(
                enable_slayer=False,
                through_expansion="don",
                preserve_enable_state=True,
            ),
            (),
        )

    def test_exact_enable_selection_requires_a_selection(self):
        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            "requires at least one progression selection option",
        ):
            IMPORTER.generate_sql(
                self.resources,
                exact_enable_selection=True,
            )

    def test_exact_enable_selection_rejects_global_replacement(self):
        selection = IMPORTER.build_enable_selection(
            self.resources, through_expansion="kunark"
        )
        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            "cannot be combined with replace_existing",
        ):
            IMPORTER.generate_sql(
                self.resources,
                replace_existing=True,
                enable_selection=selection,
                exact_enable_selection=True,
            )

    def test_default_sql_keeps_the_existing_disabled_import_policy(self):
        sql = IMPORTER.generate_sql(self.resources)

        self.assertIn("-- Newly inserted definitions are disabled;", sql)
        self.assertNotIn(
            "UPDATE `achievements` SET `enabled` = 1",
            sql,
        )
        self.assertNotIn("INSERT INTO `achievement_criteria`", sql)

    def test_max_level_generates_only_milestones_through_the_cap(self):
        selection = IMPORTER.build_enable_selection(
            self.resources, max_level=70
        )
        sql = IMPORTER.generate_sql(
            self.resources, enable_selection=selection
        )

        self.assertEqual(selection.level_achievement_ids, (5, 60, 65, 70))
        criteria_sql = sql.split(
            "INSERT INTO `achievement_criteria`", maxsplit=1
        )[1].split("COMMIT;", maxsplit=1)[0]
        self.assertIn("(70, 1, 1, 70, 1, 3, 0, 0, 0, 70, 1, 1)", criteria_sql)
        self.assertNotIn("(75, 1, 1, 75,", criteria_sql)
        self.assertIn(
            "`component_sequence` = VALUES(`component_sequence`)",
            criteria_sql,
        )
        self.assertIn("`enabled` = VALUES(`enabled`)", criteria_sql)

    def test_noncanonical_level_components_are_rejected(self):
        valid_five = level_components(5)
        invalid_cases = (
            (
                "missing state component",
                (valid_five[1],),
            ),
            (
                "component type",
                (
                    component(5, 1, 0, 5, "Reach Level 5"),
                    valid_five[1],
                ),
            ),
            (
                "component sequence",
                (
                    component(5, 2, 1, 5, "Reach Level 5"),
                    valid_five[1],
                ),
            ),
            (
                "component ID",
                (
                    component(5, 1, 1, 500, "Reach Level 5"),
                    valid_five[1],
                ),
            ),
            (
                "component description",
                (
                    component(5, 1, 1, 5, "Become Level 5"),
                    valid_five[1],
                ),
            ),
            (
                "additional state component",
                valid_five
                + (component(5, 3, 2, 500, "Another requirement"),),
            ),
        )

        for description, replacement in invalid_cases:
            with self.subTest(description=description):
                resources = IMPORTER.ResourceSet(
                    categories=self.resources.categories,
                    achievements=self.resources.achievements,
                    category_associations=self.resources.category_associations,
                    components=tuple(
                        row
                        for row in self.resources.components
                        if row.achievement_id != 5
                    )
                    + replacement,
                    component_counts=self.resources.component_counts,
                )
                with self.assertRaisesRegex(
                    IMPORTER.ResourceError,
                    "cannot safely generate a Level criterion",
                ):
                    IMPORTER.build_enable_selection(resources, max_level=5)

    def test_noncanonical_level_count_is_rejected(self):
        resources = IMPORTER.ResourceSet(
            categories=self.resources.categories,
            achievements=self.resources.achievements,
            category_associations=self.resources.category_associations,
            components=self.resources.components,
            component_counts=(IMPORTER.ComponentCount(5, 2),),
        )

        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            "cannot safely generate a Level criterion",
        ):
            IMPORTER.build_enable_selection(resources, max_level=5)

    def test_level_name_and_achievement_id_must_agree(self):
        resources = IMPORTER.ResourceSet(
            categories=self.resources.categories,
            achievements=tuple(
                row for row in self.resources.achievements
                if row.achievement_id != 5
            )
            + (achievement(500, "Level 5"),),
            category_associations=tuple(
                row for row in self.resources.category_associations
                if row.achievement_id != 5
            )
            + (association(13, 500),),
            components=tuple(
                row for row in self.resources.components
                if row.achievement_id != 5
            )
            + (
                component(500, 1, 1, 5, "Reach Level 5"),
                component(500, 2, 3, 1605, "Meet or Exceed the Level Cap (5)"),
            ),
            component_counts=self.resources.component_counts,
        )

        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            "cannot safely generate a Level criterion",
        ):
            IMPORTER.build_enable_selection(resources, max_level=5)

    def test_unknown_expansion_is_rejected(self):
        with self.assertRaisesRegex(
            IMPORTER.ResourceError, "unknown expansion"
        ):
            IMPORTER.build_enable_selection(
                self.resources, through_expansion="not-an-expansion"
            )

    def test_unparseable_level_milestone_is_rejected(self):
        resources = IMPORTER.ResourceSet(
            categories=self.resources.categories,
            achievements=self.resources.achievements
            + (achievement(999, "Maximum Level"),),
            category_associations=self.resources.category_associations
            + (association(13, 999),),
            components=self.resources.components,
            component_counts=self.resources.component_counts,
        )

        with self.assertRaisesRegex(
            IMPORTER.ResourceError, "cannot determine the level"
        ):
            IMPORTER.build_enable_selection(resources, max_level=70)


class GeneratedCriteriaTests(unittest.TestCase):
    @staticmethod
    def base_categories():
        return (
            category(10, 0, "General"),
            category(13, 10, "Level"),
            category(100, 0, "EverQuest"),
            category(102, 100, "Exploration"),
            category(106, 100, "Raids"),
            category(108, 100, "Hunter"),
        )

    @staticmethod
    def level_policy_collision_resources():
        return IMPORTER.ResourceSet(
            categories=(
                category(10, 0, "General"),
                category(13, 10, "Level"),
                category(14, 10, "Skills"),
                category(18, 10, "Progression"),
            ),
            achievements=(
                achievement(
                    60,
                    "Level 60",
                    "This achievement is completed by reaching level 60.",
                ),
                achievement(10050, "Reach Level 60", "Reach Level 60"),
                achievement(
                    20160,
                    "Warlord's Combat Proficiency, Level 60",
                    "This achievement is completed by reaching the maximum "
                    "skill in all combat skills at level 60.",
                ),
            ),
            category_associations=(
                association(13, 60),
                association(18, 10050),
                association(14, 20160),
            ),
            components=(
                component(60, 1, 1, 60, "Reach Level 60"),
                component(
                    60,
                    2,
                    3,
                    1660,
                    "Meet or Exceed the Level Cap (60)",
                ),
                component(10050, 0, 1, 10050, "Reach level 60"),
                component(
                    10050,
                    0,
                    3,
                    10014,
                    "On a Level Locked Server",
                ),
                component(
                    20160,
                    0,
                    3,
                    1303,
                    "Meet or Exceed the Class Requirement (Warrior)",
                ),
                component(
                    20160,
                    1,
                    1,
                    150060,
                    "Reach the maximum skill in 1H Blunt at level 60.",
                ),
            ),
            component_counts=(),
        )

    def selection_for(
        self,
        achievements,
        components,
        *,
        associations=None,
        component_counts=(),
        categories=None,
    ):
        if associations is None:
            associations = tuple(
                association(102, row.achievement_id)
                for row in achievements
            )
        resources = IMPORTER.ResourceSet(
            categories=categories or self.base_categories(),
            achievements=tuple(achievements),
            category_associations=tuple(associations),
            components=tuple(components),
            component_counts=tuple(component_counts),
        )
        return (
            resources,
            IMPORTER.build_enable_selection(
                resources,
                through_expansion="classic",
                max_level=1,
            ),
        )

    def test_leaf_traveler_and_mpg_trials_generate_zone_enter_criteria(self):
        achievements = (
            achievement(1033700, "The Broodlands Traveler"),
            achievement(
                930400,
                "Muramite Proving Grounds Trials Traveler",
            ),
        )
        components = (
            component(
                1033700, 1, 1, 270276, "Visit The Broodlands"
            ),
        ) + tuple(
            component(
                930400,
                sequence,
                1,
                270255 + sequence,
                f"Visit Trial {sequence}",
            )
            for sequence in range(1, 7)
        )
        resources, selection = self.selection_for(
            achievements, components
        )

        traveler = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type == IMPORTER.EVENT_ZONE_ENTER
        )
        self.assertEqual(
            tuple(row.target_id for row in traveler),
            (304, 305, 306, 307, 308, 309, 337),
        )
        self.assertTrue(
            all(
                row.progress_mode == IMPORTER.PROGRESS_BOOLEAN
                and row.behavior == IMPORTER.BEHAVIOR_REQUIRED
                and row.required_count == 1
                for row in traveler
            )
        )
        self.assertEqual(selection.criteria_report.traveler_candidates, 2)
        self.assertEqual(selection.criteria_report.traveler_generated, 7)
        self.assertEqual(selection.criteria_report.traveler_rejected, 0)
        sql = IMPORTER.generate_sql(
            resources, enable_selection=selection
        )
        self.assertIn(
            "(1033700, 1, 1, 270276, 5, 3, 0, 337, 0, 0, 1, 1)",
            sql,
        )

    def test_bad_traveler_shapes_are_preserved_without_criteria(self):
        cases = (
            (
                achievement(1033701, "Bad ID Traveler"),
                (component(1033701, 1, 1, 1, "Visit Bad ID"),),
                (),
            ),
            (
                achievement(1033700, "Bad Text Traveler"),
                (component(1033700, 1, 1, 1, "Explore Bad Text"),),
                (),
            ),
            (
                achievement(1033700, "Bad Type Traveler"),
                (component(1033700, 1, 2, 1, "Visit Bad Type"),),
                (),
            ),
            (
                achievement(1033700, "Bad Count Traveler"),
                (component(1033700, 1, 1, 1, "Visit Bad Count"),),
                (IMPORTER.ComponentCount(1, 2),),
            ),
            (
                achievement(1033700, "Too Many Traveler"),
                (
                    component(1033700, 1, 1, 1, "Visit One"),
                    component(1033700, 2, 1, 2, "Visit Two"),
                ),
                (),
            ),
        )

        for row, components, counts in cases:
            with self.subTest(name=row.name):
                _resources, selection = self.selection_for(
                    (row,),
                    components,
                    component_counts=counts,
                )
                self.assertFalse(
                    any(
                        criterion.event_type
                        == IMPORTER.EVENT_ZONE_ENTER
                        for criterion in selection.generated_criteria
                    )
                )
                self.assertEqual(
                    selection.criteria_report.traveler_rejected, 1
                )

    def test_exact_dependencies_use_required_and_optional_semantics(self):
        achievements = (
            achievement(1000, "Parent"),
            achievement(1001, "Required Child"),
            achievement(1002, "Optional Child"),
            achievement(1003, "Duplicate"),
            achievement(1004, "Duplicate"),
            achievement(1005, "Self"),
            achievement(1006, "Counted Child"),
        )
        components = (
            component(1000, 1, 1, 2001, "Required Child"),
            component(1000, 2, 2, 2002, "Optional Child"),
            component(1000, 3, 1, 2003, "Duplicate"),
            component(1000, 4, 1, 2005, "Counted Child"),
            component(1005, 1, 1, 2004, "Self"),
        )
        _resources, selection = self.selection_for(
            achievements,
            components,
            component_counts=(IMPORTER.ComponentCount(2005, 2),),
        )

        dependencies = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type == IMPORTER.EVENT_ACHIEVEMENT_COMPLETE
        )
        self.assertEqual(
            tuple(
                (row.target_id, row.behavior)
                for row in dependencies
            ),
            (
                (1001, IMPORTER.BEHAVIOR_REQUIRED),
                (1002, IMPORTER.BEHAVIOR_OPTIONAL),
            ),
        )
        self.assertEqual(selection.criteria_report.dependency_ambiguous, 1)
        self.assertEqual(selection.criteria_report.dependency_self, 1)
        self.assertEqual(selection.criteria_report.dependency_rejected, 1)

    def test_dependency_cycles_are_rejected(self):
        achievements = (
            achievement(1000, "First"),
            achievement(1001, "Second"),
        )
        components = (
            component(1000, 1, 1, 2000, "Second"),
            component(1001, 1, 1, 2001, "First"),
        )

        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            "dependency cycle: 1000 -> 1001 -> 1000",
        ):
            self.selection_for(achievements, components)

    def test_tradeskill_milestones_require_explicit_opt_in_and_exact_shape(self):
        categories = self.base_categories() + (
            category(50, 0, "Tradeskill"),
            category(52, 50, "Baking"),
        )
        achievements = (
            achievement(
                560050,
                "Baking (50)",
                "This achievement is completed by reaching 50 skill in Baking.",
            ),
            achievement(
                560100,
                "Baking (100)",
                "This achievement is completed by reaching 100 skill in Baking.",
            ),
            achievement(
                560150,
                "Baking (150)",
                "This achievement is completed by reaching 150 skill in Baking.",
            ),
            achievement(999999, "Baking Quest"),
        )
        associations = tuple(
            association(52, row.achievement_id)
            for row in achievements
        )
        components = tuple(
            component(
                row.achievement_id,
                1,
                1,
                row.achievement_id,
                f"Reach {value} skill in Baking",
            )
            for row, value in zip(achievements[:3], (50, 100, 150))
        )
        resources = IMPORTER.ResourceSet(
            categories=categories,
            achievements=achievements,
            category_associations=associations,
            components=components,
            component_counts=(),
        )

        ordinary = IMPORTER.build_enable_selection(
            resources,
            through_expansion="classic",
            max_level=1,
        )
        self.assertEqual(ordinary.tradeskill_achievement_ids, ())
        selected = IMPORTER.build_enable_selection(
            resources,
            max_tradeskill_skill=100,
        )
        self.assertEqual(
            selected.tradeskill_achievement_ids,
            (560050, 560100),
        )
        skill_criteria = tuple(
            row
            for row in selected.generated_criteria
            if row.event_type == IMPORTER.EVENT_SKILL_VALUE
        )
        self.assertEqual(
            tuple((row.target_id, row.target_value) for row in skill_criteria),
            ((60, 50), (60, 100)),
        )
        sql = IMPORTER.generate_sql(
            resources, enable_selection=selected
        )
        self.assertIn(
            "(560050, 1, 1, 560050, 9, 3, 0, 60, 0, 50, 1, 1)",
            sql,
        )
        self.assertNotIn(
            "UPDATE `achievements` SET `enabled` = 1 WHERE `id` IN "
            "(560050, 560100, 560150",
            sql,
        )

    def test_aa_spent_milestones_require_explicit_opt_in_and_exact_shape(self):
        categories = self.base_categories() + (
            category(11, 10, "Advancement"),
        )
        achievements = (
            achievement(
                100005,
                "5 Alternate Advancement Points",
                "This achievement is completed by spending 5 alternate "
                "advancement points.",
            ),
            achievement(
                100010,
                "10 Alternate Advancement Points",
                "This achievement is completed by spending 10 alternate "
                "advancement points.",
            ),
            achievement(
                100020,
                "20 Alternate Advancement Points",
                "This achievement is completed by spending 20 alternate "
                "advancement points.",
            ),
            achievement(
                100030,
                "30 Alternate Advancement Points",
                "This achievement is completed by earning 30 alternate "
                "advancement points.",
            ),
        )
        associations = tuple(
            association(11, row.achievement_id) for row in achievements
        )
        components = (
            component(
                100005,
                1,
                1,
                100005,
                "Spend 5 Alternate Advancement Points",
            ),
            component(
                100010,
                1,
                1,
                100010,
                "Spend 10 Alternate Advancement Points",
            ),
            component(
                100020,
                1,
                1,
                100020,
                "Spend 20 Alternate Advancement Points",
            ),
            component(
                100030,
                1,
                1,
                100030,
                "Spend 30 Alternate Advancement Points",
            ),
        )
        resources = IMPORTER.ResourceSet(
            categories=categories,
            achievements=achievements,
            category_associations=associations,
            components=components,
            component_counts=(),
        )

        ordinary = IMPORTER.build_enable_selection(
            resources,
            through_expansion="classic",
            max_level=1,
        )
        self.assertEqual(ordinary.aa_spent_achievement_ids, ())

        selected = IMPORTER.build_enable_selection(
            resources,
            max_aa_spent=20,
        )
        self.assertEqual(
            selected.aa_spent_achievement_ids,
            (100005, 100010, 100020),
        )
        aa_criteria = tuple(
            row
            for row in selected.generated_criteria
            if row.event_type == IMPORTER.EVENT_ALTERNATE_ADVANCEMENT
        )
        self.assertEqual(
            tuple(
                (row.target_id, row.target_value)
                for row in aa_criteria
            ),
            ((0, 5), (0, 10), (0, 20)),
        )
        self.assertEqual(selected.criteria_report.aa_spent_candidates, 3)
        self.assertEqual(selected.criteria_report.aa_spent_generated, 3)
        self.assertEqual(selected.criteria_report.aa_spent_rejected, 0)

        with_invalid = IMPORTER.build_enable_selection(
            resources,
            max_aa_spent=30,
        )
        self.assertEqual(
            with_invalid.aa_spent_achievement_ids,
            (100005, 100010, 100020),
        )
        self.assertEqual(with_invalid.criteria_report.aa_spent_candidates, 4)
        self.assertEqual(with_invalid.criteria_report.aa_spent_generated, 3)
        self.assertEqual(with_invalid.criteria_report.aa_spent_rejected, 1)

        sql = IMPORTER.generate_sql(
            resources,
            enable_selection=selected,
        )
        self.assertIn(
            "(100020, 1, 1, 100020, 10, 3, 0, 0, 0, 20, 1, 1)",
            sql,
        )

    def test_level_locked_and_class_skill_cap_milestones_follow_max_level(self):
        categories = (
            category(10, 0, "General"),
            category(13, 10, "Level"),
            category(14, 10, "Skills"),
            category(18, 10, "Progression"),
        )
        achievements = (
            achievement(10050, "Reach Level 60", "Reach Level 60"),
            achievement(
                20170,
                "Warlord's Combat Proficiency, Level 70",
                "This achievement is completed by reaching the maximum "
                "skill in all combat skills at level 70.",
            ),
            achievement(
                99970,
                "Broken Proficiency, Level 70",
                "Malformed test definition.",
            ),
        )
        resources = IMPORTER.ResourceSet(
            categories=categories,
            achievements=achievements,
            category_associations=(
                association(18, 10050),
                association(14, 20170),
                association(14, 99970),
            ),
            components=(
                component(10050, 0, 1, 10050, "Reach level 60"),
                component(
                    10050,
                    0,
                    3,
                    10014,
                    "On a Level Locked Server",
                ),
                component(
                    20170,
                    0,
                    3,
                    1303,
                    "Meet or Exceed the Class Requirement (Warrior)",
                ),
                component(
                    20170,
                    1,
                    1,
                    150070,
                    "Reach the maximum skill in 1H Blunt at level 70.",
                ),
                component(
                    20170,
                    2,
                    1,
                    151570,
                    "Reach the maximum skill in Defense at level 70.",
                ),
                component(
                    99970,
                    0,
                    3,
                    1303,
                    "Meet or Exceed the Class Requirement (Warrior)",
                ),
                component(
                    99970,
                    1,
                    1,
                    99971,
                    "Reach the maximum skill in Defense at level 70.",
                ),
            ),
            component_counts=(),
        )

        selection = IMPORTER.build_enable_selection(
            resources, max_level=70
        )

        self.assertEqual(
            selection.progression_level_achievement_ids, (10050,)
        )
        self.assertEqual(selection.skill_cap_achievement_ids, (20170,))
        self.assertIn(10050, selection.achievement_ids)
        self.assertIn(20170, selection.achievement_ids)
        self.assertNotIn(99970, selection.achievement_ids)
        skill_caps = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type == IMPORTER.EVENT_SKILL_CAP
        )
        self.assertEqual(
            tuple(
                (row.target_id, row.target_id2, row.target_value)
                for row in skill_caps
            ),
            ((0, 1, 70), (15, 1, 70)),
        )
        self.assertEqual(selection.criteria_report.skill_cap_candidates, 2)
        self.assertEqual(selection.criteria_report.skill_cap_rejected, 1)
        sql = IMPORTER.generate_sql(
            resources, enable_selection=selection
        )
        self.assertIn(
            "(20170, 1, 1, 150070, 13, 3, 0, 0, 1, 70, 1, 1)",
            sql,
        )
        self.assertIn(
            "(10050, 1, 0, 10050, 1, 3, 0, 0, 0, 60, 1, 1)",
            sql,
        )

    def test_specialized_level_policy_preempts_name_inferred_dependency(self):
        selection = IMPORTER.build_enable_selection(
            self.level_policy_collision_resources(),
            max_level=60,
        )

        policies_by_component = {}
        for criterion in selection.generated_criteria:
            component_identity = (
                criterion.achievement_id,
                criterion.component_type,
                criterion.component_id,
            )
            policy = (
                criterion.behavior,
                criterion.required_count,
                criterion.event_type,
                criterion.progress_mode,
            )
            if component_identity in policies_by_component:
                self.assertEqual(
                    policies_by_component[component_identity],
                    policy,
                    f"conflicting generated policy for {component_identity}",
                )
            else:
                policies_by_component[component_identity] = policy

        level_component_criteria = tuple(
            criterion
            for criterion in selection.generated_criteria
            if (
                criterion.achievement_id,
                criterion.component_type,
                criterion.component_id,
            )
            == (60, 1, 60)
        )
        self.assertEqual(len(level_component_criteria), 1)
        self.assertEqual(
            (
                level_component_criteria[0].event_type,
                level_component_criteria[0].target_id,
                level_component_criteria[0].target_id2,
                level_component_criteria[0].target_value,
            ),
            (IMPORTER.EVENT_LEVEL, 0, 0, 60),
        )
        self.assertFalse(
            any(
                criterion.achievement_id == 60
                and criterion.component_type == 1
                and criterion.component_id == 60
                and criterion.event_type
                == IMPORTER.EVENT_ACHIEVEMENT_COMPLETE
                and criterion.target_id == 10050
                for criterion in selection.generated_criteria
            )
        )
        self.assertEqual(selection.criteria_report.dependency_generated, 0)

    def test_sql_disables_exact_superseded_level_dependency_identity(self):
        resources = self.level_policy_collision_resources()
        selection = IMPORTER.build_enable_selection(resources, max_level=60)
        sql = IMPORTER.generate_sql(resources, enable_selection=selection)

        disable_statements = tuple(
            statement.strip()
            for statement in sql.split(";")
            if "UPDATE `achievement_criteria`" in statement
            and "SET `enabled` = 0" in statement
        )
        self.assertEqual(len(disable_statements), 1)
        disable_statement = disable_statements[0]
        self.assertIn(
            "(`achievement_id`, `component_type`, `component_id`, "
            "`event_type`, `target_id`, `target_id2`)",
            disable_statement,
        )
        self.assertIn("(60, 1, 60, 11, 10050, 0)", disable_statement)
        self.assertNotIn(
            "(60, 1, 1, 60, 11, 3, 0, 10050, 0, 0, 1, 1)",
            sql,
        )

    def test_generated_component_policy_conflicts_fail_before_sql(self):
        level = IMPORTER.EvaluationCriterion(
            achievement_id=60,
            component_type=1,
            component_sequence=1,
            component_id=60,
            event_type=IMPORTER.EVENT_LEVEL,
            progress_mode=IMPORTER.PROGRESS_BOOLEAN,
            behavior=IMPORTER.BEHAVIOR_REQUIRED,
            target_id=0,
            target_id2=0,
            target_value=60,
        )
        dependency = IMPORTER.EvaluationCriterion(
            achievement_id=60,
            component_type=1,
            component_sequence=1,
            component_id=60,
            event_type=IMPORTER.EVENT_ACHIEVEMENT_COMPLETE,
            progress_mode=IMPORTER.PROGRESS_BOOLEAN,
            behavior=IMPORTER.BEHAVIOR_REQUIRED,
            target_id=10050,
            target_id2=0,
            target_value=0,
        )

        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            "conflicting behavior, required-count, event, or progress-mode",
        ):
            IMPORTER._validated_component_policies((level, dependency))

    def test_key_and_epic_item_names_resolve_all_exact_item_ids_in_sql(self):
        resources = IMPORTER.ResourceSet(
            categories=(
                category(10, 0, "General"),
                category(12, 10, "Class"),
                category(17, 10, "Keys"),
            ),
            achievements=(
                achievement(
                    90000,
                    "Bone Crafted Key",
                    "This key unlocks New Paineel.",
                ),
                achievement(
                    1001,
                    "Epic 1.0",
                    "This achievement is completed by obtaining the "
                    "Warrior Epic 1.0.",
                ),
                achievement(
                    1015,
                    "Epic 1.0",
                    "This achievement is completed by obtaining the "
                    "Beastlord Epic 1.0.",
                ),
            ),
            category_associations=(
                association(17, 90000),
                association(12, 1001),
                association(12, 1015),
            ),
            components=(
                component(90000, 0, 1, 3000, "Bone Crafted Key"),
                component(
                    1001, 1, 1, 1016, "Obtain the Blade of Tactics"
                ),
                component(
                    1001, 2, 1, 1017, "Obtain the Blade of Strategy"
                ),
                component(
                    1001,
                    2,
                    3,
                    1303,
                    "Meet or Exceed the Class Requirement (Warrior)",
                ),
                component(
                    1015,
                    1,
                    1,
                    1001,
                    "Obtain the Claw of the Savage Spirit",
                ),
                component(
                    1015,
                    1,
                    1,
                    1002,
                    "Obtain the Claw of the Savage Spirit",
                ),
                component(
                    1015,
                    2,
                    3,
                    1317,
                    "Meet or Exceed the Class Requirement (Beastlord)",
                ),
            ),
            component_counts=(),
        )
        (
            selected_epics,
            item_criteria,
            candidates,
            selected,
            rejected,
        ) = IMPORTER._item_name_milestones(
            resources,
            IMPORTER.resolve_expansion("don"),
            frozenset((90000,)),
            IMPORTER._state_components_by_achievement(resources),
            IMPORTER._required_counts(resources),
        )

        self.assertEqual(selected_epics, (1001,))
        self.assertEqual((candidates, selected, rejected), (3, 2, 1))
        self.assertEqual(
            tuple(
                (
                    row.achievement_id,
                    row.component_id,
                    row.item_name,
                    row.required_class,
                )
                for row in item_criteria
            ),
            (
                (1001, 1016, "Blade of Tactics", 1),
                (1001, 1017, "Blade of Strategy", 1),
                (90000, 3000, "Bone Crafted Key", 0),
            ),
        )
        output = []
        IMPORTER._emit_item_name_criteria(output, item_criteria)
        sql = "\n".join(output)
        self.assertIn(
            "ON LOWER(matched_item.`Name`) = "
            "LOWER(mapping.`item_name`)",
            sql,
        )
        self.assertIn(
            f"{IMPORTER.EVENT_OWN_ITEM}, {IMPORTER.PROGRESS_BOOLEAN}, "
            "mapping.`behavior`, matched_item.`id`, "
            "mapping.`required_class`,",
            sql,
        )
        self.assertIn("\t1, mapping.`required_count`, 1", sql)
        self.assertIn(
            "CONVERT(0x426C616465206F662054616374696373 USING utf8mb4)",
            sql,
        )

    def test_npc_name_hash_contract_matches_golden_vectors(self):
        self.assertEqual(
            IMPORTER.npc_name_hash("Vishimtar_the_Fallen00"),
            0x708BEE77,
        )
        self.assertEqual(
            IMPORTER.npc_name_hash("Tunare's Guardian"),
            0xCF16724E,
        )
        self.assertEqual(
            IMPORTER.npc_name_hash("Tunare`s_Guardian00"),
            0xCF16724E,
        )
        self.assertEqual(
            IMPORTER.npc_name_hash("#A_Rat_01"),
            0x40FF2A77,
        )

    def test_named_kills_are_zone_scoped_for_repeated_ldon_names(self):
        achievements = (
            achievement(
                1022900, "Cauldron of Lost Souls Traveler"
            ),
            achievement(1023000, "Bloodied Quarries Traveler"),
            achievement(
                1022980, "Hunter of Cauldron of Lost Souls"
            ),
            achievement(1023080, "Hunter of Bloodied Quarries"),
        )
        associations = (
            association(102, 1022900),
            association(102, 1023000),
            association(108, 1022980),
            association(108, 1023080),
        )
        components = (
            component(
                1022900,
                1,
                1,
                270001,
                "Visit the Cauldron of Lost Souls",
            ),
            component(
                1023000,
                1,
                1,
                270002,
                "Visit the Bloodied Quarries",
            ),
            component(
                1022980, 1, 1, 280001, "#A_Rat_01"
            ),
            component(
                1023080, 1, 2, 280002, "#A_Rat_01"
            ),
        )
        resources, selection = self.selection_for(
            achievements,
            components,
            associations=associations,
        )

        named_kills = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type == IMPORTER.EVENT_NPC_NAME_KILL
        )
        self.assertEqual(len(named_kills), 2)
        self.assertEqual(
            tuple(
                (row.target_id, row.target_id2, row.behavior)
                for row in named_kills
            ),
            (
                (
                    0x40FF2A77,
                    229,
                    IMPORTER.BEHAVIOR_REQUIRED,
                ),
                (
                    0x40FF2A77,
                    230,
                    IMPORTER.BEHAVIOR_OPTIONAL,
                ),
            ),
        )
        sql = IMPORTER.generate_sql(
            resources, enable_selection=selection
        )
        self.assertIn(
            f"(1022980, 1, 1, 280001, 12, 3, 0, "
            f"{0x40FF2A77}, 229, 0, 1, 1)",
            sql,
        )

    def test_named_kills_exclude_raids_and_disambiguate_duplicate_hunters(self):
        achievements = (
            achievement(1033700, "The Broodlands Traveler"),
            achievement(1033780, "Hunter of The Broodlands"),
            achievement(999980, "Hunter of The Broodlands"),
            achievement(1033740, "Conqueror of The Broodlands"),
        )
        associations = (
            association(102, 1033700),
            association(108, 1033780),
            association(108, 999980),
            association(106, 1033740),
        )
        components = (
            component(
                1033700, 1, 1, 270276, "Visit The Broodlands"
            ),
            component(1033780, 1, 1, 280001, "Valid Named"),
            component(999980, 1, 1, 280002, "Enhanced Named"),
            component(1033740, 1, 1, 280003, "Raid Event"),
        )
        _resources, selection = self.selection_for(
            achievements,
            components,
            associations=associations,
        )

        named_kills = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type == IMPORTER.EVENT_NPC_NAME_KILL
        )
        self.assertEqual(
            tuple(
                (row.achievement_id, row.component_id, row.target_id2)
                for row in named_kills
            ),
            ((1033780, 280001, 337),),
        )
        self.assertEqual(
            selection.criteria_report.npc_name_definitions, 2
        )
        self.assertEqual(
            selection.criteria_report.npc_name_unresolved_definitions, 1
        )

    def test_named_kill_canonical_aliases_within_definition_are_skipped(self):
        achievements = (
            achievement(1033700, "The Broodlands Traveler"),
            achievement(1033780, "Hunter of The Broodlands"),
        )
        associations = (
            association(102, 1033700),
            association(108, 1033780),
        )
        components = (
            component(
                1033700, 1, 1, 270276, "Visit The Broodlands"
            ),
            component(1033780, 1, 1, 280001, "Named 1"),
            component(1033780, 2, 1, 280002, "Named 2"),
        )
        _resources, selection = self.selection_for(
            achievements,
            components,
            associations=associations,
        )

        self.assertFalse(
            any(
                row.event_type == IMPORTER.EVENT_NPC_NAME_KILL
                for row in selection.generated_criteria
            )
        )
        self.assertEqual(
            selection.criteria_report.npc_name_collision_components, 2
        )

    def test_named_kill_hash_collisions_within_a_zone_are_skipped(self):
        achievements = (
            achievement(1033700, "The Broodlands Traveler"),
            achievement(1033780, "Hunter of The Broodlands"),
        )
        associations = (
            association(102, 1033700),
            association(108, 1033780),
        )
        components = (
            component(
                1033700, 1, 1, 270276, "Visit The Broodlands"
            ),
            component(1033780, 1, 1, 280001, "First Named"),
            component(1033780, 2, 1, 280002, "Second Named"),
        )

        with mock.patch.object(IMPORTER, "fnv1a_32", return_value=123):
            _resources, selection = self.selection_for(
                achievements,
                components,
                associations=associations,
            )
        self.assertFalse(
            any(
                row.event_type == IMPORTER.EVENT_NPC_NAME_KILL
                for row in selection.generated_criteria
            )
        )
        self.assertEqual(
            selection.criteria_report.npc_name_collision_components, 2
        )

    def test_don_selection_prunes_later_zone_families_and_required_parents(self):
        categories = [
            category(10, 0, "General"),
            category(13, 10, "Level"),
        ]
        expansion_category_ids = {}
        for index, profile in enumerate(
            IMPORTER.EXPANSION_PROFILES, start=1
        ):
            category_id = 1000 + index
            expansion_category_ids[profile.key] = category_id
            categories.append(
                category(
                    category_id,
                    0,
                    profile.resource_category_name,
                )
            )
            if profile.key == "don":
                break

        achievements = (
            achievement(1034500, "Guild Hall Traveler"),
            achievement(1034600, "Late Zone Traveler"),
            achievement(1021300, "The Plane of War Traveler"),
            achievement(1021310, "Plane of War Quest"),
            achievement(6000, "Parent"),
            achievement(6001, "Grand Parent"),
            achievement(6002, "Optional Parent"),
            achievement(208100, "The Temple of Droga Traveler"),
            achievement(208180, "Hunter of The Temple of Droga"),
            achievement(258180, "Hunter of The Temple of Droga"),
            achievement(100038400, "Freeport Sewers Traveler"),
            achievement(138480, "Hunter of Freeport Sewers"),
        )
        classic_category_id = expansion_category_ids["classic"]
        hunter_category_id = 2000
        categories.append(
            category(hunter_category_id, classic_category_id, "Hunter")
        )
        hunter_ids = {208180, 258180, 138480}
        associations = tuple(
            association(
                hunter_category_id
                if row.achievement_id in hunter_ids
                else classic_category_id,
                row.achievement_id,
            )
            for row in achievements
        )
        components = (
            component(
                1034500, 1, 1, 300001, "Visit your Guild Hall"
            ),
            component(
                1034600, 1, 1, 300002, "Visit Late Zone"
            ),
            component(
                1021300, 1, 1, 300003, "Visit The Plane of War"
            ),
            component(1021310, 1, 1, 300004, "Complete War Quest"),
            component(6000, 1, 1, 300005, "Plane of War Quest"),
            component(6001, 1, 1, 300006, "Parent"),
            component(
                6002, 1, 1, 300007, "Guild Hall Traveler"
            ),
            component(6002, 2, 2, 300008, "Late Zone Traveler"),
            component(
                208100,
                1,
                1,
                300009,
                "Visit The Temple of Droga",
            ),
            component(208180, 1, 1, 300010, "Chief RokGus"),
            component(
                258180,
                1,
                3,
                300011,
                "The Temple of Droga (Enhanced) - Availability",
            ),
            component(258180, 2, 1, 300012, "Enhanced Named"),
            component(
                100038400,
                1,
                1,
                300013,
                "Visit Freeport Sewers",
            ),
            component(138480, 1, 1, 300014, "Later Named"),
        )
        resources = IMPORTER.ResourceSet(
            categories=tuple(categories),
            achievements=achievements,
            category_associations=associations,
            components=components,
            component_counts=(),
        )

        selection = IMPORTER.build_enable_selection(
            resources,
            through_expansion="don",
        )

        self.assertEqual(
            selection.expansion_achievement_ids,
            (6002, 208100, 208180, 1034500),
        )
        self.assertNotIn(138480, selection.expansion_achievement_ids)
        self.assertIn(138480, selection.achievement_ids)
        self.assertNotIn(138480, selection.forced_disabled_achievement_ids)
        self.assertIn(
            (138480, 1, 300014),
            selection.reviewed_unavailable_component_keys,
        )
        named_kills = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type == IMPORTER.EVENT_NPC_NAME_KILL
        )
        self.assertEqual(
            tuple(
                (row.achievement_id, row.target_id2)
                for row in named_kills
            ),
            ((208180, 81),),
        )
        dependencies = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type
            == IMPORTER.EVENT_ACHIEVEMENT_COMPLETE
        )
        self.assertEqual(
            tuple(
                (row.achievement_id, row.target_id, row.behavior)
                for row in dependencies
            ),
            (
                (
                    6002,
                    1034500,
                    IMPORTER.BEHAVIOR_REQUIRED,
                ),
            ),
        )


class SlayerCriteriaTests(unittest.TestCase):
    @staticmethod
    def resources():
        return IMPORTER.ResourceSet(
            categories=(
                category(80, 0, "Slayer"),
                category(81, 80, "General"),
            ),
            achievements=(
                achievement(100, "Orc Stomp!"),
                achievement(101, "Gnolls"),
                achievement(102, "Future Creature"),
                achievement(103, "Slayer Meta"),
            ),
            category_associations=(
                association(81, 100),
                association(81, 101),
                association(81, 102),
                association(81, 103),
            ),
            components=(
                component(
                    100,
                    1,
                    1,
                    1001,
                    "Orcs and Wereorcs.",
                ),
                component(101, 1, 1, 1002, "Gnolls"),
                component(102, 1, 1, 1003, "Ursarachnids"),
                component(
                    103,
                    1,
                    1,
                    1004,
                    'Complete the achievement "orc stomp!"',
                ),
                component(
                    103,
                    2,
                    2,
                    1005,
                    '(Optional) Complete the achievement "Future Creature"',
                ),
            ),
            component_counts=(
                IMPORTER.ComponentCount(1001, 5),
                IMPORTER.ComponentCount(1002, 10),
            ),
        )

    def test_slayer_is_explicit_opt_in_independent_of_expansions(self):
        resources = self.resources()

        unselected = IMPORTER.build_enable_selection(resources)
        selected = IMPORTER.build_enable_selection(
            resources,
            enable_slayer=True,
        )

        self.assertEqual(unselected.achievement_ids, frozenset())
        self.assertTrue(selected.enable_slayer)
        self.assertIsNone(selected.through_expansion)
        self.assertEqual(
            selected.slayer_achievement_ids,
            (100, 101, 103),
        )
        self.assertEqual(
            selected.criteria_report.slayer_candidates,
            4,
        )
        self.assertEqual(
            (
                selected.criteria_report.slayer_direct_selected,
                selected.criteria_report.slayer_meta_selected,
                selected.criteria_report.slayer_rejected,
            ),
            (2, 1, 1),
        )

    def test_slayer_mapping_uses_only_declared_rof2_race_ids(self):
        races_header = (
            SCRIPT_PATH.parents[2] / "common" / "races.h"
        ).read_text(encoding="utf-8")
        declared_ids = {
            int(value)
            for value in re.findall(
                r"constexpr\s+uint16\s+\w+\s*=\s*(\d+)\s*;",
                races_header,
            )
        }
        mapped_ids = {
            race_id
            for race_ids in (
                *IMPORTER.SLAYER_RACE_TERM_IDS.values(),
                *IMPORTER.SLAYER_RACE_DESCRIPTION_OVERRIDES.values(),
            )
            for race_id in race_ids
        }

        self.assertTrue(mapped_ids)
        self.assertEqual(mapped_ids - declared_ids, set())

    def test_slayer_discord_prose_uses_exact_audited_overrides(self):
        cases = (
            (
                "All creatures of discord: Aneuks, Discordlings, Girplans, "
                "Huvuls, Ikaavs, Ixts... See: Such Anguish",
                (393, 394, 395, 400, 418, 419),
            ),
            (
                "All creatures of discord: ...Kyvs, Lightning Warriors, "
                "Mastruqs... See Discord Sounds Out of Tune",
                (396, 402, 407),
            ),
        )

        for description, expected_ids in cases:
            with self.subTest(description=description):
                race_ids, unresolved = IMPORTER._slayer_race_ids(
                    description
                )
                self.assertEqual(race_ids, expected_ids)
                self.assertEqual(unresolved, ())

        race_ids, unresolved = IMPORTER._slayer_race_ids(
            cases[0][0] + "."
        )
        self.assertEqual(race_ids, ())
        self.assertTrue(unresolved)

    def test_slayer_resolves_only_audited_stale_dependency_names(self):
        resources = IMPORTER.ResourceSet(
            categories=(
                category(80, 0, "Slayer"),
                category(81, 80, "General"),
            ),
            achievements=(
                achievement(200, "Natives of Luclin"),
                achievement(201, "Love Will Teir Them Apart"),
                achievement(202, "50 Shades..."),
                achievement(203, "Slayer Meta"),
            ),
            category_associations=(
                association(81, 200),
                association(81, 201),
                association(81, 202),
                association(81, 203),
            ),
            components=(
                component(200, 1, 1, 2001, "Orcs"),
                component(201, 1, 1, 2002, "Gnolls"),
                component(202, 1, 1, 2003, "Goblins"),
                component(
                    203,
                    1,
                    1,
                    2004,
                    'Complete the achievement "Navies of Luclin"',
                ),
                component(
                    203,
                    2,
                    1,
                    2005,
                    (
                        'Complete the achievement "Dark Elf Antonican, '
                        'Please!"'
                    ),
                ),
                component(
                    203,
                    3,
                    1,
                    2006,
                    'Complete the achievement "50 Shades Repaid"',
                ),
            ),
            component_counts=(),
        )

        selection = IMPORTER.build_enable_selection(
            resources,
            enable_slayer=True,
        )
        dependencies = tuple(
            criterion.target_id
            for criterion in selection.generated_criteria
            if criterion.achievement_id == 203
        )

        self.assertIn(203, selection.achievement_ids)
        self.assertEqual(dependencies, (200, 201, 202))
        self.assertFalse(selection.slayer_rejections)

    def test_slayer_emits_increment_per_exact_race_id_and_count(self):
        selection = IMPORTER.build_enable_selection(
            self.resources(),
            enable_slayer=True,
        )

        orc_criteria = tuple(
            criterion
            for criterion in selection.generated_criteria
            if criterion.achievement_id == 100
        )
        self.assertEqual(
            tuple(criterion.target_id for criterion in orc_criteria),
            (54, 361, 366, 458, 579),
        )
        self.assertTrue(
            all(
                criterion.event_type
                == IMPORTER.EVENT_NPC_RACE_KILL
                and criterion.progress_mode
                == IMPORTER.PROGRESS_INCREMENT
                and criterion.required_count == 5
                and criterion.behavior
                == IMPORTER.BEHAVIOR_REQUIRED
                for criterion in orc_criteria
            )
        )

    def test_slayer_rejects_entire_unmapped_leaf_without_partial_criteria(self):
        selection = IMPORTER.build_enable_selection(
            self.resources(),
            enable_slayer=True,
        )

        self.assertNotIn(102, selection.achievement_ids)
        self.assertFalse(
            any(
                criterion.achievement_id == 102
                for criterion in selection.generated_criteria
            )
        )
        self.assertEqual(len(selection.slayer_rejections), 1)
        self.assertIn(
            "component 1003 has unmapped race term(s) 'Ursarachnids'",
            selection.slayer_rejections[0],
        )

    def test_slayer_meta_uses_quoted_casefold_name_and_skips_optional(self):
        selection = IMPORTER.build_enable_selection(
            self.resources(),
            enable_slayer=True,
        )
        meta_criteria = tuple(
            criterion
            for criterion in selection.generated_criteria
            if criterion.achievement_id == 103
        )

        self.assertEqual(len(meta_criteria), 1)
        self.assertEqual(
            (
                meta_criteria[0].event_type,
                meta_criteria[0].progress_mode,
                meta_criteria[0].behavior,
                meta_criteria[0].target_id,
            ),
            (
                IMPORTER.EVENT_ACHIEVEMENT_COMPLETE,
                IMPORTER.PROGRESS_BOOLEAN,
                IMPORTER.BEHAVIOR_REQUIRED,
                100,
            ),
        )

    def test_slayer_meta_requires_every_required_dependency(self):
        resources = self.resources()
        components = tuple(
            component
            for component in resources.components
            if not (
                component.achievement_id == 103
                and component.component_type == 2
            )
        ) + (
            component(
                103,
                2,
                1,
                1005,
                'Complete the achievement "Future Creature"',
            ),
        )
        resources = IMPORTER.ResourceSet(
            resources.categories,
            resources.achievements,
            resources.category_associations,
            components,
            resources.component_counts,
        )

        selection = IMPORTER.build_enable_selection(
            resources,
            enable_slayer=True,
        )

        self.assertNotIn(103, selection.achievement_ids)
        self.assertTrue(
            any(
                "definition 102 not safely selected" in rejection
                for rejection in selection.slayer_rejections
            )
        )

    def test_slayer_sql_contains_native_race_increment_policy(self):
        resources = self.resources()
        selection = IMPORTER.build_enable_selection(
            resources,
            enable_slayer=True,
        )
        sql = IMPORTER.generate_sql(
            resources,
            enable_selection=selection,
        )

        self.assertIn(
            "(100, 1, 1, 1001, 3, 0, 0, 54, 0, 0, 5, 1)",
            sql,
        )
        self.assertIn(
            "UPDATE `achievements` SET `enabled` = 1 WHERE `id` IN "
            "(100, 101, 103);",
            sql,
        )
        self.assertIn(
            "-- Slayer rejection: Slayer achievement 102",
            sql,
        )


class ReviewedDonCoverageTests(unittest.TestCase):
    @staticmethod
    def fishlord_resources(description_override=None):
        rows = (
            (1, 1, 5000222, "Coirnav the Avatar of Water"),
            (2, 1, 5000337, "Fishlord Summoning Event"),
            (
                3,
                2,
                5000334,
                "Phase 1: a hungry, stringy, or toughened anglerfish",
            ),
            (
                4,
                2,
                5000335,
                "Phase 2: a dark, foul, or wicked anglerfish",
            ),
            (
                5,
                2,
                5000336,
                "Phase 3: a prime, prismatic, or superior anglerfish",
            ),
            (
                6,
                2,
                5000337,
                "Phase 4: a king, master, or supreme anglerfish",
            ),
            (7, 1, 5000316, "Grioihin the Wise"),
            (8, 1, 5000317, "Hydrotha"),
            (9, 1, 5000318, "Krziik the Mighty"),
            (10, 1, 5000319, "Ofossaa the Enlightened"),
        )
        components = tuple(
            component(
                521640,
                sequence,
                component_type,
                component_id,
                (
                    description_override
                    if description_override is not None and sequence == 3
                    else description
                ),
            )
            for sequence, component_type, component_id, description in rows
        )
        return IMPORTER.ResourceSet(
            categories=(),
            achievements=(achievement(521640, "Conqueror of Reef of Coirnav"),),
            category_associations=(),
            components=components,
            component_counts=(),
        )

    @staticmethod
    def source_criteria(resources):
        components_by_achievement = IMPORTER._state_components_by_achievement(
            resources
        )
        return IMPORTER._reviewed_source_criteria(
            resources,
            components_by_achievement,
            IMPORTER._required_counts(resources),
        )

    @staticmethod
    def profile_resources():
        categories = [category(1, 0, "General"), category(2, 1, "Level")]
        categories.extend(
            category(100 + index, 0, profile.resource_category_name)
            for index, profile in enumerate(IMPORTER.EXPANSION_PROFILES)
        )
        categories.extend(
            (category(70, 0, "The Hero's Journey"), category(75, 70, "Quests"))
        )
        ids = (
            12000,
            12007,
            500980500,
            500980605,
            500990020,
            600050,
            722940,
        )
        return IMPORTER.ResourceSet(
            categories=tuple(categories),
            achievements=tuple(achievement(value, str(value)) for value in ids),
            category_associations=(
                association(75, 12000),
                association(75, 12007),
                association(75, 500980500),
                association(75, 500980605),
                association(75, 500990020),
                association(106, 722940),
            ),
            components=(
                component(12000, 1, 1, 12000, "City quest"),
                component(12007, 1, 1, 12007, "Unresolved city quest"),
                component(500980500, 1, 1, 59805014, "Breakdown"),
                component(500980605, 1, 1, 59806050, "Choosing sides"),
                component(500990020, 1, 1, 59900200, "Mastery"),
                component(600050, 1, 1, 600050, "Tutorial"),
                component(722940, 1, 1, 7000159, "Ritualist of Hate"),
                component(722940, 2, 1, 7000160, "The Rescue"),
                component(722940, 3, 1, 7000161, "The Curse Reborn"),
            ),
            component_counts=(),
        )

    def test_reviewed_source_maps_task_item_dependency_and_not_presentation(self):
        resources = IMPORTER.ResourceSet(
            categories=(),
            achievements=(
                achievement(90007, "A Stone Key"),
                achievement(500980610, "DoN tier"),
                achievement(100060, "Meta"),
                achievement(103940, "Dependency"),
            ),
            category_associations=(),
            components=(
                component(90007, 1, 1, 3014, "A Stone Key"),
                component(500980610, 1, 2, 59806102, "Task"),
                component(500980610, 2, 2, 59806101, "First, complete"),
                component(100060, 1, 1, 1001490, "Dependency"),
            ),
            component_counts=(),
        )

        criteria, keys, rejections = self.source_criteria(resources)
        by_key = {}
        for criterion in criteria:
            by_key.setdefault(
                (
                    criterion.achievement_id,
                    criterion.component_type,
                    criterion.component_id,
                ),
                [],
            ).append(criterion)

        self.assertFalse(rejections)
        self.assertEqual(
            (by_key[(90007, 1, 3014)][0].event_type,
             by_key[(90007, 1, 3014)][0].target_id),
            (IMPORTER.EVENT_OWN_ITEM, 12708),
        )
        self.assertEqual(
            (by_key[(500980610, 2, 59806102)][0].event_type,
             by_key[(500980610, 2, 59806102)][0].target_id),
            (IMPORTER.EVENT_TASK_COMPLETE, 4827),
        )
        self.assertEqual(
            (by_key[(100060, 1, 1001490)][0].event_type,
             by_key[(100060, 1, 1001490)][0].target_id),
            (IMPORTER.EVENT_ACHIEVEMENT_COMPLETE, 103940),
        )
        self.assertNotIn((500980610, 2, 59806101), keys)

        item_names = (
            IMPORTER.ItemNameCriterion(90007, 1, 1, 3014, 0, "A Stone Key"),
            IMPORTER.ItemNameCriterion(90020, 1, 1, 3033, 0, "Factory Code"),
            IMPORTER.ItemNameCriterion(42, 1, 1, 42, 0, "Unreviewed Item"),
        )
        self.assertEqual(
            IMPORTER._without_reviewed_item_name_criteria(item_names),
            (item_names[2],),
        )

    def test_fishlord_uses_three_native_name_alternatives_per_phase(self):
        criteria, keys, rejections = self.source_criteria(
            self.fishlord_resources()
        )

        self.assertFalse(rejections)
        for key in (
            (521640, 2, 5000334),
            (521640, 2, 5000335),
            (521640, 2, 5000336),
            (521640, 1, 5000337),
            (521640, 2, 5000337),
        ):
            matches = [
                row
                for row in criteria
                if (
                    row.achievement_id,
                    row.component_type,
                    row.component_id,
                )
                == key
            ]
            self.assertEqual(len(matches), 3)
            self.assertTrue(
                all(
                    row.event_type == IMPORTER.EVENT_NPC_NAME_KILL
                    and row.target_id2 == 216
                    for row in matches
                )
            )
            self.assertIn(key, keys)

    def test_reviewed_raid_shape_change_fails_closed(self):
        criteria, keys, rejections = self.source_criteria(
            self.fishlord_resources("changed client description")
        )

        self.assertFalse(
            any(row.achievement_id == 521640 for row in criteria)
        )
        self.assertFalse(
            any(key[0] == 521640 for key in keys)
        )
        self.assertTrue(
            any("raid definition 521640 shape changed" in row for row in rejections)
        )

    def test_clean_don_profile_enables_investigation_achievement_ids(self):
        selection = IMPORTER.build_enable_selection(
            self.profile_resources(), through_expansion="don"
        )

        for achievement_id in (
            12000,
            12007,
            500980500,
            500980605,
            500990020,
            722940,
        ):
            self.assertIn(achievement_id, selection.achievement_ids)
        self.assertIn(600050, selection.forced_disabled_achievement_ids)
        self.assertNotIn(600050, selection.achievement_ids)
        self.assertNotIn(12007, selection.forced_disabled_achievement_ids)
        self.assertNotIn(722940, selection.forced_disabled_achievement_ids)
        self.assertIn(
            (12007, 1, 12007),
            selection.reviewed_unavailable_component_keys,
        )
        self.assertIn(
            (722940, 1, 7000160),
            selection.reviewed_unavailable_component_keys,
        )
        self.assertIn(
            (722940, 1, 7000159),
            selection.reviewed_script_component_keys,
        )

        manual_criteria = [
            row
            for row in selection.generated_criteria
            if (
                row.achievement_id,
                row.component_type,
                row.component_id,
            )
            == (12000, 1, 12000)
        ]
        self.assertEqual(len(manual_criteria), 1)
        self.assertEqual(
            (
                manual_criteria[0].component_sequence,
                manual_criteria[0].event_type,
                manual_criteria[0].progress_mode,
                manual_criteria[0].behavior,
                manual_criteria[0].target_id,
                manual_criteria[0].target_id2,
                manual_criteria[0].target_value,
                manual_criteria[0].required_count,
            ),
            (
                1,
                IMPORTER.EVENT_MANUAL,
                IMPORTER.PROGRESS_SET,
                IMPORTER.BEHAVIOR_REQUIRED,
                0,
                0,
                0,
                1,
            ),
        )
        self.assertFalse(
            any(
                (
                    row.achievement_id,
                    row.component_type,
                    row.component_id,
                )
                == (12007, 1, 12007)
                for row in selection.generated_criteria
            )
        )

        sql = IMPORTER.generate_sql(
            self.profile_resources(), enable_selection=selection
        )
        criteria_sql = sql.split(
            "INSERT INTO `achievement_criteria`", maxsplit=1
        )[1].split("COMMIT;", maxsplit=1)[0]
        self.assertIn(
            "(12000, 1, 1, 12000, 0, 2, 0, 0, 0, 0, 1, 1)",
            criteria_sql,
        )
        self.assertNotIn("(12007, 1, 1, 12007, 0,", criteria_sql)

    def test_investigation_definitions_are_not_force_disabled(self):
        expected = IMPORTER.REVIEWED_PROGRESSION_UNAVAILABLE_IDS | {
            138480, 139380, 151880, 153940, 153980, 154880, 250880,
            352640, 415440, 415840, 520440,
            722940, 723040, 723140, 723240, 723340,
        }

        self.assertEqual(
            IMPORTER.REVIEWED_INVESTIGATION_ACHIEVEMENT_IDS,
            expected,
        )
        self.assertEqual(len(expected), 38)
        self.assertEqual(
            len(IMPORTER.REVIEWED_FORCED_DISABLED_ACHIEVEMENT_IDS),
            99,
        )
        self.assertFalse(
            expected & IMPORTER.REVIEWED_FORCED_DISABLED_ACHIEVEMENT_IDS
        )

    def test_investigation_ids_do_not_participate_in_dependency_inference(self):
        categories = [
            category(1, 0, "General"),
            category(2, 1, "Level"),
        ]
        for index, profile in enumerate(IMPORTER.EXPANSION_PROFILES):
            categories.append(
                category(100 + index, 0, profile.resource_category_name)
            )
            if profile.key == "don":
                break

        achievements = (
            achievement(100105, "Optional Meta"),
            achievement(138480, "Hunter of Freeport Sewers"),
            achievement(200085, "Required Meta"),
            achievement(210880, "Hunter of Veeshan's Peak"),
            achievement(250880, "Hunter of Veeshan's Peak"),
        )
        resources = IMPORTER.ResourceSet(
            categories=tuple(categories),
            achievements=achievements,
            category_associations=tuple(
                association(100, row.achievement_id)
                for row in achievements
            ),
            components=(
                component(
                    100105,
                    1,
                    2,
                    1001041,
                    "Hunter of Freeport Sewers",
                ),
                component(
                    200085,
                    1,
                    1,
                    2001023,
                    "Hunter of Veeshan's Peak",
                ),
            ),
            component_counts=(),
        )

        selection = IMPORTER.build_enable_selection(
            resources,
            through_expansion="don",
        )
        dependencies = tuple(
            row
            for row in selection.generated_criteria
            if row.event_type == IMPORTER.EVENT_ACHIEVEMENT_COMPLETE
        )

        self.assertIn(138480, selection.achievement_ids)
        self.assertIn(250880, selection.achievement_ids)
        self.assertEqual(
            tuple(
                (
                    row.achievement_id,
                    row.component_type,
                    row.component_id,
                    row.target_id,
                )
                for row in dependencies
            ),
            ((200085, 1, 2001023, 210880),),
        )

    def test_later_profiles_do_not_apply_don_forced_disables(self):
        resources = self.profile_resources()
        for profile in ("dodh", "tob"):
            with self.subTest(profile=profile):
                selection = IMPORTER.build_enable_selection(
                    resources, through_expansion=profile
                )
                self.assertFalse(selection.forced_disabled_achievement_ids)
                self.assertFalse(selection.reviewed_profile_achievement_ids)

    def test_reviewed_disables_override_preserve_state_and_report_contract(self):
        resources = IMPORTER.ResourceSet(
            categories=(category(1, 0, "General"), category(2, 1, "Special")),
            achievements=(achievement(600050, "Tutorials - Out of Gloomingdeep"),),
            category_associations=(association(2, 600050),),
            components=(component(600050, 1, 1, 600050, "Tutorial"),),
            component_counts=(),
        )
        selection = IMPORTER.EnableSelection(
            through_expansion=IMPORTER.resolve_expansion("don"),
            max_level=70,
            expansion_achievement_ids=(),
            level_achievement_ids=(),
            level_criteria=(),
            forced_disabled_achievement_ids=(600050,),
            reviewed_source_component_keys=((90007, 1, 3014),),
            reviewed_script_component_keys=((12000, 1, 12000),),
            reviewed_presentation_component_keys=((500980610, 2, 59806101),),
            reviewed_unavailable_component_keys=((600050, 1, 600050),),
        )
        sql = IMPORTER.generate_sql(
            resources,
            enable_selection=selection,
            preserve_enable_state=True,
        )

        self.assertIn(
            "UPDATE `achievements` SET `enabled` = 0 WHERE `id` IN (600050);",
            sql,
        )
        self.assertIn(
            "Disabled 600050: Tutorials - Out of Gloomingdeep | "
            "General / Special | the tutorial belongs to post-DoN "
            "Gloomingdeep content",
            sql,
        )
        self.assertIn("source-native: 90007/1/3014", sql)
        self.assertIn("quest-manual: 12000/1/12000", sql)
        self.assertIn("presentation-only: 500980610/2/59806101", sql)
        self.assertIn("unavailable: 600050/1/600050", sql)
        self.assertIn("SetAchievementProgress(achievement_id,", sql)
        self.assertIn("world state-update path", sql)

        reset_output = []
        IMPORTER._emit_reviewed_own_item_reset(
            reset_output, ((90007, 1, 3014),)
        )
        reset_sql = "\n".join(reset_output)
        self.assertIn("WHERE `event_type` = 7", reset_sql)
        self.assertIn("((90007, 1, 3014));", reset_sql)


class ResourceFileValidationTests(unittest.TestCase):
    def test_empty_resource_snapshot_is_rejected(self):
        with (
            mock.patch.object(
                IMPORTER, "_load_categories", return_value=()
            ),
            mock.patch.object(
                IMPORTER, "_load_achievements", return_value=()
            ),
            mock.patch.object(
                IMPORTER,
                "_load_category_associations",
                return_value=(),
            ),
            mock.patch.object(
                IMPORTER, "_load_components", return_value=()
            ),
            mock.patch.object(
                IMPORTER, "_load_component_counts", return_value=()
            ),
        ):
            with self.assertRaisesRegex(
                IMPORTER.ResourceError,
                "AchievementCategories.txt: expected at least one data row",
            ):
                IMPORTER.load_resources(Path(__file__).parent)

    def test_malformed_resource_line_reports_file_and_line(self):
        resource_directory = (
            Path(__file__).parent
            / "fixtures"
            / "achievements_malformed"
        )
        with self.assertRaisesRegex(
            IMPORTER.ResourceError,
            r"AchievementCategories\.txt:1: expected a trailing caret",
        ):
            IMPORTER.load_resources(resource_directory)


if __name__ == "__main__":
    unittest.main()
