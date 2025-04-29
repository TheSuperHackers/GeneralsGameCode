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
add_library(z_wwdownload STATIC)
set_target_properties(z_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_sources(z_wwdownload PRIVATE ${WWDOWNLOAD_SRC})

target_link_libraries(z_wwdownload PRIVATE
    z_wwcommon
    zi_always
    zi_gameengine_include
)
