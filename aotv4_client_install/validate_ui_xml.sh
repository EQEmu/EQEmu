#!/usr/bin/env bash
# Validate the EQUI_*.xml files we ship before copying them into the client.
#
# WHY: the RoF2 SIDL parser aborts the ENTIRE file on one bad token and the client then CRASHES at
# UI load, with only "ParseNodeList() SyntaxError" in <EQ>/UIErrorLog.txt to go on -- and the line it
# names is where it gave up, not where the mistake is. This has cost two debugging sessions already,
# both times a double hyphen inside an XML comment.
#
# There is no xmllint/python/node in this container, so these are hand-rolled checks:
#   1. double hyphen inside an XML comment  (illegal XML; the actual killer both times)
#   2. unbalanced tags
#   3. every <Pieces> target is defined in the same file
#   4. modern-only tags RoF2-shipped UI files never use (<Sortable>, column <Tooltip>)
#
#     bash aotv4_client_install/validate_ui_xml.sh [file.xml ...]     (defaults to all of ours)
set -uo pipefail
cd "$(dirname "$0")/.." || exit 1

FILES=("$@")
if [ ${#FILES[@]} -eq 0 ]; then
  mapfile -t FILES < <(ls aotv4_client_install/uifiles_default/*.xml default/EQUI_Native*.xml 2>/dev/null)
fi

rc=0
for f in "${FILES[@]}"; do
  [ -f "$f" ] || { echo "?? missing: $f"; rc=1; continue; }
  problems=""

  # 1. "--" inside a comment. Mask the real <!-- and --> delimiters first, then anything left is illegal.
  hits=$(sed 's/<!--/\xc2\xab/g; s/-->/\xc2\xbb/g' "$f" | grep -n -- "--")
  [ -n "$hits" ] && problems+=$'\n  DOUBLE HYPHEN IN COMMENT (crashes the client at UI load):\n'"$(echo "$hits" | sed 's/^/    /')"

  # 2. tag balance (comments stripped)
  unb=$(sed 's/<!--/\n\xc2\xab/g; s/-->/\xc2\xbb\n/g' "$f" | grep -v $'^\xc2\xab' \
        | grep -o '</\?[A-Za-z_][A-Za-z0-9_]*' | sed 's/^<//' \
        | awk '{if(substr($0,1,1)=="/") shut[substr($0,2)]++; else open[$0]++}
               END{for(t in open) if(open[t]!=shut[t] && t!="Schema") printf "    %-16s open=%d close=%d\n",t,open[t],shut[t]}')
  [ -n "$unb" ] && problems+=$'\n  UNBALANCED TAGS:\n'"$unb"

  # 3. <Pieces> targets must be defined in the same file.
  #    SIDL also accepts a type-qualified form, "<Pieces>Screen:NAW_SummaryPanel</Pieces>", so strip
  #    any leading "Type:" before looking the item up (EQUI_NativeAchievementWnd.xml uses that form).
  for p in $(grep -oE "<Pieces>[^<]+</Pieces>" "$f" | sed 's/<[^>]*>//g;s/^[A-Za-z]*://'); do
    grep -q "item=\"$p\"" "$f" || problems+=$'\n  UNDEFINED PIECE: '"$p"
  done

  # 4. tags no RoF2-shipped UI file uses
  mod=$(grep -nE "<Sortable>|<Tooltip>" "$f")
  [ -n "$mod" ] && problems+=$'\n  MODERN-ONLY TAG (not seen in any RoF2 file):\n'"$(echo "$mod" | sed 's/^/    /')"

  if [ -n "$problems" ]; then echo "FAIL $f$problems"; rc=1; else echo "ok   $f"; fi
done

echo
[ $rc -eq 0 ] && echo "All good -- safe to copy into <EQ>/uifiles/default/." \
              || echo "Fix the above BEFORE copying, or the client will fail to load its UI."
exit $rc
