/* macOS shim: oleauto.h — OLE Automation stubs */
#pragma once
#include "windows.h"

typedef wchar_t OLECHAR;
typedef OLECHAR *BSTR;
typedef OLECHAR *LPOLESTR;

inline BSTR SysAllocString(const OLECHAR *) { return NULL; }
inline void SysFreeString(BSTR) {}
inline unsigned int SysStringLen(BSTR) { return 0; }
