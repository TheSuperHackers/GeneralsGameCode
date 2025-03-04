# Set source files
set(WWDOWNLOAD_SRC
    Download.cpp
    Ftp.cpp
    registry.cpp
    urlBuilder.cpp
    Download.h
    DownloadDebug.h
    DownloadDefs.h
    Ftp.h
    FtpDefs.h
    Registry.h
    urlBuilder.h
)

# Targets to build.
add_library(g_wwdownload STATIC)
set_target_properties(g_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_sources(g_wwdownload PRIVATE ${WWDOWNLOAD_SRC})

target_link_libraries(g_wwdownload PRIVATE
    g_wwcommon
)
