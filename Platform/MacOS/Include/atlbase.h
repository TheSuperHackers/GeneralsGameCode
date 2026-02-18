/* macOS shim: atlbase.h — ATL stubs */
#pragma once

#ifndef _ATLBASE_MACOS_SHIM_
#define _ATLBASE_MACOS_SHIM_

/* CComModule — minimal stub */
struct CComModule {
  void Init(void * = 0, void * = 0, void * = 0) {}
  void Term() {}
};

#endif
