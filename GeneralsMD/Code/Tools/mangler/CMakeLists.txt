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

add_library(manglerlib STATIC)
target_include_directories(manglerlib PRIVATE . wlib wnet)
target_sources(manglerlib PUBLIC ${MANGLERLIB_SRC})
target_link_libraries(manglerlib PRIVATE wsock32)

# mangler

add_executable(mangler WIN32)
target_include_directories(mangler PRIVATE . wlib wnet)
target_sources(mangler PRIVATE ${MANGLER_SRC})
target_link_libraries(mangler PRIVATE manglerlib)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(mangler PRIVATE /subsystem:console)
endif()


# manglertest

add_executable(manglertest WIN32)
target_include_directories(manglertest PRIVATE . wlib wnet)
target_sources(manglertest PRIVATE ${MANGLERTEST_SRC})
target_link_libraries(manglertest PRIVATE manglerlib)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(manglertest PRIVATE /subsystem:console)
endif()
