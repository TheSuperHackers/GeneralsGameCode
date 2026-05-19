; ==============================================================================
;  Hotkey-spam desync repro driver - debug build
; ==============================================================================
;  AutoHotkey v1.1.x.
;
;  Controls (try in this order to diagnose):
;     F6      - SINGLE-SHOT TEST: send one Ctrl+1, then beep. Use this
;               in-game to confirm AutoHotkey can deliver keystrokes to the
;               game window at all. If F6 doesn't create a control group,
;               the toggle keys won't either.
;     F7      - diagnostics popup (active window title, spam state, cycle
;               count)
;     Ctrl+F8 - toggle continuous spam on/off (was just F8, but GenTool's
;               d3d8 proxy hooks F8 below AHK level on some setups, which
;               eats the keystroke before AHK sees it - Ctrl+F8 dodges
;               the conflict)
;     Ctrl+F9 - toggle alternate ControlSend path (sends to the window
;               handle directly, bypassing focus). Useful if SendInput is
;               being blocked.
;     F12     - quit
; ==============================================================================

#NoEnv
#SingleInstance Force

SetTitleMatchMode, 2
SendMode Input
SetKeyDelay, 30, 30           ; bigger inter-press delay - DX games sometimes
                              ; drop keystrokes that come too fast
SetBatchLines, -1

global kGameWindowMatch := "Generals"
global spamEnabled := false
global cycleCount  := 0
global useControlSend := false
global lastSendResult := "(none yet)"

Menu, Tray, Tip, % "Desync Repro - F6 single-shot - Ctrl+F8 toggle spam"
TrayTip, % "Desync Repro", % "Loaded - F6 single - F7 diag - Ctrl+F8 spam", 3
MsgBox, 64, % "Desync Repro", % "Loaded.`n`nFirst test: switch to the game and press F6. It sends one Ctrl+1 to the game and beeps when done. If a control group forms, you're set.`n`nF6 = single shot`nF7 = diagnostics`nCtrl+F8 = toggle spam`nCtrl+F9 = toggle ControlSend mode`nF12 = quit`n`nNote: plain F8 is grabbed by GenTool on some setups, so the toggle is Ctrl+F8."
return

; ---- F6: single shot test send ------------------------------------------
F6::
    WinGetTitle, fgTitle, A
    if (!InStr(fgTitle, kGameWindowMatch)) {
        lastSendResult := "Refused - window title was: " . fgTitle
        SoundBeep, 400, 200
        return
    }
    if (useControlSend) {
        WinGet, hwnd, ID, A
        ControlSend,, ^1, % "ahk_id " . hwnd
        lastSendResult := "ControlSend ^1 to hwnd " . hwnd
    } else {
        Send, ^1
        lastSendResult := "Send ^1 via SendInput"
    }
    Sleep, 100
    SoundBeep, 800, 150
return

; ---- F7: diagnostics ----------------------------------------------------
F7::
    WinGetTitle, activeTitle, A
    WinGet, activeHwnd, ID, A
    isMatch := InStr(activeTitle, kGameWindowMatch) ? "YES" : "no"
    modeStr := useControlSend ? "ControlSend (window-direct)" : "SendInput (global)"
    MsgBox, 64, % "Desync Repro Diagnostics", % "Spam: " . spamEnabled . "`nCycles run: " . cycleCount . "`nSend mode: " . modeStr . "`nLast send: " . lastSendResult . "`n`nActive window title:`n" . activeTitle . "`nActive hwnd: " . activeHwnd . "`n`nMatches """ . kGameWindowMatch . """: " . isMatch
return

; ---- Ctrl+F8: toggle spam -----------------------------------------------
; ^ = Ctrl in AHK hotkey syntax. We use Ctrl+F8 instead of plain F8 because
; GenTool's d3d8.dll proxy grabs F8 below AHK's keyboard hook on some
; setups, swallowing the keystroke before AHK ever sees it.
^F8::
    spamEnabled := !spamEnabled
    if (spamEnabled) {
        cycleCount := 0
        SetTimer, SpamCycle, 150
        SoundBeep, 1000, 150
        ToolTip, % "SPAM ON - cycles will count up (press F7 to confirm)", 100, 100
        SetTimer, ClearToolTip, -2000
    } else {
        SetTimer, SpamCycle, Off
        SoundBeep, 500, 150
        ToolTip, % "SPAM OFF - cycles run: " . cycleCount, 100, 100
        SetTimer, ClearToolTip, -2500
    }
return

; ---- Ctrl+F9: toggle SendInput vs ControlSend --------------------------
^F9::
    useControlSend := !useControlSend
    modeStr := useControlSend ? "ControlSend (window-direct)" : "SendInput (global)"
    SoundBeep, 700, 100
    ToolTip, % "Mode: " . modeStr, 100, 100
    SetTimer, ClearToolTip, -2000
return

; ---- F12: quit ----------------------------------------------------------
F12::
    SetTimer, SpamCycle, Off
    ExitApp
return

; ---- spam body ----------------------------------------------------------
SpamCycle:
    if (!spamEnabled)
        return

    WinGetTitle, fgTitle, A
    if (!InStr(fgTitle, kGameWindowMatch))
        return

    cycleCount += 1

    if (useControlSend) {
        WinGet, hwnd, ID, A
        ControlSend,, ^1, % "ahk_id " . hwnd
        Sleep, 50
        ControlSend,, 1, % "ahk_id " . hwnd
        Sleep, 30
        ControlSend,, ^2, % "ahk_id " . hwnd
        Sleep, 50
        ControlSend,, 2, % "ahk_id " . hwnd
        Sleep, 30
    } else {
        Send, ^1
        Sleep, 50
        Send, 1
        Sleep, 30
        Send, ^2
        Sleep, 50
        Send, 2
        Sleep, 30
    }
return

ClearToolTip:
    ToolTip
return
