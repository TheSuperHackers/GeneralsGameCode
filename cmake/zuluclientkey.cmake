# Reads the build-time radarvan auth key from ${CMAKE_SOURCE_DIR}/ZULUCLIENT_KEY
# (gitignored) and writes it into a generated header that the game engine
# includes. If the file is missing the key is empty and the runtime simply
# skips the Authorization header.

set(ZULU_CLIENT_KEY_FILE "${CMAKE_SOURCE_DIR}/ZULUCLIENT_KEY")

if(EXISTS "${ZULU_CLIENT_KEY_FILE}")
    file(READ "${ZULU_CLIENT_KEY_FILE}" ZULU_CLIENT_KEY_RAW)
    string(STRIP "${ZULU_CLIENT_KEY_RAW}" ZULU_CLIENT_KEY)
    # Re-run CMake configure when the key file changes so the generated
    # header is rewritten and dependent translation units recompile.
    set_property(DIRECTORY "${CMAKE_SOURCE_DIR}"
        APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ZULU_CLIENT_KEY_FILE}")
else()
    set(ZULU_CLIENT_KEY "")
    message(STATUS "ZULUCLIENT_KEY not found at ${ZULU_CLIENT_KEY_FILE}; "
                   "radarvan requests will be sent without Authorization.")
endif()

# Reject any character that would break out of the C string literal in the
# generated header. The auth token format is up to the user but quotes,
# backslashes, and CR/LF are not safe.
if(ZULU_CLIENT_KEY MATCHES "[\"\\\\\r\n]")
    message(FATAL_ERROR
        "ZULUCLIENT_KEY contains characters that cannot be embedded in a "
        "C string literal (\", \\, CR, or LF).")
endif()

set(ZULU_CLIENT_KEY_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/ZuluClientKey.h.in"
    "${ZULU_CLIENT_KEY_GENERATED_DIR}/ZuluClientKey.h"
    @ONLY)
