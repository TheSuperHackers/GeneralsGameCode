#!/bin/sh

# BSD Zero Clause License
#
# Copyright (c) 2025 zzambers
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

set -eu

scriptName="$0"

printHelp() {
cat << EOF
USAGE:
${scriptName} [OPTION]... SRC_DIR

Fixes filename case include directives of source files found in SRC_DIR,
as necessary. It also fixes slashes ( \\ -> / ).

Details:
Script tries to find included file in include paths. (first relative to source
file's directory, then in specified include paths). First it tries
case-sensitive search in all paths. If that fails, it tries case-insensitive
search in all include paths (excpet for nofix paths). If it succseeds, it fixes
filename in include directory as necessary. If it fails no change is done.
Slashes are fixed either way, if necessary ( \\ -> / ).

Limitations:
Script does not support include paths with spaces.
Script cannot fix case in include directive starting with ../ (parent dir),
except for fixing slashes.

OPTIONS:
-I [PATH]
--include-path [PATH]
    include path, where to search included files. Also used to fix filename
    case as necessary, if possible. This argument can be specified
    more than once.

-N [PATH]
--include-paths-nofix [PATH]
    similar include-path, but files here are not considered when trying to fix
    case (only in search, if file exists) This argument can be specified
    more than once.
EOF
exit 0
}

includePaths=""
includePathsNofix=""
ignoredIncludes=""

fixFile() {
    file="$1"
    regex='^[[:space:]]*#[[:space:]]*include[[:space:]]*["<](.*[.][hH])[">].*$'
    includeLines="$( cat "$file" | grep -E "$regex" )" || :
    if [ -z "${includeLines}" ] ; then
        return
    fi
    dir="$(dirname "${file}" )"

    printf "%s\n" "${includeLines}" | while IFS='' read -r includeL ; do
        includeOrig="$( echo "$includeL" | sed -E "s;${regex};\\1;g" )"
        include="$( echo "$includeOrig" | sed -E 's;\\+;/;g' )"

        if echo "${ignoredIncludes:-}" | grep -qi "^${include}\$" ; then
            continue
        fi
        found=0
        for path in "$( dirname "${file}" )" ${includePaths:-} ${includePathsNofix:-} ; do
            if [ -e "${path}/${include}" ] ; then
                found=1
                break
            fi
        done
        includeOrigEscaped="$( echo "${includeOrig}" | sed 's;\\;[\\];g' )"
        updated=0
        if [ $found -eq 0 ]; then
            echo "Srcfile: ${file}"
            echo "missing: ${include}"

            includeLower="$( echo "${include}" | tr '[:upper:]' '[:lower:]' )"
            for path in "$( dirname "${file}" )" ${includePaths:-} ; do
                for testedFile in $( find "${path}" -iname "$( basename "${include}" )" ) ; do
                    testedLower="$( echo "${testedFile}" | tr '[:upper:]' '[:lower:]' )"
                    if echo "${testedLower}" | grep -q "${includeLower}\$" ; then
                        fixed="${testedFile#${path}/}"
                        fixedLower="$( echo "$fixed" | tr '[:upper:]' '[:lower:]' )"
                        if [ "${includeLower}" = "${fixedLower}" ] ; then
                            echo "fix: ${testedFile#${path}/}"
                            updated=1
                            sed -E -i "s;([\"<])${includeOrigEscaped}([\">]);\\1${fixed}\\2;g" "${file}"
                            break
                        fi
                    fi
                done
                [ $updated -eq 1 ] && break
            done
            echo
        fi
        [ $updated -eq 1 ] && continue
        if ! [ "${include}" = "${includeOrig}" ] ; then
            echo "Srcfile: ${file}"
            echo "orig: ${includeOrig}"
            echo "fix: ${include}"
            sed -E -i "s;([\"<])${includeOrigEscaped}([\">]);\\1${include}\\2;g" "${file}"
            echo
        fi
    done
}

fixTree() {
    find "$@" | grep -iE '[.](c|cpp|h)$' | while IFS='' read -r file ; do
        fixFile "${file}"
    done
}

main() {
    while [ "$#" -gt 0 ] ; do
        case "$1" in
            -h|--help)
                printHelp
                ;;
            -I|--include-path)
                includePaths="$( printf '%s\nx' "${includePaths:-}$2" )"
                includePaths="${includePaths%x}" # to keep newline
                shift;
                shift;
                ;;
            -N|--include-paths-nofix)
                includePathsNofix="$( printf '%s\nx' "${includePathsNofix:-}$2" )"
                includePathsNofix="${includePathsNofix%x}" # to keep newline
                shift;
                shift;
                ;;
            *)
                printf "SRC_DIR:\n%s\n" "$@"
                if [ "x${includePaths:-}" != "x" ] ; then
                    printf 'INCLUDE PATHS:\n%s' "$includePaths"
                fi
                if [ "x${includePathsNofix:-}" != "x" ] ; then
                    printf 'INCLUDE PATHS NOFIX:\n%s' "$includePathsNofix"
                fi
                echo
                fixTree "$@"
                break;
                ;;
        esac
    done
}

main "$@"
