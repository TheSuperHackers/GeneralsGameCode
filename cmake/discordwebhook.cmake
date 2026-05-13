# Bakes a Discord webhook URL into a generated header that the game engine
# includes. The Makefile pulls the secret out of GCP Secret Manager (one of
# debug_discord_webhook for `make installer`, discord_webhook for
# `make installer-release`) and exports it as ZULU_DISCORD_WEBHOOK_URL
# before invoking the docker build, mirroring how the radarvan client key
# is handled in zuluclientkey.cmake.
#
# Value resolution and the cache:
#  - When the env var is set and non-empty, FORCE-update the CMake cache.
#    A `make installer` run that fetches a fresh secret will replace the
#    previously cached value, and the configure_file step below will then
#    rewrite the generated header with the new URL.
#  - When the env var is unset (the typical case for any `--cmake` pass
#    that didn't fetch the secret — `docker-build-z_launcher`, an
#    in-build cmake regen, an IDE-driven reconfigure), the cached value
#    from the previous configure is reused. This is the bit that prevents
#    `DiscordWebhook.h` from silently resetting to "" mid-build when a
#    sibling recipe in the same shared build dir reconfigures without
#    the env var in scope.
#  - If neither the env nor the cache has a value, the generated header
#    falls through to an empty string. The engine code treats an empty
#    URL as "feature disabled" at runtime, so plain `cmake ..` dev builds
#    keep working.

if(DEFINED ENV{ZULU_DISCORD_WEBHOOK_URL} AND NOT "$ENV{ZULU_DISCORD_WEBHOOK_URL}" STREQUAL "")
    set(ZULU_DISCORD_WEBHOOK_URL "$ENV{ZULU_DISCORD_WEBHOOK_URL}" CACHE STRING
        "Discord webhook URL baked into LobbyDiscord.cpp at build time." FORCE)
endif()

# Make sure the variable is defined as a local for configure_file even when
# nothing has supplied a value yet (first dev `cmake ..` with no env, no
# cache).
if(NOT DEFINED ZULU_DISCORD_WEBHOOK_URL)
    set(ZULU_DISCORD_WEBHOOK_URL "")
endif()

string(STRIP "${ZULU_DISCORD_WEBHOOK_URL}" ZULU_DISCORD_WEBHOOK_URL)

# Reject any character that would break out of the C string literal in the
# generated header. Discord URLs are plain ASCII so quotes, backslashes,
# and CR/LF should never appear; if one does, abort rather than emit a
# subtly broken header.
if(ZULU_DISCORD_WEBHOOK_URL MATCHES "[\"\\\\\r\n]")
    message(FATAL_ERROR
        "ZULU_DISCORD_WEBHOOK_URL contains characters that cannot be embedded "
        "in a C string literal (\", \\, CR, or LF).")
endif()

set(ZULU_DISCORD_WEBHOOK_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/DiscordWebhook.h.in"
    "${ZULU_DISCORD_WEBHOOK_GENERATED_DIR}/DiscordWebhook.h"
    @ONLY)
