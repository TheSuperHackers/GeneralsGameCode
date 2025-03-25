set(MATCHBOT_SRC
    "debug.cpp"
    "debug.h"
    "encrypt.cpp"
    "encrypt.h"
    "generals.cpp"
    "generals.h"
    "global.cpp"
    "global.h"
    "main.cpp"
    "matcher.cpp"
    "matcher.h"
    "mydebug.cpp"
    "mydebug.h"
    "rand.cpp"
    "rand.h"
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

add_executable(z_matchbot WIN32)
set_target_properties(z_matchbot PROPERTIES OUTPUT_NAME matchbot)

target_sources(z_matchbot PRIVATE ${MATCHBOT_SRC})

target_include_directories(z_matchbot PRIVATE . wlib wnet)

target_link_libraries(z_matchbot PRIVATE
    gamespy::gamespy
    gz_utility
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_matchbot PRIVATE /subsystem:console)
endif()
