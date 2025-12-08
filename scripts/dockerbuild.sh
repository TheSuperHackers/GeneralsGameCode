#!/bin/bash
#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
RESOURCES_DIR=$SCRIPT_DIR/../resources


usage() {
  cat <<EOF
Usage:
  $(basename "$0") [OPTIONS] [TARGET]

TARGET is passed to ninja.exe

Options:
  -h, --help            Show this help and exit
  --cmake               Force run cmake.
  --bash                Run the container interactively with bash entrypoint.

Examples:
  $(basename "$0")
  $(basename "$0") my_target
  $(basename "$0") --cmake
  $(basename "$0") --cmake my_target
  $(basename "$0") --bash
EOF
}


FORCE_CMAKE="false"
MAKE_TARGET=""
INTERACTIVE="false"
while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      usage
      exit 0
      ;;
    --cmake)
      FORCE_CMAKE="true"
      shift
      ;;
    --bash)
      INTERACTIVE="true"
      shift
      ;;
    --) # end of options
      shift
      break
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage
      exit 1
      ;;
    *)
      # Positional TARGET (accept only one)
      MAKE_TARGET="$MAKE_TARGET $1"
      shift
      ;;
  esac
done


docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g)  $RESOURCES_DIR/dockerbuild -t zerohour-build
if [[ "$INTERACTIVE" == "true" ]]; then
    FLAGS="  -it --entrypoint bash"
else
    FLAGS=""
fi
docker run   -u $(id -u):$(id -g) -e MAKE_TARGET="$MAKE_TARGET" -e FORCE_CMAKE=$FORCE_CMAKE -v "`pwd`":/build/cnc --rm $FLAGS zerohour-build





