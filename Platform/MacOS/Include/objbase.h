/*
**  macOS compatibility shim for <objbase.h>
**  Provides COM types needed by DX8 SDK headers: IUnknown, GUID, REFIID,
**  STDMETHOD, DECLARE_INTERFACE_, DEFINE_GUID, etc.
**
**  Copyright 2025 TheSuperHackers
**  Licensed under GPL-3.0
*/
#pragma once
#ifndef _OBJBASE_H_MACOS_SHIM_
#define _OBJBASE_H_MACOS_SHIM_

#include <windows.h> // our shim — brings in HRESULT, ULONG, LPVOID, etc.

// ── GUID / IID / CLSID ────────────────────────────────────────────────
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;
#endif

typedef GUID IID;
typedef GUID CLSID;

#ifdef __cplusplus
typedef const GUID &REFGUID;
typedef const IID &REFIID;
typedef const CLSID &REFCLSID;
#else
typedef const GUID *REFGUID;
typedef const IID *REFIID;
typedef const CLSID *REFCLSID;
#endif

typedef GUID *LPGUID;
typedef IID *LPIID;
typedef CLSID *LPCLSID;

// ── DEFINE_GUID ────────────────────────────────────────────────────────
#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)           \
  extern const GUID name = {l, w1, w2, {b1, b2, b3, b4, b5, b6, b7, b8}}
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)           \
  extern const GUID name
#endif

// ── IsEqualGUID ────────────────────────────────────────────────────────
#ifdef __cplusplus
inline bool IsEqualGUID(REFGUID a, REFGUID b) {
  return memcmp(&a, &b, sizeof(GUID)) == 0;
}
inline bool IsEqualIID(REFIID a, REFIID b) { return IsEqualGUID(a, b); }
inline bool IsEqualCLSID(REFCLSID a, REFCLSID b) { return IsEqualGUID(a, b); }
#endif

// ── COM Interface macros ───────────────────────────────────────────────
// These macros are used by d3d8.h to declare COM interfaces.
// On macOS/C++ they expand to simple struct inheritance.

#ifndef STDMETHOD
#define STDMETHOD(method) virtual HRESULT WINAPI method
#endif
#ifndef STDMETHOD_
#define STDMETHOD_(type, method) virtual type WINAPI method
#endif
#ifndef PURE
#define PURE = 0
#endif
#ifndef THIS_
#define THIS_
#endif
#ifndef THIS
#define THIS void
#endif
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

#ifndef DECLARE_INTERFACE
#define DECLARE_INTERFACE(iface) struct iface
#endif

#ifndef DECLARE_INTERFACE_
#define DECLARE_INTERFACE_(iface, baseiface) struct iface : public baseiface
#endif

// ── IUnknown ───────────────────────────────────────────────────────────
#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

// Forward-declare the GUID
extern const IID IID_IUnknown;

#ifdef __cplusplus
struct IUnknown {
  virtual HRESULT WINAPI QueryInterface(REFIID riid, void **ppvObject) = 0;
  virtual ULONG WINAPI AddRef() = 0;
  virtual ULONG WINAPI Release() = 0;
  virtual ~IUnknown() {}
};
typedef IUnknown *LPUNKNOWN;
#endif

#endif // __IUnknown_INTERFACE_DEFINED__

// ── CoInitialize / CoUninitialize stubs ────────────────────────────────
#ifdef __cplusplus
inline HRESULT CoInitialize(LPVOID pvReserved) {
  (void)pvReserved;
  return S_OK;
}
inline void CoUninitialize() {}
inline HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                                DWORD dwClsContext, REFIID riid, LPVOID *ppv) {
  (void)rclsid;
  (void)pUnkOuter;
  (void)dwClsContext;
  (void)riid;
  *ppv = nullptr;
  return E_NOTIMPL;
}
#endif
// ── IDispatch stub (used by dx8webbrowser.h) ──────────────────────────
#ifndef __IDispatch_FWD_DEFINED__
#define __IDispatch_FWD_DEFINED__
typedef IUnknown IDispatch;
typedef IDispatch *LPDISPATCH;
#endif

#endif // _OBJBASE_H_MACOS_SHIM_
