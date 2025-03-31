set(BABYLON_SRC
    "bin.cpp"
    "bin.h"
    "DlgProxy.cpp"
    "DlgProxy.h"
    "excel8.cpp"
    "excel8.h"
    "expimp.cpp"
    "expimp.h"
    "ExportDlg.cpp"
    "ExportDlg.h"
    "fileops.cpp"
    "fileops.h"
    "GenerateDlg.cpp"
    "GenerateDlg.h"
    "iff.cpp"
    "iff.h"
    "list.cpp"
    "list.h"
    "loadsave.cpp"
    "loadsave.h"
    "MatchDlg.cpp"
    "MatchDlg.h"
    "noxstring.cpp"
    "noxstring.h"
    "noxstringDlg.cpp"
    "noxstringDlg.h"
    "olestring.cpp"
    "olestring.h"
    "ProceedDlg.cpp"
    "ProceedDlg.h"
    "Report.cpp"
    "Report.h"
    "resource.h"
    "RetranslateDlg.cpp"
    "RetranslateDlg.h"
    "StdAfx.cpp"
    "StdAfx.h"
    "transcs.cpp"
    "transcs.h"
    "TransDB.cpp"
    "TransDB.h"
    "VerifyDlg.cpp"
    "VerifyDlg.h"
    "VerifyTextDlg.cpp"
    "VerifyTextDlg.h"
    "ViewDBsDlg.cpp"
    #"VIEWDBSII.cpp"
    "VIEWDBSII.h"
    "XLStuff.cpp"
    "XLStuff.h"
)

add_executable(g_babylon WIN32)
set_target_properties(g_babylon PROPERTIES OUTPUT_NAME noxstring)

target_sources(g_babylon PRIVATE ${BABYLON_SRC})

target_link_libraries(g_babylon PRIVATE
    gz_config
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(g_babylon PRIVATE _AFXDLL)
    target_sources(g_babylon PRIVATE noxstring.rc)
endif()
