#!/bin/sh
#
# Compares every math-*.txt dump against the macOS one and writes two files:
#   math-diff.txt      the differing lines of each comparison
#   math-summary.txt   one table, row kinds down the side, modes across the top
#
# Reads and writes Tests/GameMath/Snapshots, wherever it is run from:
#   sh Tests/GameMath/Scripts/compare_math.sh
#
# Windows writes CRLF, so line endings are normalised before comparing.

set -e

cd -- "$(dirname -- "$0")/../Snapshots"

OUT=math-diff.txt
SUM_OUT=math-summary.txt

BASE=$(ls math-mac-*.txt 2>/dev/null | head -1)
if [ -z "$BASE" ]; then
    echo "no macOS dump found (math-mac-*.txt)" >&2
    exit 1
fi

OTHERS=$(ls math-*.txt 2>/dev/null \
    | grep -v "^$BASE$" | grep -v "^$OUT$" | grep -v "^$SUM_OUT$" || true)
if [ -z "$OTHERS" ]; then
    echo "nothing to compare against $BASE" >&2
    exit 1
fi

norm() {
    tr -d '\r' < "$1" | grep -v '^================'
}

rows() {
    norm "$1" | wc -l | tr -d ' '
}

BASE_ROWS=$(rows "$BASE")
for f in $OTHERS; do
    if [ "$(rows "$f")" -ne "$BASE_ROWS" ]; then
        echo "$f has $(rows "$f") rows, $BASE has $BASE_ROWS; rebuild both dumps from the same verify_game_math.c." >&2
        exit 1
    fi
done

# Collected across all comparisons, pivoted into one table at the end.
SUMMARY=$(mktemp)
trap 'rm -f "$SUMMARY"' EXIT

short() {
    printf '%s' "$1" | sed -E 's/^win-//; s/precise/prec/'
}

# Reads the differing lines first, then the whole dump, counting per row suffix.
AWK_COMMON='
    function sfx(name,   i) {
        i = index(name, ".")
        return (i ? substr(name, i + 1) : "")
    }
    function grp(s) {
        if (s == "f2d")                return "float -> double"
        if (s ~ /2f$/)                 return "double -> float"
        if (s == "f" || s ~ /\.f$/)    return "plain float"
        return "through double"
    }
    FILENAME == ARGV[1] { s = sfx($1); if (s != "") { dif[s]++ } next }
    { s = sfx($1); if (s != "") { tot[s]++ } }
'

# $1 other file
pair_up() {
    a=$(mktemp)
    b=$(mktemp)
    norm "$BASE" > "$a"
    norm "$1" > "$b"

    label=$(printf '%s' "$1" | sed -E 's/^math-//; s/\.txt$//')

    d=$(mktemp)
    awk 'FILENAME == ARGV[1] { line[FNR] = $0; next } (line[FNR] "") != ($0 "") { print line[FNR] }' "$a" "$b" > "$d"
    count=$(wc -l < "$d" | tr -d ' ')

    printf '================ %s vs %s ================\n' \
        "$(printf '%s' "$BASE" | sed -E 's/^math-//; s/\.txt$//')" "$label"
    printf 'differing lines: %s\n\n' "$count"

    if [ "$count" -gt 0 ]; then
        printf '%-18s %9s %9s\n' "group" "differing" "of total"
        printf '%-18s %9s %9s\n' "------------------" "---------" "--------"
        awk "$AWK_COMMON"'
            END {
                order[1] = "through double"; order[2] = "float -> double"
                order[3] = "double -> float"; order[4] = "plain float"
                for (s in tot) { g = grp(s); tg[g] += tot[s]; dg[g] += dif[s] + 0 }
                for (i = 1; i <= 4; i++)
                    if (tg[order[i]] > 0)
                        printf "%-18s %9d %9d\n", order[i], dg[order[i]] + 0, tg[order[i]]
            }
        ' "$d" "$a"

        printf '\n%-18s %9s %9s\n' "row" "differing" "of total"
        printf '%-18s %9s %9s\n' "------------------" "---------" "--------"
        awk "$AWK_COMMON"'
            END {
                for (s in tot)
                    if (dif[s] > 0)
                        printf "%-18s %9d %9d\n", "." s, dif[s], tot[s]
            }
        ' "$d" "$a" | sort -k2,2nr -k1,1

        awk -v cfg="$(short "$label")" "$AWK_COMMON"'
            END {
                for (s in tot) {
                    g = grp(s)
                    tg[g] += tot[s]; dg[g] += dif[s] + 0
                    printf "%s\t.%s\t%d\t%d\n", cfg, s, dif[s] + 0, tot[s]
                    all_d += dif[s] + 0; all_t += tot[s]
                }
                for (g in tg)
                    printf "%s\t%s\t%d\t%d\n", cfg, g, dg[g] + 0, tg[g]
                printf "%s\t%s\t%d\t%d\n", cfg, "total", all_d, all_t
            }
        ' "$d" "$a" >> "$SUMMARY"

        rm -f "$d"
        printf '\n'
    else
        awk -v cfg="$(short "$label")" "$AWK_COMMON"'
            END {
                for (s in tot) {
                    g = grp(s)
                    tg[g] += tot[s]
                    printf "%s\t.%s\t0\t%d\n", cfg, s, tot[s]
                    all_t += tot[s]
                }
                for (g in tg)
                    printf "%s\t%s\t0\t%d\n", cfg, g, tg[g]
                printf "%s\t%s\t0\t%d\n", cfg, "total", all_t
            }
        ' /dev/null "$a" >> "$SUMMARY"
    fi

    if [ "$count" -gt 0 ]; then
        printf '%-20s %-46s %-18s %s\n' "row" "arguments" "macOS" "$label"
        printf '%-20s %-46s %-18s %s\n' \
            "--------------------" \
            "----------------------------------------------" \
            "------------------" "------------------"

        paste "$a" "$b" | while IFS="$(printf '\t')" read -r l r; do
            [ "$l" = "$r" ] && continue
            case "$l" in
                ----*) continue ;;
            esac
            row=$(printf '%s' "$l" | sed -E 's/ .*//')
            args=$(printf '%s' "$l" | sed -E 's/^[^ ]+ +//; s/ +[^ ]+$//')
            lv=$(printf '%s' "$l" | sed -E 's/.* //')
            rv=$(printf '%s' "$r" | sed -E 's/.* //')
            printf '%-20s %-46s %-18s %s\n' "$row" "$args" "$lv" "$rv"
        done
    fi

    printf '\n'
    rm -f "$a" "$b" "$d"
}

{
    printf 'GameMath cross-platform comparison\n'
    printf 'generated %s\n' "$(date -u '+%Y-%m-%d %H:%M UTC')"
    printf 'baseline %s\n\n' "$BASE"

    printf 'Each function is called on the same value several ways. The suffix on\n'
    printf 'the row name says which way, so a difference can be traced to the\n'
    printf 'function itself or to a conversion around it.\n\n'
    printf '  .d     double function, double result        gm_f(x)\n'
    printf '  .f     float function, float result          gm_ff((float)x)\n'
    printf '  .f2d   float function, result widened        (double)gm_ff((float)x)\n'
    printf '  .d2f   double function, result narrowed      (float)gm_f(x)\n\n'
    printf 'Two argument functions vary each argument on its own, since a value can\n'
    printf 'arrive as a full double or as one that already went through a float,\n'
    printf 'and each combination is recorded both ways round:\n\n'
    printf '  .dd     gm_f(x, y)\n'
    printf '  .dd2f   (float)gm_f(x, y)\n'
    printf '  .df     gm_f(x, (double)(float)y)\n'
    printf '  .df2f   (float)gm_f(x, (double)(float)y)\n'
    printf '  .fd     gm_f((double)(float)x, y)\n'
    printf '  .fd2f   (float)gm_f((double)(float)x, y)\n'
    printf '  .ff     gm_f((double)(float)x, (double)(float)y)\n'
    printf '  .ff2f   (float)gm_f((double)(float)x, (double)(float)y)\n\n'
    printf 'The .f2d row is what the WWMath wrappers do, so it is the one that\n'
    printf 'matters for the game.\n\n'
    printf 'Every function is also fed into a small expression, since a result that\n'
    printf 'is merely stored may be rounded correctly while the same result kept in\n'
    printf 'a register and used in arithmetic is not. r1, r2 and r3 are the function\n'
    printf 'applied to three consecutive inputs, in float and in double:\n\n'
    printf '  .mul.d  .mul.f   r1 * r2\n'
    printf '  .inv.d  .inv.f   1 / r1\n'
    printf '  .mad.d  .mad.f   r1 * r2 + r3\n\n'

    for f in $OTHERS; do
        pair_up "$f"
    done
} > "$OUT"

{
    printf 'Differing lines against %s, by row kind.\n\n' \
        "$(printf '%s' "$BASE" | sed -E 's/^math-//; s/\.txt$//')"

    sort -t"$(printf '\t')" -k1,1 "$SUMMARY" | awk -F'\t' '
        # Columns are grouped by precision control first, so the PC24 and PC53
        # pairs sit next to each other and can be read against one another.
        function colkey(c,   pc) {
            pc = (c ~ /PC24/) ? "1" : (c ~ /PC53/) ? "2" : "0"
            return substr(c, 1, 3) pc c
        }
        {
            if (!(($1) in seen)) { seen[$1] = 1; cfg[++nc] = $1; ck[nc] = colkey($1) }
            d[$2, $1] = $3
            tot[$2] = $4
            if (!(($2) in kseen)) { kseen[$2] = 1; key[++nk] = $2 }
        }
        END {
            order[1] = "through double"; order[2] = "float -> double"
            order[3] = "double -> float"; order[4] = "plain float"
            order[5] = "total"

            for (i = 1; i < nc; i++)
                for (j = 1; j <= nc - i; j++)
                    if (ck[j] > ck[j + 1]) {
                        t = ck[j]; ck[j] = ck[j + 1]; ck[j + 1] = t
                        t = cfg[j]; cfg[j] = cfg[j + 1]; cfg[j + 1] = t
                    }

            w = 18
            line = sprintf("%-*s %6s", w, "row kind", "rows")
            for (c = 1; c <= nc; c++) line = line sprintf(" %14s", cfg[c])
            print line
            line = sprintf("%-*s %6s", w, "------------------", "------")
            for (c = 1; c <= nc; c++) line = line sprintf(" %14s", "--------------")
            print line

            for (i = 1; i <= 5; i++) {
                k = order[i]
                if (!(k in tot)) continue
                line = sprintf("%-*s %6d", w, k, tot[k])
                for (c = 1; c <= nc; c++) line = line sprintf(" %14d", d[k, cfg[c]] + 0)
                print line
                if (i == 4) print ""
            }

            print ""
            for (i = 1; i <= nk; i++) {
                k = key[i]
                if (substr(k, 1, 1) != ".") continue
                line = sprintf("%-*s %6d", w, k, tot[k])
                for (c = 1; c <= nc; c++) line = line sprintf(" %14d", d[k, cfg[c]] + 0)
                print line
            }
        }
    '
} > "$SUM_OUT"

echo "wrote $OUT and $SUM_OUT"
