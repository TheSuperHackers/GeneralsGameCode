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
add_library(wwdownloadg STATIC)

target_link_libraries(wwdownloadg PRIVATE
    wwcommong
)

target_sources(wwdownloadg PRIVATE ${WWDOWNLOAD_SRC})
