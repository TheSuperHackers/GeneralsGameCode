set(WOLSETUP_SRC
    "resource.h"
    "StdAfx.cpp"
    "StdAfx.h"
    "verchk.cpp"
    "verchk.h"
    "wolInit.cpp"
    "wolSetup.cpp"
    "wolSetup.h"
    "WOLAPI/chatdefs.h"
    "WOLAPI/downloaddefs.h"
    "WOLAPI/ftpdefs.h"
    "WOLAPI/netutildefs.h"
    "WOLAPI/wolapi.h"
    "WOLAPI/wolapi_i.c"
)

add_executable(wolsetup WIN32)

target_sources(wolsetup PRIVATE ${WOLSETUP_SRC})

target_link_libraries(wolsetup PRIVATE
    Version
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(wolsetup PRIVATE wolSetup.rc)
endif()

# TODO: Copy the dll's that reside in the WOLAPI folder.
