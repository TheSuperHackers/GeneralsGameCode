set(MANGLERLIB_SRC
    "crc.cpp"
    "crc.h"
    "endian.h"
    "wlib/arraylist.h"
    "wlib/configfile.cpp"
    "wlib/configfile.h"
    "wlib/critsec.cpp"
    "wlib/critsec.h"
    "wlib/dictionary.h"
    "wlib/filed.h"
    "wlib/linkedlist.h"
    "wlib/mboxd.h"
    "wlib/monod.cpp"
    "wlib/monod.h"
    "wlib/odevice.h"
    "wlib/sem4.cpp"
    "wlib/sem4.h"
    "wlib/stderrd.h"
    "wlib/stdoutd.h"
    "wlib/streamer.cpp"
    "wlib/streamer.h"
    "wlib/syslogd.cpp"
    "wlib/syslogd.h"
    "wlib/threadfac.cpp"
    "wlib/threadfac.h"
    "wlib/threadsafe.h"
    "wlib/timezone.cpp"
    "wlib/timezone.h"
    "wlib/ustring.h"
    "wlib/wdebug.cpp"
    "wlib/wdebug.h"
    "wlib/wstring.cpp"
    "wlib/wstring.h"
    "wlib/wstypes.h"
    "wlib/wtime.cpp"
    "wlib/wtime.h"
    "wlib/xtime.cpp"
    "wlib/xtime.h"
    "wnet/field.cpp"
    "wnet/field.h"
    "wnet/packet.cpp"
    "wnet/packet.h"
    "wnet/tcp.cpp"
    "wnet/tcp.h"
    "wnet/udp.cpp"
    "wnet/udp.h"
)

set(MANGLER_SRC
    "mangler.cpp"
    "mangler.h"
)

set(MANGLERTEST_SRC
    "manglertest.cpp"
)

add_library(z_manglerlib STATIC)
set_target_properties(z_manglerlib PROPERTIES OUTPUT_NAME manglerlib)

target_sources(z_manglerlib PUBLIC ${MANGLERLIB_SRC})
target_include_directories(z_manglerlib PRIVATE . wlib wnet)
target_link_libraries(z_manglerlib PRIVATE wsock32)


# mangler app

add_executable(z_mangler WIN32)
set_target_properties(z_mangler PROPERTIES OUTPUT_NAME mangler)

target_sources(z_mangler PRIVATE ${MANGLER_SRC})
target_include_directories(z_mangler PRIVATE . wlib wnet)
target_link_libraries(z_mangler PRIVATE z_manglerlib)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_mangler PRIVATE /subsystem:console)
endif()


# manglertest app

add_executable(z_manglertest WIN32)
set_target_properties(z_manglertest PROPERTIES OUTPUT_NAME manglertest)

target_sources(z_manglertest PRIVATE ${MANGLERTEST_SRC})
target_include_directories(z_manglertest PRIVATE . wlib wnet)
target_link_libraries(z_manglertest PRIVATE z_manglerlib)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_manglertest PRIVATE /subsystem:console)
endif()
