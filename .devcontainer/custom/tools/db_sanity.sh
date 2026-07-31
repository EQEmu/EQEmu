#!/usr/bin/env bash
# AoTv4 -- "is the database I am talking to the one I was working on?"          2026-07-30
# =============================================================================================
# RUN THIS FIRST, BEFORE STARTING THE STACK OR APPLYING ANY SQL.
#
# ⚠️⚠️ THE FAILURE THIS CATCHES IS SILENT. The server boots, the DB answers every query, and
# nothing anywhere reports an error -- you are simply connected to an OLDER COPY of peq than the
# one yesterday's work went into. It has happened at least twice (2026-07-24 and 2026-07-30); the
# second time it cost most of a session and looked exactly like "six days of work vanished".
#
# The root cause both times was that /var/lib/mysql was NOT the persistent named volume the
# devcontainer.json says it is -- the live DB was sitting on some container's writable layer, and a
# rebuild produced a container that mounted the (stale) volume instead. See CLAUDE.md §25.
#
# The trick that makes this detectable at all: /src is a BIND MOUNT, so artifacts on it survive
# every container. Two of them are generated FROM the database and are therefore datable evidence
# of what the DB contained at a given moment:
#     build/bin/shared/items  +  build/bin/shared/spells   (written by ./shared_memory)
#     build/bin/logs/**                                    (written by the running server)
# If either is meaningfully NEWER than the newest file in the datadir, the DB in front of you did
# not produce them, and you are looking at the wrong copy.
#
# Exit status: 0 = consistent, 1 = SUSPECT (read the output, do not start the stack).
# =============================================================================================

set -uo pipefail

DATADIR=/var/lib/mysql/peq
SHARED=/src/build/bin/shared
LOGS=/src/build/bin/logs
DB_HOST=127.0.0.1
DB_USER=peq
DB_PASS=peqpass
DB_NAME=peq

# a datadir file is only touched when InnoDB actually flushes, so use the NEWEST of them.
# ⚠️⚠️ sudo IS MANDATORY on the datadir. The per-database dirs are `drwx------ mysql`, so an
# unprivileged find returns NOTHING and the check silently reports "n/a" -- which reads as "no
# data" on a machine holding the full database. This is the same trap CLAUDE.md §17 records for
# `docker exec` needing `-u root`, and it caught this script during its own first run.
newest_epoch() { sudo find "$1" -type f -printf '%T@\n' 2>/dev/null | sort -rn | head -1 | cut -d. -f1; }

# ⚠️ `logs/` holds ~90,000 files and /src is a 9p mount, where a full recursive find costs ~15
# SILENT seconds -- the check looked hung after printing its header. Only the top-level logs are
# written live by a running stack (world_manual, zone-dynamic_NN, ...), so scan depth 1 and take the
# zone/ subdirectory's own mtime, which advances whenever a new zone log is created. Fast, and it
# answers the same question: "when did a server last run here?"
newest_epoch_shallow() {
    { sudo find "$1" -maxdepth 1 -type f -printf '%T@\n' 2>/dev/null
      sudo find "$1" -maxdepth 1 -type d -printf '%T@\n' 2>/dev/null
    } | sort -rn | head -1 | cut -d. -f1
}
show() { [ -n "${1:-}" ] && date -d "@$1" '+%Y-%m-%d %H:%M' || echo "n/a"; }

echo "=============================================================================="
echo " AoTv4 database sanity check          container: $(hostname)"
echo "=============================================================================="

# ---------------------------------------------------------------- is mariadb even up
if ! mysqladmin -h"$DB_HOST" -u"$DB_USER" -p"$DB_PASS" ping >/dev/null 2>&1; then
    echo "!! MariaDB is not answering on $DB_HOST. Start it:  sudo service mariadb start"
    exit 1
fi

# ⚠️ Say what is happening BEFORE each scan. A check that prints a header and then sits silent for
# 15 seconds reads as hung, and the first person to run this killed it rather than waiting.
printf "  scanning datadir ... "        ; DB_T=$(newest_epoch "$DATADIR")        ; echo "done"
printf "  scanning shared_memory ... "  ; SH_T=$(newest_epoch "$SHARED")         ; echo "done"
printf "  scanning logs ... "           ; LG_T=$(newest_epoch_shallow "$LOGS")   ; echo "done"

echo
echo "  datadir newest write   $(show "$DB_T")   ($DATADIR)"
echo "  shared_memory blobs    $(show "$SH_T")   ($SHARED)"
echo "  server logs            $(show "$LG_T")   ($LOGS)"

# ---------------------------------------------------------------- the actual test
# ⚠️ A generous slack: shared memory and logs are written by the server WHILE the DB is live, so
# they are legitimately a little newer. Hours of gap is the signal, not minutes.
SLACK=$((6 * 3600))
SUSPECT=0

for pair in "shared_memory:$SH_T" "server logs:$LG_T"; do
    label=${pair%%:*}; t=${pair##*:}
    if [ -n "$t" ] && [ -n "$DB_T" ] && [ "$t" -gt $((DB_T + SLACK)) ]; then
        gap=$(( (t - DB_T) / 3600 ))
        echo
        echo "  !! $label is ${gap}h NEWER than the database's own files."
        echo "     Whatever generated it was talking to a DIFFERENT copy of peq than this one."
        SUSPECT=1
    fi
done

# ---------------------------------------------------------------- content markers
# ⚠️ These are dated features. Checked by NAME as well as id, because an id range alone cannot tell
# "never created" apart from "renumbered". Add a row here whenever a datable feature lands.
echo
echo "  content markers (expected on a CURRENT database):"
# ⚠️ Read via PROCESS SUBSTITUTION, not `mysql | while`. A pipeline runs its right-hand side in a
# SUBSHELL, so a SUSPECT=1 set inside the loop is discarded when the loop exits -- the check then
# prints every marker as STALE and still exits 0 "consistent". It did exactly that on first run.
while IFS=$'\t' read -r what n want; do
    flag=""
    if [ "${want:-0}" -gt 0 ] && [ "${n:-0}" -lt "${want:-0}" ]; then
        flag="   <-- expected >= $want  STALE?"
        SUSPECT=1
    fi
    printf "    %-24s %s%s\n" "$what" "$n" "$flag"
done < <(mysql -h"$DB_HOST" -u"$DB_USER" -p"$DB_PASS" "$DB_NAME" -N -B 2>/dev/null <<'SQL'
SELECT 'custom spells 43xxx',   COUNT(*), 250 FROM spells_new WHERE id >= 43000;
SELECT 'Sinew line (07-29)',    COUNT(*),   6 FROM spells_new WHERE name LIKE 'Sinew%';
SELECT 'Delver Sigil (07-30)',  COUNT(*),  10 FROM items      WHERE Name LIKE '%Delver%Sigil%';
SELECT 'max npc_types id',      MAX(id),   0  FROM npc_types;
SELECT 'data_buckets rows',     COUNT(*),  0  FROM data_buckets;
SQL
)

echo
if [ "$SUSPECT" -ne 0 ]; then
    echo "  RESULT: SUSPECT -- this looks like an OLD copy of peq."
    echo "          DO NOT start the stack and DO NOT apply SQL: it would go into the wrong"
    echo "          database and appear to succeed."
    echo "          Most likely you are in a dev container built on the WRONG DOCKER ENGINE, so"
    echo "          /var/lib/mysql is a DIFFERENT volume that happens to share the name."
    echo "          Check:  hostname   against   docker ps -a   on the host -- if this container"
    echo "          is not in that list, that is the problem.  Full write-up: CLAUDE.md \$25."
    exit 1
fi

echo "  RESULT: consistent. The datadir is at least as new as everything generated from it."
exit 0
