/* macOS shim: dinput.h — DirectInput stubs */
#pragma once
#include "windows.h"

/* Key scan codes (matching Windows DirectInput values) */
#define DIK_ESCAPE 0x01
#define DIK_1 0x02
#define DIK_2 0x03
#define DIK_3 0x04
#define DIK_4 0x05
#define DIK_5 0x06
#define DIK_6 0x07
#define DIK_7 0x08
#define DIK_8 0x09
#define DIK_9 0x0A
#define DIK_0 0x0B
#define DIK_MINUS 0x0C
#define DIK_EQUALS 0x0D
#define DIK_BACK 0x0E
#define DIK_TAB 0x0F
#define DIK_Q 0x10
#define DIK_W 0x11
#define DIK_E 0x12
#define DIK_R 0x13
#define DIK_T 0x14
#define DIK_Y 0x15
#define DIK_U 0x16
#define DIK_I 0x17
#define DIK_O 0x18
#define DIK_P 0x19
#define DIK_LBRACKET 0x1A
#define DIK_RBRACKET 0x1B
#define DIK_RETURN 0x1C
#define DIK_LCONTROL 0x1D
#define DIK_A 0x1E
#define DIK_S 0x1F
#define DIK_D 0x20
#define DIK_F 0x21
#define DIK_G 0x22
#define DIK_H 0x23
#define DIK_J 0x24
#define DIK_K 0x25
#define DIK_L 0x26
#define DIK_SEMICOLON 0x27
#define DIK_APOSTROPHE 0x28
#define DIK_GRAVE 0x29
#define DIK_LSHIFT 0x2A
#define DIK_BACKSLASH 0x2B
#define DIK_Z 0x2C
#define DIK_X 0x2D
#define DIK_C 0x2E
#define DIK_V 0x2F
#define DIK_B 0x30
#define DIK_N 0x31
#define DIK_M 0x32
#define DIK_COMMA 0x33
#define DIK_PERIOD 0x34
#define DIK_SLASH 0x35
#define DIK_RSHIFT 0x36
#define DIK_MULTIPLY 0x37
#define DIK_LMENU 0x38
#define DIK_SPACE 0x39
#define DIK_CAPITAL 0x3A
#define DIK_F1 0x3B
#define DIK_F2 0x3C
#define DIK_F3 0x3D
#define DIK_F4 0x3E
#define DIK_F5 0x3F
#define DIK_F6 0x40
#define DIK_F7 0x41
#define DIK_F8 0x42
#define DIK_F9 0x43
#define DIK_F10 0x44
#define DIK_NUMLOCK 0x45
#define DIK_SCROLL 0x46
#define DIK_NUMPAD7 0x47
#define DIK_NUMPAD8 0x48
#define DIK_NUMPAD9 0x49
#define DIK_SUBTRACT 0x4A
#define DIK_NUMPAD4 0x4B
#define DIK_NUMPAD5 0x4C
#define DIK_NUMPAD6 0x4D
#define DIK_ADD 0x4E
#define DIK_NUMPAD1 0x4F
#define DIK_NUMPAD2 0x50
#define DIK_NUMPAD3 0x51
#define DIK_NUMPAD0 0x52
#define DIK_DECIMAL 0x53
#define DIK_F11 0x57
#define DIK_F12 0x58
#define DIK_NUMPADENTER 0x9C
#define DIK_RCONTROL 0x9D
#define DIK_DIVIDE 0xB5
#define DIK_SYSRQ 0xB7
#define DIK_RMENU 0xB8
#define DIK_PAUSE 0xC5
#define DIK_HOME 0xC7
#define DIK_UP 0xC8
#define DIK_PRIOR 0xC9
#define DIK_LEFT 0xCB
#define DIK_RIGHT 0xCD
#define DIK_END 0xCF
#define DIK_DOWN 0xD0
#define DIK_NEXT 0xD1
#define DIK_INSERT 0xD2
#define DIK_DELETE 0xD3
#define DIK_LWIN 0xDB
#define DIK_RWIN 0xDC
#define DIK_APPS 0xDD
#define DIK_NUMPADEQUALS 0x8D
#define DIK_PREVTRACK 0x90
#define DIK_NEXTTRACK 0x99
#define DIK_MUTE 0xA0
#define DIK_PLAYPAUSE 0xA2
#define DIK_MEDIASTOP 0xA4
#define DIK_VOLUMEDOWN 0xAE
#define DIK_VOLUMEUP 0xB0

/* Alternate names used by some code */
#define DIK_NUMPADPERIOD DIK_DECIMAL
#define DIK_NUMPADSTAR DIK_MULTIPLY
#define DIK_NUMPADMINUS DIK_SUBTRACT
#define DIK_NUMPADPLUS DIK_ADD
#define DIK_NUMPADSLASH DIK_DIVIDE
#define DIK_CAPSLOCK DIK_CAPITAL
#define DIK_LALT DIK_LMENU
#define DIK_RALT DIK_RMENU
#define DIK_UPARROW DIK_UP
#define DIK_DOWNARROW DIK_DOWN
#define DIK_LEFTARROW DIK_LEFT
#define DIK_RIGHTARROW DIK_RIGHT
#define DIK_PGUP DIK_PRIOR
#define DIK_PGDN DIK_NEXT

/* ── DirectInput error codes ─────────────────────────────────────────── */
#define DI_OK                         S_OK
#define DIERR_ACQUIRED                ((HRESULT)0x800704BBL)
#define DIERR_ALREADYINITIALIZED      ((HRESULT)0x800704B0L)
#define DIERR_BADDRIVERVER            ((HRESULT)0x80070077L)
#define DIERR_BETADIRECTINPUTVERSION  ((HRESULT)0x80040154L)
#define DIERR_DEVICEFULL              ((HRESULT)0x80040203L)
#define DIERR_DEVICENOTREG            ((HRESULT)0x80040154L)
#define DIERR_EFFECTPLAYING           ((HRESULT)0x80040208L)
#define DIERR_GENERIC                 E_FAIL
#define DIERR_HANDLEEXISTS            ((HRESULT)0x80070005L)
#define DIERR_HASEFFECTS              ((HRESULT)0x80040209L)
#define DIERR_INCOMPLETEEFFECT        ((HRESULT)0x80040206L)
#define DIERR_INPUTLOST               ((HRESULT)0x8007001EL)
#define DIERR_INVALIDPARAM            E_INVALIDARG
#define DIERR_MAPFILEFAIL             ((HRESULT)0x8004020BL)
#define DIERR_MOREDATA                ((HRESULT)0x80040207L)
#define DIERR_NOAGGREGATION           ((HRESULT)0x80040110L)
#define DIERR_NOINTERFACE             E_NOINTERFACE
#define DIERR_NOTACQUIRED             ((HRESULT)0x8007000CL)
#define DIERR_NOTBUFFERED             ((HRESULT)0x80040204L)
#define DIERR_NOTDOWNLOADED           ((HRESULT)0x80040205L)
#define DIERR_NOTEXCLUSIVEACQUIRED    ((HRESULT)0x80040207L)
#define DIERR_NOTFOUND                ((HRESULT)0x80070002L)
#define DIERR_NOTINITIALIZED          ((HRESULT)0x80070015L)
#define DIERR_OBJECTNOTFOUND          ((HRESULT)0x80070002L)
#define DIERR_OLDDIRECTINPUTVERSION   ((HRESULT)0x80040154L)
#define DIERR_OTHERAPPHASPRIO         ((HRESULT)0x80040003L)
#define DIERR_OUTOFMEMORY             E_OUTOFMEMORY
#define DIERR_READONLY                ((HRESULT)0x80070005L)
#define DIERR_REPORTFULL              ((HRESULT)0x80040204L)
#define DIERR_UNPLUGGED               ((HRESULT)0x80040209L)
#define DIERR_UNSUPPORTED             E_NOTIMPL

/* ── DirectInput version ────────────────────────────────────────────── */
#define DIRECTINPUT_VERSION 0x0800

/* ── Cooperative level flags ────────────────────────────────────────── */
#define DISCL_EXCLUSIVE   0x00000001
#define DISCL_NONEXCLUSIVE 0x00000002
#define DISCL_FOREGROUND  0x00000004
#define DISCL_BACKGROUND  0x00000008
#define DISCL_NOWINKEY    0x00000010

/* ── Property constants ─────────────────────────────────────────────── */
#define DIPH_DEVICE       0
#define DIPH_BYOFFSET     1

/* ── DIDEVICEOBJECTDATA ─────────────────────────────────────────────── */
typedef struct _DIDEVICEOBJECTDATA {
  DWORD dwOfs;
  DWORD dwData;
  DWORD dwTimeStamp;
  DWORD dwSequence;
  UINT_PTR uAppData;
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;

/* ── DIPROPHEADER / DIPROPDWORD ─────────────────────────────────────── */
typedef struct _DIPROPHEADER {
  DWORD dwSize;
  DWORD dwHeaderSize;
  DWORD dwObj;
  DWORD dwHow;
} DIPROPHEADER, *LPDIPROPHEADER;

typedef struct _DIPROPDWORD {
  DIPROPHEADER diph;
  DWORD dwData;
} DIPROPDWORD, *LPDIPROPDWORD;

/* Property GUIDs (as pointers — stub) */
#define DIPROP_BUFFERSIZE ((const DIPROPHEADER*)1)

/* ── Data format structures ─────────────────────────────────────────── */
typedef struct _DIOBJECTDATAFORMAT {
  const GUID *pguid;
  DWORD dwOfs;
  DWORD dwType;
  DWORD dwFlags;
} DIOBJECTDATAFORMAT, *LPDIOBJECTDATAFORMAT;

typedef struct _DIDATAFORMAT {
  DWORD dwSize;
  DWORD dwObjSize;
  DWORD dwFlags;
  DWORD dwDataSize;
  DWORD dwNumObjs;
  LPDIOBJECTDATAFORMAT rgodf;
} DIDATAFORMAT, *LPDIDATAFORMAT;
typedef const DIDATAFORMAT *LPCDIDATAFORMAT;

/* Predefined data format for keyboard */
#ifdef __cplusplus
extern "C" {
#endif
static const DIDATAFORMAT c_dfDIKeyboard = {sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), 0, 256, 0, nullptr};
#ifdef __cplusplus
}
#endif

/* ── GUIDs (stubs) ──────────────────────────────────────────────────── */
static const GUID GUID_SysKeyboard = {};
static const GUID GUID_SysMouse = {};
static const GUID IID_IDirectInput8 = {};

/* ── VK constants ───────────────────────────────────────────────────── */
#ifndef VK_CAPITAL
#define VK_CAPITAL 0x14
#endif

/* GetKeyState stub */
inline SHORT GetKeyState(int) { return 0; }

/* ── IDirectInputDevice8 (stub interface) ───────────────────────────── */
struct IDirectInputDevice8 {
  virtual HRESULT SetDataFormat(LPCDIDATAFORMAT) { return S_OK; }
  virtual HRESULT SetCooperativeLevel(HWND, DWORD) { return S_OK; }
  virtual HRESULT SetProperty(const DIPROPHEADER*, const DIPROPHEADER*) { return S_OK; }
  virtual HRESULT Acquire() { return S_OK; }
  virtual HRESULT Unacquire() { return S_OK; }
  virtual HRESULT GetDeviceData(DWORD, LPDIDEVICEOBJECTDATA, DWORD*, DWORD) { return S_OK; }
  virtual HRESULT Release() { return 0; }
  virtual ~IDirectInputDevice8() {}
};

/* ── IDirectInput8 (stub interface) ─────────────────────────────────── */
struct IDirectInput8 {
  virtual HRESULT CreateDevice(const GUID&, IDirectInputDevice8**, void*) { return E_FAIL; }
  virtual HRESULT Release() { return 0; }
  virtual ~IDirectInput8() {}
};

typedef IDirectInput8 *LPDIRECTINPUT8;
typedef IDirectInputDevice8 *LPDIRECTINPUTDEVICE8;

/* DirectInput8Create stub */
inline HRESULT DirectInput8Create(HINSTANCE, DWORD, const GUID&, void**, void*) {
  return E_FAIL;
}
