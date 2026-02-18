#pragma once

// Stub header for legacy MacOSRenderDevice buffer classes.
// These are no longer used — the Metal backend uses MetalVertexBuffer8
// and MetalIndexBuffer8 through the DX8 wrapper layer.

class MacOSVertexBufferClass {
public:
  void *Get_Metal_Buffer() { return nullptr; }
};

class MacOSIndexBufferClass {
public:
  void *Get_Metal_Buffer() { return nullptr; }
  bool Is_32Bit() { return false; }
};
