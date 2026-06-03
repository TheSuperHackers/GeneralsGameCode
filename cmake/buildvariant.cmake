# Bakes a build-variant tag ("dev-a1b2c3d" / "release-a1b2c3d") into a
# generated header consumed by both the game engine (so every outbound
# HTTP request advertises which build sent it) and the launcher (so it
# knows whether to read latest.json vs latest-dev.json and which update
# gate to apply).
#
# The "kind" (dev vs release) is supplied by the caller via the
# ZULU_BUILD_VARIANT cache var (-DZULU_BUILD_VARIANT=...) or the
# ZULU_BUILD_VARIANT environment variable. Defaults to "dev" so a fresh
# clone or stray local build is auto-tagged; only the canonical release
# pipeline (Makefile target installer / installer-release) sets it to
# "release". The git short hash is appended automatically using the
# GIT_EXECUTABLE that the docker entrypoint already configures.

if(NOT DEFINED ZULU_BUILD_VARIANT OR ZULU_BUILD_VARIANT STREQUAL "")
    if(DEFINED ENV{ZULU_BUILD_VARIANT} AND NOT "$ENV{ZULU_BUILD_VARIANT}" STREQUAL "")
        set(ZULU_BUILD_VARIANT "$ENV{ZULU_BUILD_VARIANT}")
    else()
        set(ZULU_BUILD_VARIANT "dev")
    endif()
endif()

string(STRIP "${ZULU_BUILD_VARIANT}" ZULU_BUILD_VARIANT)
string(TOLOWER "${ZULU_BUILD_VARIANT}" ZULU_BUILD_VARIANT)

if(NOT ZULU_BUILD_VARIANT MATCHES "^(dev|release)$")
    message(FATAL_ERROR
        "ZULU_BUILD_VARIANT must be 'dev' or 'release' (got '${ZULU_BUILD_VARIANT}').")
endif()

# Git short hash. Fall back to "nohash" if git is unavailable or the
# working tree isn't a git repo (e.g. a tarball-based build), so the
# build doesn't fail just because the provenance signal is missing.
set(ZULU_BUILD_GIT_HASH "nohash")
if(DEFINED GIT_EXECUTABLE AND EXISTS "${GIT_EXECUTABLE}")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_hash
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _git_rc)
    if(_git_rc EQUAL 0 AND NOT _git_hash STREQUAL "")
        set(ZULU_BUILD_GIT_HASH "${_git_hash}")
    endif()
endif()

# Defensive scrub: reject anything that would break the C string literal.
if(ZULU_BUILD_GIT_HASH MATCHES "[\"\\\\\r\n ]")
    set(ZULU_BUILD_GIT_HASH "nohash")
endif()

set(ZULU_BUILD_VARIANT_TAG "${ZULU_BUILD_VARIANT}-${ZULU_BUILD_GIT_HASH}")

set(ZULU_BUILD_VARIANT_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/BuildVariant.h.in"
    "${ZULU_BUILD_VARIANT_GENERATED_DIR}/BuildVariant.h"
    @ONLY)
