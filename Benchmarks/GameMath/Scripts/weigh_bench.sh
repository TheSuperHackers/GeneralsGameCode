#!/bin/sh
#
# Weights the benchmark timings by how often the game actually calls each
# function, and writes bench-weighted.txt.
#
# A microbenchmark says how long one call takes. It does not say what that costs
# the game, because the expensive functions are not the frequent ones, and the
# mix shifts with the load. Combining the two gives milliseconds per logic
# frame, which is the number that decides whether _PC_24 or _PC_53 is cheaper.
#
# Inputs, read from Benchmarks/GameMath/Snapshots:
#   callcounts-zh.txt   calls per frame, one column per load profile
#   bench-*.txt         nanoseconds per call, one file per configuration
# and from Tests/GameMath/Snapshots:
#   math-summary.txt    rows that differ from macOS, per configuration
#
# Reads and writes Benchmarks/GameMath/Snapshots, wherever it is run from:
#   sh Benchmarks/GameMath/Scripts/weigh_bench.sh
#
# Windows writes CRLF, so line endings are normalised on the way in.

set -e

cd -- "$(dirname -- "$0")/../Snapshots"

PROFILE=callcounts-zh.txt
OUT=bench-weighted.txt
MISMATCH=../../../Tests/GameMath/Snapshots/math-summary.txt

if [ ! -f "$PROFILE" ]; then
    echo "call profile not found: $PROFILE" >&2
    exit 1
fi

if [ ! -f "$MISMATCH" ]; then
    echo "dump comparison not found: Tests/GameMath/Snapshots/math-summary.txt" >&2
    exit 1
fi

BENCHES=$(ls bench-*.txt 2>/dev/null | grep -v "^$OUT$" || true)
if [ -z "$BENCHES" ]; then
    echo "no benchmark results found (bench-*.txt)" >&2
    exit 1
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

tr -d '\r' < "$PROFILE" > "$WORK/profile"
tr -d '\r' < "$MISMATCH" > "$WORK/summary"

INPUTS="$WORK/profile"
for f in $BENCHES; do
    label=$(printf '%s' "$f" | sed -E 's/^bench-//; s/\.txt$//; s/^win-//; s/precise/prec/')
    tr -d '\r' < "$f" | sed "s|^|$label	|" > "$WORK/$label.tsv"
    INPUTS="$INPUTS $WORK/$label.tsv"
done

{
    printf 'GameMath cost per logic frame\n'
    printf 'generated %s\n\n' "$(date -u '+%Y-%m-%d %H:%M UTC')"

    printf 'Nanoseconds per call from bench-*.txt, multiplied by calls per frame\n'
    printf 'from %s:\n\n' "$PROFILE"
    printf '  ms per frame = calls per frame * ns per call / 1e6\n\n'
    printf 'The call profile is built without reference to any timing, so the same\n'
    printf 'profile can be laid against any implementation. This file is the step\n'
    printf 'where the two meet, and the only place milliseconds appear.\n\n'
    printf 'Only the GameMath columns of the benchmark are used; the system libm\n'
    printf 'ones are carried along for reference.\n\n'
    printf 'This is an upper bound. The benchmark calls each function in a tight\n'
    printf 'loop with a hot cache and a predicted branch.\n\n'

    printf 'Next to each cost in the matrix: how many rows of that route differ\n'
    printf 'from macOS in Tests/GameMath/Snapshots/math-summary.txt, or ok if none do.\n\n'

    awk -v prof="$WORK/profile" -v summary="$WORK/summary" '
        BEGIN {
            while ((getline line < summary) > 0) {
                n = split(line, tok, " ")
                if (line ~ /^row kind/) {
                    for (i = 4; i <= n; i++) scol[i] = tok[i]
                    continue
                }
                if (line ~ /^(through double|plain float) /)
                    for (i = 4; i <= n; i++) mism[tok[1] " " tok[2], scol[i]] = tok[i]
            }
            close(summary)
        }
        function colkey(c,   pc) {
            pc = (c ~ /PC24/) ? "1" : (c ~ /PC53/) ? "2" : "0"
            return substr(c, 1, 3) pc c
        }
        function bar(w, s,   i, r) {
            r = ""
            for (i = 0; i < w; i++) r = r s
            return r
        }
        function ns_of(c, f) {
            return (bcol[f] == "d") ? gmd[c, brow[f]] : gmf[c, brow[f]]
        }
        # The same call routed through the other entry point. gm_sqrtf and
        # gm_sqrt both become sqrt.d under "all double" and sqrt.f under
        # "all float"; the call counts never change, only the way in.
        function ns_as(c, f, want) {
            return (want == "d") ? gmd[c, brow[f]] : gmf[c, brow[f]]
        }
        function sysns_of(c, f) {
            return (bcol[f] == "d") ? sysd[c, brow[f]] : sysf[c, brow[f]]
        }
        function sysns_as(c, f, want) {
            return (want == "d") ? sysd[c, brow[f]] : sysf[c, brow[f]]
        }
        function differing(c, kind,   name) {
            name = c
            sub(/-rev$/, "", name)
            if (name ~ /^mac-/) return 0
            if (!((kind, name) in mism)) return -1
            return mism[kind, name] + 0
        }
        function verdict(n) {
            if (n < 0) return "?"
            return (n == 0) ? "ok" : "(" n ")"
        }

        FILENAME == prof {
            if ($1 ~ /^#/ || NF == 0) next
            if ($1 == "profiles") {
                for (i = 2; i <= NF; i++) pname[++np] = $i
                next
            }
            if ($1 == "frames") {
                for (i = 2; i <= NF; i++) pframes[i - 1] = $i
                next
            }
            fn[++nf] = $1
            brow[$1] = $2
            bcol[$1] = $3
            # Absolute call counts, one column per profile. The rate is derived
            # here rather than stored, so nothing is lost to rounding.
            for (i = 4; i <= 3 + np; i++) calls[$1, i - 3] = $i
            next
        }

        # bench rows: label, name, gm dbl, sys dbl, ratio, gm flt, sys flt, ratio
        {
            if (NF != 8 || $3 !~ /^[0-9.]+$/) next
            if (!(($1) in seen)) { seen[$1] = 1; cfg[++nc] = $1; ck[nc] = colkey($1) }
            gmd[$1, $2] = $3
            gmf[$1, $2] = $6
            sysd[$1, $2] = $4
            sysf[$1, $2] = $7
        }

        END {
            for (i = 1; i < nc; i++)
                for (j = 1; j <= nc - i; j++)
                    if (ck[j] > ck[j + 1]) {
                        t = ck[j]; ck[j] = ck[j + 1]; ck[j + 1] = t
                        t = cfg[j]; cfg[j] = cfg[j + 1]; cfg[j + 1] = t
                    }

            # The answer is read off the busiest profile: the plateau where the
            # highest call volume was seen. The others sit alongside it to show
            # how far the extremes are apart.
            decide = 1
            for (p = 1; p <= np; p++) {
                vol = 0
                for (i = 1; i <= nf; i++) vol += calls[fn[i], p] / pframes[p]
                if (vol > bestvol) { bestvol = vol; decide = p }
            }

            # ---- calls per frame ----

            print "---- calls per logic frame ----"
            print ""
            line = sprintf("%-12s %-8s", "gm function", "bench")
            for (p = 1; p <= np; p++)
                line = line sprintf(" %16s %12s", pname[p] " calls", "per frame")
            print line
            line = sprintf("%-12s %-8s", "------------", "--------")
            for (p = 1; p <= np; p++)
                line = line sprintf(" %16s %12s", bar(16, "-"), bar(12, "-"))
            print line

            for (i = 1; i <= nf; i++) {
                f = fn[i]
                line = sprintf("%-12s %-8s", f, brow[f] "." bcol[f])
                for (p = 1; p <= np; p++)
                    line = line sprintf(" %16d %12.4f",
                                        calls[f, p], calls[f, p] / pframes[p])
                print line
            }
            line = sprintf("%-12s %-8s", "frames", "")
            for (p = 1; p <= np; p++) line = line sprintf(" %16d %12s", pframes[p], "")
            print line

            # ---- nanoseconds per call ----

            print ""
            print "---- nanoseconds per call ----"
            print ""
            line = sprintf("%-12s %-8s", "gm function", "bench")
            for (c = 1; c <= nc; c++) line = line sprintf(" %16s", cfg[c])
            print line sprintf(" %10s", "sys libm")
            line = sprintf("%-12s %-8s", "------------", "--------")
            for (c = 1; c <= nc; c++) line = line sprintf(" %16s", bar(16, "-"))
            print line sprintf(" %10s", bar(10, "-"))

            for (i = 1; i <= nf; i++) {
                f = fn[i]
                line = sprintf("%-12s %-8s", f, brow[f] "." bcol[f])
                for (c = 1; c <= nc; c++) line = line sprintf(" %16.1f", ns_of(cfg[c], f))
                print line sprintf(" %10.1f", sysns_of(cfg[1], f))
            }

            # ---- the superposition matrix ----
            #
            # Every call in the profile costed three ways: through the double
            # entry point, through the float one, and as the game reaches them
            # today. Across every architecture and precision control that was
            # measured, this is the grid the choice is made from.

            for (p = 1; p <= np; p++) {
                printf "\n---- superposition matrix, %s profile (%d frames) ----\n\n",
                       pname[p], pframes[p]

                print "ms per logic frame. Same calls, same counts, different way in."
                print ""
                print "Next to each cost: rows of that route that differ from macOS in"
                print "Tests/GameMath/Snapshots/math-summary.txt, or ok if none do."
                print ""
                print "  as the game  through double and plain float rows"
                print "  all double   through double rows"
                print "  all float    plain float rows"
                print ""

                printf "%-22s  %17s  %17s  %17s  %s\n",
                       "configuration", "as the game", "all double", "all float",
                       "fastest valid"
                printf "%-22s  %17s  %17s  %17s  %s\n",
                       bar(22, "-"), bar(17, "-"), bar(17, "-"), bar(17, "-"),
                       bar(13, "-")

                for (c = 1; c <= nc; c++) {
                    mix = 0; alld = 0; allf = 0
                    for (i = 1; i <= nf; i++) {
                        rate = calls[fn[i], p] / pframes[p]
                        mix  += rate * ns_of(cfg[c], fn[i]) / 1e6
                        alld += rate * ns_as(cfg[c], fn[i], "d") / 1e6
                        allf += rate * ns_as(cfg[c], fn[i], "f") / 1e6
                    }

                    nd = differing(cfg[c], "through double")
                    nflt = differing(cfg[c], "plain float")
                    nm = (nd < 0 || nflt < 0) ? -1 : nd + nflt
                    fm = verdict(nm); fd = verdict(nd); ff = verdict(nflt)

                    best = "none"; low = 0
                    if (nm == 0)   { best = "as the game"; low = mix }
                    if (nd == 0 && (best == "none" || alld < low)) { best = "all double"; low = alld }
                    if (nflt == 0 && (best == "none" || allf < low)) { best = "all float";  low = allf }

                    printf "%-22s  %9.4f %-7s  %9.4f %-7s  %9.4f %-7s  %s\n",
                           cfg[c], mix, fm, alld, fd, allf, ff, best
                }
            }

            # ---- GameMath against the platform library ----
            #
            # What the deterministic library costs over whatever the platform
            # already ships. The system column is the same calls at the same
            # counts, costed with the system entry points.

            for (p = 1; p <= np; p++) {
                printf "\n---- GameMath against the system library, %s profile ----\n\n",
                       pname[p]
                print "ms per logic frame, and what GameMath adds over the platform."
                print ""
                printf "%-22s  %19s  %19s\n", "", "as the game", "all float"
                printf "%-22s  %8s %9s  %8s %9s\n",
                       "configuration", "GameMath", "system", "GameMath", "system"
                printf "%-22s  %8s %9s  %8s %9s\n",
                       bar(22, "-"), bar(8, "-"), bar(9, "-"), bar(8, "-"), bar(9, "-")

                for (c = 1; c <= nc; c++) {
                    gmix = 0; smix = 0; gflt = 0; sflt = 0
                    for (i = 1; i <= nf; i++) {
                        rate = calls[fn[i], p] / pframes[p]
                        gmix += rate * ns_of(cfg[c], fn[i]) / 1e6
                        smix += rate * sysns_of(cfg[c], fn[i]) / 1e6
                        gflt += rate * ns_as(cfg[c], fn[i], "f") / 1e6
                        sflt += rate * sysns_as(cfg[c], fn[i], "f") / 1e6
                    }
                    printf "%-22s  %8.4f %9.4f  %8.4f %9.4f   x%.2f / x%.2f\n",
                           cfg[c], gmix, smix, gflt, sflt,
                           (smix > 0 ? gmix / smix : 0),
                           (sflt > 0 ? gflt / sflt : 0)
                }
            }

            # ---- ms per frame, one block per profile ----

            for (p = 1; p <= np; p++) {
                printf "\n---- ms per logic frame, %s profile (%d frames) ----\n\n",
                       pname[p], pframes[p]

                line = sprintf("%-12s %-8s %11s", "gm function", "bench", "calls/frame")
                for (c = 1; c <= nc; c++) line = line sprintf(" %16s", cfg[c])
                print line
                line = sprintf("%-12s %-8s %11s", "------------", "--------", "-----------")
                for (c = 1; c <= nc; c++) line = line sprintf(" %16s", bar(16, "-"))
                print line

                for (c = 1; c <= nc; c++) msum[c] = 0

                for (i = 1; i <= nf; i++) {
                    f = fn[i]
                    rate = calls[f, p] / pframes[p]
                    line = sprintf("%-12s %-8s %11.4f", f, brow[f] "." bcol[f], rate)
                    for (c = 1; c <= nc; c++) {
                        ms = rate * ns_of(cfg[c], f) / 1e6
                        msum[c] += ms
                        line = line sprintf(" %16.4f", ms)
                    }
                    print line
                }

                line = sprintf("%-12s %-8s %11s", "------------", "--------", "-----------")
                for (c = 1; c <= nc; c++) line = line sprintf(" %16s", bar(16, "-"))
                print line
                line = sprintf("%-12s %-8s %11s", "TOTAL", "", "")
                for (c = 1; c <= nc; c++) line = line sprintf(" %16.4f", msum[c])
                print line
                line = sprintf("%-12s %-8s %11s", "% of 16.67 ms", "", "")
                for (c = 1; c <= nc; c++) line = line sprintf(" %15.2f%%", msum[c] / 16.667 * 100)
                print line
            }

            # ---- the question ----

            print ""
            print "---- _PC_24 against _PC_53 ----"
            print ""

            npairs = 0
            for (c = 1; c <= nc; c++) {
                if (cfg[c] !~ /PC24/) continue
                mate = cfg[c]; sub(/PC24/, "PC53", mate)
                if (!(mate in seen)) continue
                npairs++

                base = cfg[c]; sub(/-PC24/, "", base)

                printf "%s, ms per logic frame\n\n", base
                print "profile        frames       PC24 ms       PC53 ms      delta ms   delta %"
                print "----------  ----------  ------------  ------------  ------------  --------"

                for (p = 1; p <= np; p++) {
                    a = 0; b = 0
                    for (i = 1; i <= nf; i++) {
                        rate = calls[fn[i], p] / pframes[p]
                        a += rate * ns_of(cfg[c], fn[i]) / 1e6
                        b += rate * ns_of(mate, fn[i]) / 1e6
                    }
                    printf "%-10s  %10d  %12.4f  %12.4f  %+12.4f  %+7.1f%%%s\n",
                           pname[p], pframes[p], a, b, a - b,
                           (b > 0 ? (a - b) / b * 100 : 0),
                           (p == decide ? "   <--" : "")
                }

                printf "\n%s, where the difference sits, %s profile\n\n", base, pname[decide]
                print "gm function   bench       calls/frame       PC24 ms       PC53 ms      delta ms   delta %"
                print "------------  --------  -------------  ------------  ------------  ------------  --------"

                t24 = 0; t53 = 0
                for (i = 1; i <= nf; i++) {
                    f = fn[i]
                    if (calls[f, decide] + 0 == 0) continue
                    rate = calls[f, decide] / pframes[decide]
                    a = rate * ns_of(cfg[c], f) / 1e6
                    b = rate * ns_of(mate, f) / 1e6
                    t24 += a; t53 += b
                    printf "%-12s  %-8s  %13.4f  %12.4f  %12.4f  %+12.4f  %+7.1f%%\n",
                           f, brow[f] "." bcol[f], rate, a, b, a - b,
                           (b > 0 ? (a - b) / b * 100 : 0)
                }
                print "------------  --------  -------------  ------------  ------------  ------------  --------"
                printf "%-12s  %-8s  %13s  %12.4f  %12.4f  %+12.4f  %+7.1f%%\n",
                       "TOTAL", "", "", t24, t53, t24 - t53,
                       (t53 > 0 ? (t24 - t53) / t53 * 100 : 0)
                print ""
            }

            if (npairs == 0) {
                print "No PC24 / PC53 pair among the results. The precision control only"
                print "exists on 32-bit x86; run the benchmark there to fill this in."
                print ""
            }
        }
    ' $INPUTS
} > "$OUT"

echo "wrote $OUT"
