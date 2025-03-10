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
add_library(wwdownload STATIC)

target_link_libraries(wwdownload PRIVATE
    wwcommon
)

target_sources(wwdownload PRIVATE ${WWDOWNLOAD_SRC})
