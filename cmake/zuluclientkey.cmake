# Bakes the build-time radarvan auth key into a generated header that the
# game engine includes. The key is supplied by the caller via either the
# ZULU_CLIENT_KEY CMake cache variable (-DZULU_CLIENT_KEY=...) or the
# ZULU_CLIENT_KEY environment variable. Configure fails if neither is set
# so a release build cannot silently ship without a key. Fetching the
# secret out of GCP Secret Manager is the caller's job (see Makefile),
# which lets the docker build run without gcloud or GCP credentials.

if(NOT DEFINED ZULU_CLIENT_KEY OR ZULU_CLIENT_KEY STREQUAL "")
    if(DEFINED ENV{ZULU_CLIENT_KEY} AND NOT "$ENV{ZULU_CLIENT_KEY}" STREQUAL "")
        set(ZULU_CLIENT_KEY "$ENV{ZULU_CLIENT_KEY}")
    endif()
endif()

string(STRIP "${ZULU_CLIENT_KEY}" ZULU_CLIENT_KEY)

if(ZULU_CLIENT_KEY STREQUAL "")
    message(FATAL_ERROR
        "ZULU_CLIENT_KEY is not set. Pass it via -DZULU_CLIENT_KEY=... or "
        "set the ZULU_CLIENT_KEY environment variable. The Makefile pulls "
        "it from GCP Secret Manager (secret zuluclientkey) automatically.")
endif()

# Reject any character that would break out of the C string literal in the
# generated header. The auth token format is up to the user but quotes,
# backslashes, and CR/LF are not safe.
if(ZULU_CLIENT_KEY MATCHES "[\"\\\\\r\n]")
    message(FATAL_ERROR
        "ZULU_CLIENT_KEY contains characters that cannot be embedded in a "
        "C string literal (\", \\, CR, or LF).")
endif()

set(ZULU_CLIENT_KEY_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/ZuluClientKey.h.in"
    "${ZULU_CLIENT_KEY_GENERATED_DIR}/ZuluClientKey.h"
    @ONLY)
