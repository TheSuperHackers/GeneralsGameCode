#include "PreRTS.h"

// SDLPlatformWindow.cpp is shared with the legacy platform boundary and still
// expects the process-wide ATL module that the removed WOL browser used to
// provide. GeneralsMD has no browser usage; this is only the compatibility
// definition required by that platform boundary.
CComModule _Module;
