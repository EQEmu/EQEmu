import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


SCRIPT_PATH = Path(__file__).parents[1] / "export_achievement_resources.py"
SPEC = importlib.util.spec_from_file_location(
    "export_achievement_resources", SCRIPT_PATH
)
assert SPEC is not None and SPEC.loader is not None
EXPORTER = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = EXPORTER
SPEC.loader.exec_module(EXPORTER)


def snapshot():
    return {
        "AchievementCategories.txt": (
            ("10", "2", "12", "Class", "Class achievements", "A_ClassSub"),
            ("0", "1", "10", "General", "General achievements", "A_General"),
            ("10", "1", "11", "Advancement", "AA achievements", "A_AASub"),
        ),
        "AchievementsClient.txt": (
            ("20", "Twenty", "Second", "200", "10", "0", "3"),
            ("5", "Five", "First", "100", "5", "1", "7"),
        ),
        "AchievementCategoryAssociationsClient.txt": (
            ("11", "2", "20"),
            ("11", "1", "5"),
        ),
        "AchievementComponentsClient.txt": (
            ("5", "2", "3", "105", "Optional cap"),
            ("5", "2", "1", "102", "Second required"),
            ("5", "1", "1", "101", "First required"),
        ),
        "AchievementAssociationsClient.txt": (
            ("102", "2"),
            ("101", "1"),
        ),
    }


class RenderTests(unittest.TestCase):
    def test_renders_five_files_in_deterministic_client_order(self):
        rendered = EXPORTER.render_resources(snapshot())

        self.assertEqual(
            tuple(rendered),
            tuple(spec.filename for spec in EXPORTER.RESOURCE_SPECS),
        )
        self.assertEqual(
            rendered["AchievementCategories.txt"],
            "^1^10^General^General achievements^A_General^\n"
            "10^1^11^Advancement^AA achievements^A_AASub^\n"
            "10^2^12^Class^Class achievements^A_ClassSub^\n",
        )
        self.assertEqual(
            rendered["AchievementComponentsClient.txt"],
            "5^1^1^101^First required^\n"
            "5^2^1^102^Second required^\n"
            "5^2^3^105^Optional cap^\n",
        )
        self.assertEqual(
            rendered["AchievementCategoryAssociationsClient.txt"],
            "11^1^5^\n11^2^20^\n",
        )
        self.assertEqual(
            rendered["AchievementAssociationsClient.txt"],
            "101^1^\n102^2^\n",
        )

    def test_preserves_has_reward_and_client_flag(self):
        rendered = EXPORTER.render_resources(snapshot())

        self.assertEqual(
            rendered["AchievementsClient.txt"],
            "5^Five^First^100^5^1^7^\n"
            "20^Twenty^Second^200^10^0^3^\n",
        )

    def test_achievement_query_includes_generic_reward_mappings(self):
        query = EXPORTER.RESOURCE_SPECS[1].query()

        self.assertIn("CASE WHEN `has_reward` <> 0", query)
        self.assertIn("FROM `reward_sources` AS `rs`", query)
        self.assertIn("`rs`.`source_type` = 1", query)
        self.assertIn("`rs`.`enabled` = 1", query)
        self.assertIn("FROM `reward_source_entries` AS `rse`", query)
        self.assertIn("`rse`.`source_type` = 1", query)
        self.assertIn("`reward`.`enabled` = 1", query)

    def test_rejects_unrepresentable_text_before_writing(self):
        for invalid in ("bad^name", "two\nlines", "carriage\rreturn"):
            with self.subTest(invalid=invalid), tempfile.TemporaryDirectory() as root:
                rows = snapshot()
                achievements = list(rows["AchievementsClient.txt"])
                achievements[0] = (
                    "20",
                    invalid,
                    "Second",
                    "200",
                    "10",
                    "0",
                    "3",
                )
                rows["AchievementsClient.txt"] = tuple(achievements)

                output_directory = Path(root) / "resources"
                output_directory.mkdir()
                existing_file = output_directory / "AchievementsClient.txt"
                existing_file.write_text("unchanged\n", encoding="utf-8")

                class FakeClient:
                    @staticmethod
                    def fetch(spec):
                        return rows[spec.filename]

                with self.assertRaisesRegex(
                    EXPORTER.ExportError, "cannot be represented"
                ):
                    EXPORTER.export_resources(FakeClient(), output_directory)
                self.assertEqual(
                    existing_file.read_text(encoding="utf-8"), "unchanged\n"
                )
                self.assertEqual(
                    tuple(path.name for path in output_directory.iterdir()),
                    ("AchievementsClient.txt",),
                )


class MySQLClientTests(unittest.TestCase):
    def test_uses_mysql_login_mechanisms_without_password_arguments(self):
        with tempfile.TemporaryDirectory() as root:
            defaults_file = Path(root) / "client.cnf"
            defaults_file.touch()
            client = EXPORTER.MySQLClient(
                database="peq_content",
                executable="mariadb",
                defaults_extra_file=defaults_file,
                login_path="eqemu-content",
            )
            row = ("5", "Name", "Description", "100", "10", "1", "9")
            encoded = "\t".join(value.encode().hex() for value in row) + "\n"
            completed = subprocess.CompletedProcess(
                args=[], returncode=0, stdout=encoded.encode(), stderr=b""
            )

            with mock.patch.object(
                EXPORTER.subprocess, "run", return_value=completed
            ) as run:
                result = client.fetch(EXPORTER.RESOURCE_SPECS[1])

            command = run.call_args.args[0]
            self.assertEqual(command[0], "mariadb")
            self.assertTrue(command[1].startswith("--defaults-extra-file="))
            self.assertIn("--login-path=eqemu-content", command)
            self.assertIn("--database=peq_content", command)
            self.assertFalse(any("password" in argument for argument in command))
            self.assertEqual(result, (row,))
            self.assertIn(
                b"CASE WHEN `has_reward` <> 0",
                run.call_args.kwargs["input"],
            )

    def test_reports_null_database_fields(self):
        client = EXPORTER.MySQLClient(database="peq")
        completed = subprocess.CompletedProcess(
            args=[],
            returncode=0,
            stdout=b"31\tNULL\t33\t34\t35\t36\t37\n",
            stderr=b"",
        )

        with mock.patch.object(EXPORTER.subprocess, "run", return_value=completed):
            with self.assertRaisesRegex(EXPORTER.ExportError, "field 2 is NULL"):
                client.fetch(EXPORTER.RESOURCE_SPECS[1])


if __name__ == "__main__":
    unittest.main()
