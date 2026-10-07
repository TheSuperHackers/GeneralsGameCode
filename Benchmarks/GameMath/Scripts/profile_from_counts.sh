#!/bin/sh
#
# Turns a raw counter dump into a call profile: calls per logic frame, per
# function.
#
#   sh profile_from_counts.sh <dump>                    list the sessions
#   sh profile_from_counts.sh <dump> <session>          whole session
#   sh profile_from_counts.sh <dump> <session> <a>-<b>  blocks a to b
#
# A fourth argument names the profile column; it defaults to "calls".
#
# The raw dump is what the in-game counters write: a block every 100 logic
# frames, each block a list of "gm_function count". A block whose frame number
# is not greater than the one before it starts a new session. Sessions are
# numbered from 1 in the order they were played.
#
# Nothing here knows what a call costs, and that is deliberate. A call count is
# a fact about the game; a nanosecond is a fact about one implementation on one
# machine. Selecting a load window by cost would bake that implementation into
# the profile, and the profile exists precisely to compare implementations.
# Windows are chosen by wall clock or by call volume, never by time per call.
#
# The output is the body of callcounts-zh.txt.

set -e

DUMP=$1
SESSION=$2
BLOCKS=$3
NAME=${4:-calls}

if [ -z "$DUMP" ] || [ ! -f "$DUMP" ]; then
    echo "usage: sh profile_from_counts.sh <dump> [session] [first-last] [name]" >&2
    exit 1
fi

tr -d '\r' < "$DUMP" | awk -v want="$SESSION" -v blocks="$BLOCKS" -v name="$NAME" '
    # gm_sqrtf is the float column of the sqrt row, gm_sqrt the double one.
    # lrint has no row of its own in the benchmark; rint stands in for it.
    function classify(f,   base, stem) {
        base = f
        sub(/^gm_/, "", base)
        if (base == "lrint")  { brow[f] = "rint"; bcol[f] = "d"; return }
        if (base == "lrintf") { brow[f] = "rint"; bcol[f] = "f"; return }
        if (base ~ /f$/) {
            stem = substr(base, 1, length(base) - 1)
            if (stem in isfn) { brow[f] = stem; bcol[f] = "f"; return }
        }
        brow[f] = base
        bcol[f] = "d"
    }

    BEGIN {
        session = 0
        prev_frame = 0
        n = split("sin cos tan asin acos atan atan2 sinh cosh tanh asinh " \
                  "acosh atanh exp exp2 expm1 log log10 log1p logb pow " \
                  "sqrt cbrt hypot ceil floor trunc round rint fabs fmod " \
                  "remainder copysign fmax fmin erf erfc lgamma j0 y0", names, " ")
        for (i = 1; i <= n; i++) isfn[names[i]] = 1
    }

    /^==== frame/ {
        frame = $3 + 0
        if (frame <= prev_frame) { session++; block = 0 }
        prev_frame = frame
        block++
        sblocks[session] = block
        sframes[session] += $4 + 0
        allframes += $4 + 0
        cur_s = session
        cur_b = block
        inwin = take()
        if (inwin) frames += $4 + 0
        next
    }

    NF == 2 && $2 ~ /^[0-9]+$/ {
        if (!($1 in seen)) { seen[$1] = 1; fn[++nf] = $1; classify($1) }
        run[$1] += $2
        if (inwin) sum[$1] += $2
        next
    }

    function take() {
        if (want == "") return 0
        if (cur_s + 1 != want + 0) return 0
        if (blocks == "") return 1
        split(blocks, r, "-")
        return (cur_b >= r[1] + 0 && cur_b <= (r[2] == "" ? r[1] : r[2]) + 0)
    }

    END {
        if (want == "") {
            printf "%-10s %10s %12s %14s\n", "session", "blocks", "frames", "minutes at 60Hz"
            for (s = 0; s <= session; s++)
                printf "%-10d %10d %12d %14.1f\n",
                       s + 1, sblocks[s], sframes[s], sframes[s] / 3600
            printf "\nPick one: sh profile_from_counts.sh <dump> <session> [first-last]\n"
            exit 0
        }

        if (frames == 0) {
            print "the selected window holds no blocks" > "/dev/stderr"
            exit 1
        }

        # Busiest first, by call volume in the window, then over the whole dump
        # so a function that is quiet here but busy elsewhere does not sink to
        # the bottom. No timing is involved anywhere.
        for (i = 1; i <= nf; i++) rank[i] = i
        for (i = 1; i < nf; i++)
            for (j = 1; j <= nf - i; j++) {
                a = fn[rank[j]]; b = fn[rank[j + 1]]
                if (sum[a] + 0 < sum[b] + 0 ||
                    (sum[a] + 0 == sum[b] + 0 && run[a] + 0 < run[b] + 0)) {
                    t = rank[j]; rank[j] = rank[j + 1]; rank[j + 1] = t
                }
            }

        printf "# session %s", want
        if (blocks != "") printf ", blocks %s", blocks
        printf ", %d frames, %.1f minutes at 60 Hz\n", frames, frames / 3600
        printf "# calls counted in the window, exactly as the counters recorded\n"
        printf "# them. The trailing rate is for reading only; the coefficient is\n"
        printf "# worked out from the counts where it is used, so nothing is lost\n"
        printf "# to rounding on the way.\n\n"

        printf "# %-12s %-7s %-5s %14s %12s\n", "", "", "", "calls", "per frame"
        printf "%-14s %-7s %-5s %14s\n", "profiles", "", "", name
        printf "%-14s %-7s %-5s %14d\n\n", "frames", "", "", frames

        for (i = 1; i <= nf; i++) {
            f = fn[rank[i]]
            printf "%-14s %-7s %-5s %14d %12.1f\n",
                   f, brow[f], bcol[f], sum[f], sum[f] / frames
            total += sum[f]
        }
        printf "\n# total %d calls, %.1f per frame\n", total, total / frames
    }
'
