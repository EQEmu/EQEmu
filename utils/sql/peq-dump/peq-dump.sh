#!/usr/bin/env bash
# Run from the context of server directory

world_path=""

#############################################
# world path
#############################################
if [ -d "bin" ]
then
    world_path="bin/"
fi

world_bin="${world_path}world"

echo "World path is [$world_path] bin is [$world_bin]"

#############################################
# dump
#############################################

dump_path=/tmp/peq-dump/
echo "Generating dump path [${dump_path}]"
rm -rf ${dump_path}
mkdir -p ${dump_path}

#############################################
# generate "drop_" table files
#############################################
echo "Generating [drop_*] table exports..."
bash -c "${world_bin} database:dump --content-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_content.sql"
bash -c "${world_bin} database:dump --login-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_login.sql"
bash -c "${world_bin} database:dump --player-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_player.sql"
bash -c "${world_bin} database:dump --system-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_system.sql"
bash -c "${world_bin} database:dump --state-tables --drop-table-syntax-only --dump-output-to-console > ${dump_path}drop_tables_state.sql"

#############################################
# generate "create_" table files
#############################################
echo "Generating [create_*] table exports..."

# structure only
bash -c "${world_bin} database:dump --login-tables --table-structure-only --dump-output-to-console | sed 's/ AUTO_INCREMENT=[0-9]*\b//g' > ${dump_path}create_tables_login.sql"
bash -c "${world_bin} database:dump --player-tables --table-structure-only --dump-output-to-console | sed 's/ AUTO_INCREMENT=[0-9]*\b//g' > ${dump_path}create_tables_player.sql"
bash -c "${world_bin} database:dump --state-tables --table-structure-only --dump-output-to-console | sed 's/ AUTO_INCREMENT=[0-9]*\b//g' > ${dump_path}create_tables_state.sql"
bash -c "${world_bin} database:dump --static-instance-data --dump-output-to-console >> ${dump_path}create_tables_state.sql"

# with content
bash -c "${world_bin} database:dump --content-tables --dump-output-to-console > ${dump_path}create_tables_content.sql"
bash -c "${world_bin} database:dump --system-tables --dump-output-to-console > ${dump_path}create_tables_system.sql"

#############################################
# "all" exports
#############################################
bash -c "cd ${dump_path} && ls * | grep create | sed 's/.*/source &;/' > create_all_tables.sql"
bash -c "cd ${dump_path} && ls * | grep drop | sed 's/.*/source &;/' > drop_all_tables.sql"

#############################################
# zip
#############################################
echo "Compressing..."
bash -c "cd /tmp/ && rm -rf peq-latest.zip && zip peq-latest.zip peq-dump/* && mv ${dump_path}peq-latest.zip /tmp/peq-latest.zip"

echo "Cleaning up..."
rm -rf ${dump_path}

echo "Dump located [/tmp/peq-latest.zip]"
