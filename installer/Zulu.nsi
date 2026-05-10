; Zulu installer for Command & Conquer: Generals Zero Hour
;
; Build with:  makensis installer/Zulu.nsi
;          or: make installer   (drives this script with staged inputs)
; Output:      installer/Zulu_Setup.exe
;
; Inputs default to ../generalszh_zulu.exe, ../ZuluLauncher.exe and ../Zulu.big
; (relative to this script). The Makefile overrides them with /D to point at
; the staged copies under build/installer-tmp/ so the repo root stays clean.
;
; Layout produced on the target machine:
;   <user-chosen install dir>\generalszh_zulu.exe
;   <user-chosen install dir>\ZuluLauncher.exe
;   <user-chosen install dir>\Uninstall_Zulu.exe
;   %USERPROFILE%\Documents\Command and Conquer Generals Zero Hour Data\Zulu.big
;   Desktop and Start Menu shortcuts that launch:
;     <install dir>\ZuluLauncher.exe -mod Zulu.big
;   The launcher fetches https://storage.googleapis.com/zulu-installer/latest.json
;   on every start, downloads a new installer if version > installed, runs it
;   with /S /D=<install dir>, then exits. The installer's Section calls the
;   launcher again at the end when run silently, so an update completes with
;   one UAC click and no extra shortcut clicks.

!define APPNAME       "Zulu"
!define APPVERSION    "1.1.1"
!define EXENAME       "generalszh_zulu.exe"
!define LAUNCHERNAME  "ZuluLauncher.exe"
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
!ifndef LAUNCHER_SOURCE
    !define LAUNCHER_SOURCE "..\${LAUNCHERNAME}"
!endif
!ifndef BIG_SOURCE
    !define BIG_SOURCE "..\${BIGNAME}"
!endif

Name        "${APPNAME}"
OutFile     "Zulu_Setup.exe"
Unicode     true
SetCompressor /SOLID lzma

; Default install dir: prefer the location our own uninstaller wrote (so
; silent update-installs from the launcher land in the right place), then
; the retail EA Games registry key, then a guess. The user can change this
; on the Directory page; for `/S` (silent) installs from the launcher the
; correct path is forwarded via `/D=` anyway.
InstallDirRegKey HKLM "${UNINSTREGKEY}" "InstallLocation"
InstallDir       "$PROGRAMFILES\EA Games\Command and Conquer Generals Zero Hour"

; Writing to Program Files needs admin; the wizard will trigger UAC.
RequestExecutionLevel admin

!include "MUI2.nsh"

!define MUI_ABORTWARNING

; First-time interactive install: offer to launch Zulu when the user clicks
; Finish. For silent /S installs (triggered by the launcher's update flow)
; the finish page is skipped entirely; see the Exec at the end of Section
; Install which handles that case.
!define MUI_FINISHPAGE_RUN "$INSTDIR\${LAUNCHERNAME}"
!define MUI_FINISHPAGE_RUN_PARAMETERS "${LAUNCHARGS}"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${APPNAME}"

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

    ; --- Game executable + launcher -> user-chosen install dir ----------
    ; /oname forces the basename inside the installer regardless of how
    ; the staged input is named on disk.
    SetOutPath "$INSTDIR"
    File "/oname=${EXENAME}" "${EXE_SOURCE}"
    File "/oname=${LAUNCHERNAME}" "${LAUNCHER_SOURCE}"

    ; --- Mod BIG -> user data dir ---------------------------------------
    ; $DOCUMENTS resolves to the invoking user's Documents folder. With UAC
    ; elevation via the consent prompt this is still the original user.
    SetOutPath "$DOCUMENTS\${USERDATALEAF}"
    File "/oname=${BIGNAME}" "${BIG_SOURCE}"

    ; --- Shortcuts -------------------------------------------------------
    ; Targets the launcher, not the game directly, so every cold start
    ; gets an update check. The launcher forwards the same args to the
    ; game when no update is pending.
    CreateShortcut "$DESKTOP\${APPNAME}.lnk" \
        "$INSTDIR\${LAUNCHERNAME}" \
        "${LAUNCHARGS}" \
        "$INSTDIR\${LAUNCHERNAME}" 0

    CreateDirectory "$SMPROGRAMS\${APPNAME}"
    CreateShortcut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" \
        "$INSTDIR\${LAUNCHERNAME}" \
        "${LAUNCHARGS}" \
        "$INSTDIR\${LAUNCHERNAME}" 0
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

    ; Silent invocations come from the launcher's update flow. The
    ; *calling* launcher (still running while we install) waits for us
    ; to exit and then re-launches the game with the user's original
    ; argv — so any extras like -zulu_debug carry through. We
    ; intentionally do NOT Exec the launcher here, since that would
    ; only know about the shortcut's hardcoded LAUNCHARGS and would
    ; race the launcher's own relaunch.
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\${EXENAME}"
    Delete "$INSTDIR\${LAUNCHERNAME}"
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
