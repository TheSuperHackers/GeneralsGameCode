# Set source files
set(WWDOWNLOAD_SRC
    Download.cpp
    Download.h
    DownloadDebug.h
    downloaddefs.h
    FTP.CPP
    ftp.h
    ftpdefs.h
    registry.cpp
    Registry.h
    urlBuilder.cpp
    urlBuilder.h
)

# Targets to build.
add_library(g_wwdownload STATIC)
set_target_properties(g_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_sources(g_wwdownload PRIVATE ${WWDOWNLOAD_SRC})

target_link_libraries(g_wwdownload PRIVATE
    g_wwcommon
    gi_gameengine_include
)
