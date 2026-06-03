# Bakes the build-time cncstats auth key into a generated header that the
# game engine includes. The key is supplied by the caller via either the
# CNCSTATS_ZULU_CLIENT_KEY CMake cache variable
# (-DCNCSTATS_ZULU_CLIENT_KEY=...) or the CNCSTATS_ZULU_CLIENT_KEY
# environment variable. Configure fails if neither is set so a release
# build cannot silently ship without a key. Fetching the secret out of
# GCP Secret Manager is the caller's job (see scripts/docker-build.sh),
# which lets the docker build run without gcloud or GCP credentials.
#
# Mirrors cmake/zuluclientkey.cmake; kept as a separate module so the two
# keys can rotate independently.

if(NOT DEFINED CNCSTATS_ZULU_CLIENT_KEY OR CNCSTATS_ZULU_CLIENT_KEY STREQUAL "")
    if(DEFINED ENV{CNCSTATS_ZULU_CLIENT_KEY} AND NOT "$ENV{CNCSTATS_ZULU_CLIENT_KEY}" STREQUAL "")
        set(CNCSTATS_ZULU_CLIENT_KEY "$ENV{CNCSTATS_ZULU_CLIENT_KEY}")
    endif()
endif()

string(STRIP "${CNCSTATS_ZULU_CLIENT_KEY}" CNCSTATS_ZULU_CLIENT_KEY)

if(CNCSTATS_ZULU_CLIENT_KEY STREQUAL "")
    message(FATAL_ERROR
        "CNCSTATS_ZULU_CLIENT_KEY is not set. Pass it via "
        "-DCNCSTATS_ZULU_CLIENT_KEY=... or set the CNCSTATS_ZULU_CLIENT_KEY "
        "environment variable. scripts/docker-build.sh pulls it from GCP "
        "Secret Manager (secret cncstats_zuluclientkey) automatically.")
endif()

if(CNCSTATS_ZULU_CLIENT_KEY MATCHES "[\"\\\\\r\n]")
    message(FATAL_ERROR
        "CNCSTATS_ZULU_CLIENT_KEY contains characters that cannot be "
        "embedded in a C string literal (\", \\, CR, or LF).")
endif()

set(CNCSTATS_ZULU_CLIENT_KEY_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/CncStatsClientKey.h.in"
    "${CNCSTATS_ZULU_CLIENT_KEY_GENERATED_DIR}/CncStatsClientKey.h"
    @ONLY)
