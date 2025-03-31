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
add_library(gz_wwdownload STATIC)
set_target_properties(gz_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_sources(gz_wwdownload PRIVATE ${WWDOWNLOAD_SRC})

target_link_libraries(gz_wwdownload PRIVATE
    gz_wwcommon
)
