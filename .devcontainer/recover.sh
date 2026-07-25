#!/usr/bin/env bash
# AoTv4 post-rebuild recovery — one command to get the server up.
# Idempotent: seeds the peq DB only if the (now persistent) volume is empty, then
# rebuilds shared memory and starts the 5-process stack. Safe to re-run.
#
# See POST_REBUILD_RECOVERY.md / CLAUDE.md §2. Run from anywhere:
#     bash /src/.devcontainer/recover.sh
set -uo pipefail

BIN=/src/build/bin
SNAP=/src/peq_recovered.sql.gz      # canonical recovery point (2026-07-24, incl. achievements)
SNAP_OLD=/src/aotv4_current.sql     # Jul 21 fallback — PREDATES the achievement system
DBPASS=peqpass

say(){ printf '\n=== %s ===\n' "$*"; }

say "1. Start MariaDB"
sudo service mariadb start
for i in $(seq 1 10); do
  (ss -ltn 2>/dev/null || netstat -ltn) | grep -q ':3306' && break
  sleep 1
done
(ss -ltn 2>/dev/null || netstat -ltn) | grep -q ':3306' && echo "3306 UP" || { echo "MariaDB DOWN — abort"; exit 1; }

say "2/3. Seed peq DB + grants (only if empty)"
TABLES=$(sudo mariadb -N -e "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='peq';" 2>/dev/null)
if [ "${TABLES:-0}" -lt 100 ]; then
  # STOP. Seeding is NOT the first move -- see POST_REBUILD_RECOVERY.md step 0 / CLAUDE.md §17.
  # On 2026-07-24 an empty volume did NOT mean the DB was gone: the PREVIOUS container still had the
  # full datadir, and importing a snapshot here would have discarded 3 days of work (the whole
  # achievement catalog, which exists nowhere on disk). Check the old containers FIRST.
  cat <<'WARN'

  !!  peq is EMPTY -- do NOT seed until you have checked the previous container.  !!

  A container rebuild leaves the OLD container holding the real datadir. On the Windows host:

    docker ps -a --format "{{.ID}}  {{.CreatedAt}}  {{.Status}}  {{.Image}}"
    docker exec -u root <ID> sh -c "ls /var/lib/mysql | tr '\n' ' '; echo"    # a `peq` entry = jackpot

  ( -u root is MANDATORY: as `vscode` the per-DB dirs are drwx------ mysql, so the check
    falsely reports "no DB" on a container that has the full database. )

  If a container has peq, recover it (CLAUDE.md §17) instead of seeding.
  To seed anyway, re-run with:   SEED=1 bash /src/.devcontainer/recover.sh

WARN
  [ "${SEED:-0}" = "1" ] || { echo "aborting without seeding."; exit 1; }
  if [ -f "$SNAP" ]; then
    echo "seeding from $SNAP ..."
    gzip -dc "$SNAP" | sudo mariadb || { echo "import failed"; exit 1; }
  elif [ -f "$SNAP_OLD" ]; then
    echo "WARNING: $SNAP missing, falling back to $SNAP_OLD (PREDATES achievements/43xxx spells/auras)"
    pv -f "$SNAP_OLD" 2>/dev/null | sudo mariadb || { echo "import failed"; exit 1; }
  else
    echo "NO SNAPSHOT on /src — cannot seed."; exit 1
  fi
else
  echo "peq already has $TABLES tables (persistent volume intact) — skipping import."
fi
# peq@'%' as well as @127.0.0.1: a PUBLISHED docker port arrives from the gateway (172.17.0.1), not
# loopback, so HeidiSQL on 127.0.0.1:3307 fails "Access denied" with only the @127.0.0.1 user. (§17)
sudo mariadb -e "CREATE USER IF NOT EXISTS 'peq'@'127.0.0.1' IDENTIFIED BY '$DBPASS';
                 GRANT ALL PRIVILEGES ON *.* TO 'peq'@'127.0.0.1';
                 CREATE USER IF NOT EXISTS 'peq'@'%' IDENTIFIED BY '$DBPASS';
                 GRANT ALL PRIVILEGES ON *.* TO 'peq'@'%'; FLUSH PRIVILEGES;"

say "4. Verify (expect chars>=1, hallowed/mythic>0, spelldmg_rule=true)"
mysql -h127.0.0.1 -upeq -p"$DBPASS" peq -N -e "
  SELECT 'chars', COUNT(*) FROM character_data
  UNION ALL SELECT 'hallowed', COUNT(*) FROM items WHERE id BETWEEN 300000 AND 599999
  UNION ALL SELECT 'mythic',   COUNT(*) FROM items WHERE id BETWEEN 600000 AND 899999
  UNION ALL SELECT 'spelldmg_rule', rule_value FROM rule_values WHERE rule_name='Spells:IgnoreSpellDmgLvlRestriction'
  UNION ALL SELECT 'aa_bard_tagged', COUNT(*) FROM aa_ability WHERE classes & 128;" || { echo "DB verify failed"; exit 1; }

say "5. Rebuild shared memory (world must be down — nothing started yet)"
( cd "$BIN" && ./shared_memory 2>&1 | tail -3 )

say "6. Start the stack (detached)"
cd "$BIN"
for p in loginserver world eqlaunch ucs queryserv; do pkill -x "$p" 2>/dev/null; done
sleep 1
setsid nohup ./loginserver > logs/loginserver_manual.log 2>&1 < /dev/null &
sleep 2
setsid nohup ./world       > logs/world_manual.log       2>&1 < /dev/null &
sleep 6
setsid nohup ./eqlaunch zone > logs/eqlaunch_manual.log  2>&1 < /dev/null &
setsid nohup ./ucs         > logs/ucs_manual.log         2>&1 < /dev/null &
setsid nohup ./queryserv   > logs/queryserv_manual.log   2>&1 < /dev/null &
sleep 6

say "7. Health"
for p in loginserver world eqlaunch ucs queryserv; do echo "$p: $(pgrep -x "$p" | wc -l)"; done
echo "--- login UDP ports (need 5998 + 5999 bound by loginserver) ---"
(ss -lunp 2>/dev/null || netstat -lunp) | grep -E ':5998|:5999'
echo "--- world DB connect ---"
grep -iE 'Connected to (the )?database' logs/world_manual.log | tail -1
grep -iE 'fatal|Cannot continue|Access denied' logs/world_manual.log logs/loginserver_manual.log && echo "!! CHECK LOGS" || echo "no fatal errors"
echo
echo "Done. Client: account mikethefiend, eqhost Host=127.0.0.1:5999, run WINDOWED."
