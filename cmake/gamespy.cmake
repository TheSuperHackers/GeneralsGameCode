set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheAssemblyArmada/GamespySDK.git
    GIT_TAG        d7ec6d4fea1c11fc37173ea954fc1ec47202a931
)

FetchContent_MakeAvailable(gamespy)
