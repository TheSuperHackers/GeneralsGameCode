# Bakes a Discord webhook URL into a generated header that the game engine
# includes. The Makefile pulls the secret out of GCP Secret Manager (one of
# debug_discord_webhook for `make installer`, discord_webhook for
# `make installer-release`) and exports it as ZULU_DISCORD_WEBHOOK_URL
# before invoking the docker build, mirroring how the radarvan client key
# is handled in zuluclientkey.cmake.
#
# Configure does NOT fail when the variable is unset so dev builds (raw
# `cmake ..` without an installer pipeline) keep working; the engine code
# treats an empty URL as "feature disabled" at runtime. The Makefile is
# what enforces "build fails if the secret is missing" by aborting the
# `gcloud secrets versions access` step before docker is even invoked.

if(NOT DEFINED ZULU_DISCORD_WEBHOOK_URL OR ZULU_DISCORD_WEBHOOK_URL STREQUAL "")
    if(DEFINED ENV{ZULU_DISCORD_WEBHOOK_URL} AND NOT "$ENV{ZULU_DISCORD_WEBHOOK_URL}" STREQUAL "")
        set(ZULU_DISCORD_WEBHOOK_URL "$ENV{ZULU_DISCORD_WEBHOOK_URL}")
    endif()
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
