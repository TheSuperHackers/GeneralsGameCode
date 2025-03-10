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

add_executable(matchbot WIN32)
target_include_directories(matchbot PRIVATE . wlib wnet)
target_sources(matchbot PRIVATE ${MATCHBOT_SRC})

target_link_libraries(matchbot PRIVATE
    gamespy::gamespy
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(matchbot PRIVATE /subsystem:console)
endif()
