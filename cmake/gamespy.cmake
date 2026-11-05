set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net" CACHE STRING "Host name or IPv4 address of the GameSpy server. Override in Options.ini.")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheSuperHackers/GamespySDK.git
    GIT_TAG        07e3d15c500415abc281efb74322ab6d9c857eb8
)

FetchContent_MakeAvailable(gamespy)

# Override GameSpy SDK peerchat connect. Each program that uses gspeer must define GameSpyChatConnectSecure.
target_compile_definitions(gspeer PRIVATE chatConnectSecureA=GameSpyChatConnectSecure)

target_compile_definitions(core_config INTERFACE RTS_GAMESPY_SERVER_NAME="${GAMESPY_SERVER_NAME}")
