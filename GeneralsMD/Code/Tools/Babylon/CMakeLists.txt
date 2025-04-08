set(BABYLON_SRC
    "Babylon.cpp"
    "Babylon.h"
    "BabylonDlg.cpp"
    "BabylonDlg.h"
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

add_executable(z_babylon WIN32)
set_target_properties(z_babylon PROPERTIES OUTPUT_NAME babylon)

target_sources(z_babylon PRIVATE ${BABYLON_SRC})

target_link_libraries(z_babylon PRIVATE
    core_config
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(z_babylon PRIVATE _AFXDLL)
    target_sources(z_babylon PRIVATE Babylon.rc)
endif()
