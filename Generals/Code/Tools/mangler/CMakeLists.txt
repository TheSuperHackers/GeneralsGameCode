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

add_library(g_manglerlib STATIC)
set_target_properties(g_manglerlib PROPERTIES OUTPUT_NAME manglerlib)

target_sources(g_manglerlib PUBLIC ${MANGLERLIB_SRC})
target_include_directories(g_manglerlib PRIVATE . wlib wnet)
target_link_libraries(g_manglerlib PRIVATE wsock32)
target_link_libraries(g_manglerlib PUBLIC
    core_config
    core_utility
)

# mangler app

add_executable(g_mangler WIN32)
set_target_properties(g_mangler PROPERTIES OUTPUT_NAME mangler)

target_sources(g_mangler PRIVATE ${MANGLER_SRC})
target_include_directories(g_mangler PRIVATE . wlib wnet)
target_link_libraries(g_mangler PRIVATE g_manglerlib)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(g_mangler PRIVATE /subsystem:console)
endif()


# manglertest app

add_executable(g_manglertest WIN32)
set_target_properties(g_manglertest PROPERTIES OUTPUT_NAME manglertest)

target_sources(g_manglertest PRIVATE ${MANGLERTEST_SRC})
target_include_directories(g_manglertest PRIVATE . wlib wnet)
target_link_libraries(g_manglertest PRIVATE g_manglerlib)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(g_manglertest PRIVATE /subsystem:console)
endif()
