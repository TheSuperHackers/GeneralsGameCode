; Zulu installer for Command & Conquer: Generals Zero Hour
;
; Build with:  makensis installer/Zulu.nsi
;          or: make installer   (drives this script with staged inputs)
; Output:      installer/Zulu_Setup.exe
;
; Inputs default to ../generalszh_zulu.exe and ../Zulu.big (relative to this
; script). The Makefile overrides them with /D to point at the staged copies
; under build/installer-tmp/ so the repo root stays clean.
;
; Layout produced on the target machine:
;   <user-chosen install dir>\generalszh_zulu.exe
;   <user-chosen install dir>\Uninstall_Zulu.exe
;   %USERPROFILE%\Documents\Command and Conquer Generals Zero Hour Data\Zulu.big
;   Desktop and Start Menu shortcuts that launch:
;     <install dir>\generalszh_zulu.exe -quickstart -win -mod Zulu.big

!define APPNAME       "Zulu"
!define APPVERSION    "1.0"
!define EXENAME       "generalszh_zulu.exe"
!define BIGNAME       "Zulu.big"
!define USERDATALEAF  "Command and Conquer Generals Zero Hour Data"
!define LAUNCHARGS    "-mod Zulu.big"
!define UNINSTREGKEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"

; Source paths for the files that get packed into the installer. The
; Makefile passes /D overrides; the defaults preserve the historical
; "binaries sit at the repo root" workflow.
!ifndef EXE_SOURCE
    !define EXE_SOURCE "..\${EXENAME}"
!endif
!ifndef BIG_SOURCE
    !define BIG_SOURCE "..\${BIGNAME}"
!endif

Name        "${APPNAME}"
OutFile     "Zulu_Setup.exe"
Unicode     true
SetCompressor /SOLID lzma

; Default install dir: try the registry key the retail installer wrote, then
; fall back to a guess. The user can change this on the Directory page; the
; intent is for them to point it at their existing Zero Hour folder so
; generalszh_zulu.exe sits next to the original generals.exe / generalszh.exe.
InstallDirRegKey HKLM "Software\Electronic Arts\EA Games\Command and Conquer Generals Zero Hour" "InstallPath"
InstallDir       "$PROGRAMFILES\EA Games\Command and Conquer Generals Zero Hour"

; Writing to Program Files needs admin; the wizard will trigger UAC.
RequestExecutionLevel admin

!include "MUI2.nsh"

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Install ${APPNAME}" SecInstall
    SectionIn RO

    ; --- Game executable -> user-chosen install dir ----------------------
    ; /oname forces the basename inside the installer to ${EXENAME} regardless
    ; of how the staged input is named on disk.
    SetOutPath "$INSTDIR"
    File "/oname=${EXENAME}" "${EXE_SOURCE}"

    ; --- Mod BIG -> user data dir ---------------------------------------
    ; $DOCUMENTS resolves to the invoking user's Documents folder. With UAC
    ; elevation via the consent prompt this is still the original user.
    SetOutPath "$DOCUMENTS\${USERDATALEAF}"
    File "/oname=${BIGNAME}" "${BIG_SOURCE}"

    ; --- Shortcuts -------------------------------------------------------
    ; CreateShortcut: target, args, icon-file, icon-index, start-options
    ; The "start in" directory defaults to the target's directory, which is
    ; what we want — Zero Hour expects to find its own data files relative
    ; to the .exe.
    CreateShortcut "$DESKTOP\${APPNAME}.lnk" \
        "$INSTDIR\${EXENAME}" \
        "${LAUNCHARGS}" \
        "$INSTDIR\${EXENAME}" 0

    CreateDirectory "$SMPROGRAMS\${APPNAME}"
    CreateShortcut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" \
        "$INSTDIR\${EXENAME}" \
        "${LAUNCHARGS}" \
        "$INSTDIR\${EXENAME}" 0
    CreateShortcut "$SMPROGRAMS\${APPNAME}\Uninstall ${APPNAME}.lnk" \
        "$INSTDIR\Uninstall_Zulu.exe"

    ; --- Uninstaller + Add/Remove Programs registration ------------------
    WriteUninstaller "$INSTDIR\Uninstall_Zulu.exe"
    WriteRegStr HKLM "${UNINSTREGKEY}" "DisplayName"     "${APPNAME}"
    WriteRegStr HKLM "${UNINSTREGKEY}" "DisplayVersion"  "${APPVERSION}"
    WriteRegStr HKLM "${UNINSTREGKEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "${UNINSTREGKEY}" "UninstallString" '"$INSTDIR\Uninstall_Zulu.exe"'
    WriteRegStr HKLM "${UNINSTREGKEY}" "Publisher"       "Bill Rich"
    WriteRegDWORD HKLM "${UNINSTREGKEY}" "NoModify" 1
    WriteRegDWORD HKLM "${UNINSTREGKEY}" "NoRepair" 1
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\${EXENAME}"
    Delete "$INSTDIR\Uninstall_Zulu.exe"
    Delete "$DOCUMENTS\${USERDATALEAF}\${BIGNAME}"

    Delete "$DESKTOP\${APPNAME}.lnk"
    Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"
    Delete "$SMPROGRAMS\${APPNAME}\Uninstall ${APPNAME}.lnk"
    RMDir  "$SMPROGRAMS\${APPNAME}"

    DeleteRegKey HKLM "${UNINSTREGKEY}"

    ; Don't RMDir $INSTDIR — that's the user's Zero Hour folder and we
    ; share it with the retail install. Leave it alone.
SectionEnd
