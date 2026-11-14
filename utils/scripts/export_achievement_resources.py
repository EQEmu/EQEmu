#!/usr/bin/env python3
"""Export achievement presentation tables to client resource files."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from typing import Mapping, Optional, Sequence, Tuple


@dataclass(frozen=True)
class ResourceSpec:
    filename: str
    table: str
    columns: Tuple[str, ...]
    numeric_fields: Tuple[int, ...]
    sort_fields: Tuple[int, ...]
    blank_zero_fields: Tuple[int, ...] = ()
    select_overrides: Tuple[Tuple[int, str], ...] = ()

    def query(self) -> str:
        expressions = [f"`{column}`" for column in self.columns]
        for index, expression in self.select_overrides:
            expressions[index] = expression
        encoded_columns = ", ".join(
            f"HEX(CAST({expression} AS CHAR CHARACTER SET utf8mb4))"
            for expression in expressions
        )
        order_columns = ", ".join(
            f"`{self.columns[index]}`" for index in self.sort_fields
        )
        return (
            f"SELECT {encoded_columns} FROM `{self.table}` "
            f"ORDER BY {order_columns}"
        )


RESOURCE_SPECS = (
    ResourceSpec(
        "AchievementCategories.txt",
        "achievement_categories",
        ("parent_id", "sequence", "id", "name", "description", "icon"),
        (0, 1, 2),
        (0, 1, 2),
        (0,),
    ),
    ResourceSpec(
        "AchievementsClient.txt",
        "achievements",
        (
            "id",
            "name",
            "description",
            "icon_id",
            "points",
            "has_reward",
            "client_flag",
        ),
        (0, 3, 4, 5, 6),
        (0,),
        select_overrides=(
            (
                5,
                "CASE WHEN `has_reward` <> 0 OR EXISTS ("
                "SELECT 1 FROM `reward_sources` AS `rs` "
                "INNER JOIN `reward_sets` AS `rset` "
                "ON `rset`.`reward_set_id` = `rs`.`reward_set_id` "
                "AND `rset`.`enabled` = 1 "
                "WHERE `rs`.`source_type` = 1 "
                "AND `rs`.`source_id` = `achievements`.`id` "
                "AND `rs`.`enabled` = 1"
                ") OR EXISTS ("
                "SELECT 1 FROM `reward_source_entries` AS `rse` "
                "INNER JOIN `rewards` AS `reward` "
                "ON `reward`.`reward_id` = `rse`.`reward_id` "
                "AND `reward`.`enabled` = 1 "
                "WHERE `rse`.`source_type` = 1 "
                "AND `rse`.`source_id` = `achievements`.`id`"
                ") THEN 1 ELSE 0 END",
            ),
        ),
    ),
    ResourceSpec(
        "AchievementCategoryAssociationsClient.txt",
        "achievement_category_associations",
        ("category_id", "sequence", "achievement_id"),
        (0, 1, 2),
        (0, 1, 2),
    ),
    ResourceSpec(
        "AchievementComponentsClient.txt",
        "achievement_components",
        ("achievement_id", "sequence", "component_type", "component_id", "name"),
        (0, 1, 2, 3),
        (0, 2, 1, 3),
    ),
    ResourceSpec(
        "AchievementAssociationsClient.txt",
        "achievement_associations",
        ("component_id", "required_count"),
        (0, 1),
        (0,),
    ),
)


class ExportError(RuntimeError):
    pass


class MySQLClient:
    """Read rows through the standard mysql/mariadb command-line client."""

    def __init__(
        self,
        *,
        database: str,
        executable: str = "mysql",
        defaults_extra_file: Optional[Path] = None,
        login_path: Optional[str] = None,
    ) -> None:
        if not database:
            raise ExportError("content database name cannot be empty")
        if defaults_extra_file is not None and not defaults_extra_file.is_file():
            raise ExportError(
                f"MySQL defaults file does not exist: {defaults_extra_file}"
            )

        self.database = database
        self.executable = executable
        self.defaults_extra_file = defaults_extra_file
        self.login_path = login_path

    def _command(self) -> list[str]:
        command = [self.executable]
        # MySQL requires defaults-file options before all other client options.
        if self.defaults_extra_file is not None:
            command.append(
                f"--defaults-extra-file={self.defaults_extra_file.resolve()}"
            )
        if self.login_path:
            command.append(f"--login-path={self.login_path}")
        command.extend(
            (
                f"--database={self.database}",
                "--batch",
                "--raw",
                "--skip-column-names",
                "--default-character-set=utf8mb4",
            )
        )
        return command

    def fetch(self, spec: ResourceSpec) -> Tuple[Tuple[str, ...], ...]:
        query = f"{spec.query()};\n".encode("ascii")
        try:
            result = subprocess.run(
                self._command(),
                input=query,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
        except OSError as error:
            raise ExportError(
                f"unable to run MySQL client {self.executable!r}: {error}"
            ) from error

        if result.returncode != 0:
            message = result.stderr.decode("utf-8", errors="replace").strip()
            raise ExportError(
                f"MySQL query for {spec.filename} failed"
                + (f": {message}" if message else "")
            )

        try:
            output = result.stdout.decode("ascii", errors="strict")
        except UnicodeError as error:
            raise ExportError(
                f"MySQL returned malformed encoded data for {spec.filename}"
            ) from error

        rows = []
        for row_number, line in enumerate(output.splitlines(), start=1):
            encoded_fields = line.split("\t")
            if len(encoded_fields) != len(spec.columns):
                raise ExportError(
                    f"{spec.filename}: query row {row_number} returned "
                    f"{len(encoded_fields)} fields; expected {len(spec.columns)}"
                )

            fields = []
            for field_number, value in enumerate(encoded_fields, start=1):
                if value == "NULL":
                    raise ExportError(
                        f"{spec.filename}: query row {row_number}, field "
                        f"{field_number} is NULL"
                    )
                try:
                    decoded = bytes.fromhex(value).decode("utf-8", errors="strict")
                except (UnicodeError, ValueError) as error:
                    raise ExportError(
                        f"{spec.filename}: query row {row_number}, field "
                        f"{field_number} is not valid UTF-8 hex data"
                    ) from error
                fields.append(decoded)
            rows.append(tuple(fields))
        return tuple(rows)


def _validate_and_sort(
    spec: ResourceSpec, rows: Sequence[Sequence[str]]
) -> Tuple[Tuple[str, ...], ...]:
    validated = []
    numeric_fields = set(spec.numeric_fields)
    for row_number, source_row in enumerate(rows, start=1):
        row = tuple(source_row)
        if len(row) != len(spec.columns):
            raise ExportError(
                f"{spec.filename}: row {row_number} contains {len(row)} fields; "
                f"expected {len(spec.columns)}"
            )

        for field_number, value in enumerate(row, start=1):
            if not isinstance(value, str):
                raise ExportError(
                    f"{spec.filename}: row {row_number}, field {field_number} "
                    "is not text"
                )
            if any(character in value for character in ("^", "\r", "\n", "\x00")):
                raise ExportError(
                    f"{spec.filename}: row {row_number}, field {field_number} "
                    "contains a caret, newline, or NUL that cannot be represented"
                )
            if field_number - 1 in numeric_fields and (
                not value or not value.isascii() or not value.isdecimal()
            ):
                raise ExportError(
                    f"{spec.filename}: row {row_number}, field {field_number} "
                    "is not an unsigned decimal integer"
                )
        validated.append(row)

    validated.sort(
        key=lambda row: tuple(int(row[index], 10) for index in spec.sort_fields)
    )
    return tuple(validated)


def render_resources(
    rows_by_filename: Mapping[str, Sequence[Sequence[str]]]
) -> Mapping[str, str]:
    rendered = {}
    for spec in RESOURCE_SPECS:
        try:
            source_rows = rows_by_filename[spec.filename]
        except KeyError as error:
            raise ExportError(f"missing query results for {spec.filename}") from error

        lines = []
        for row in _validate_and_sort(spec, source_rows):
            fields = list(row)
            for index in spec.blank_zero_fields:
                if fields[index] == "0":
                    fields[index] = ""
            lines.append("^".join(fields) + "^\n")
        rendered[spec.filename] = "".join(lines)
    return rendered


def _write_resources(output_directory: Path, resources: Mapping[str, str]) -> None:
    try:
        output_directory.mkdir(parents=True, exist_ok=True)
    except OSError as error:
        raise ExportError(
            f"unable to create output directory {output_directory}: {error}"
        ) from error

    temporary_paths = {}
    try:
        for spec in RESOURCE_SPECS:
            descriptor, temporary_name = tempfile.mkstemp(
                dir=output_directory,
                prefix=f".{spec.filename}.",
                suffix=".tmp",
            )
            temporary_path = Path(temporary_name)
            temporary_paths[spec.filename] = temporary_path
            with os.fdopen(
                descriptor, "w", encoding="utf-8", errors="strict", newline="\n"
            ) as output:
                output.write(resources[spec.filename])

        for spec in RESOURCE_SPECS:
            os.replace(
                temporary_paths[spec.filename], output_directory / spec.filename
            )
    except OSError as error:
        raise ExportError(
            f"unable to write achievement resources in {output_directory}: {error}"
        ) from error
    finally:
        for path in temporary_paths.values():
            try:
                path.unlink()
            except FileNotFoundError:
                pass


def export_resources(client: MySQLClient, output_directory: Path) -> None:
    rows_by_filename = {spec.filename: client.fetch(spec) for spec in RESOURCE_SPECS}
    resources = render_resources(rows_by_filename)
    _write_resources(output_directory, resources)


def _build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Export the achievement presentation tables from the content "
            "database to the five client resource files."
        )
    )
    parser.add_argument(
        "output_directory",
        type=Path,
        help="directory to receive the five achievement resource files",
    )
    parser.add_argument(
        "--database",
        required=True,
        help="content database schema name",
    )
    parser.add_argument(
        "--mysql-client",
        default="mysql",
        help="mysql-compatible client executable (default: mysql)",
    )
    parser.add_argument(
        "--defaults-extra-file",
        type=Path,
        help=(
            "MySQL client option file containing connection settings; credentials "
            "are never accepted on this script's command line"
        ),
    )
    parser.add_argument(
        "--login-path",
        help="named login created with mysql_config_editor",
    )
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    arguments = _build_argument_parser().parse_args(argv)
    try:
        client = MySQLClient(
            database=arguments.database,
            executable=arguments.mysql_client,
            defaults_extra_file=arguments.defaults_extra_file,
            login_path=arguments.login_path,
        )
        export_resources(client, arguments.output_directory)
    except ExportError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    print(f"Exported achievement resources to {arguments.output_directory}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
