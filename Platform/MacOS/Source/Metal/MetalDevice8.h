/**
 * MetalDevice8 — IDirect3DDevice8 implementation on Metal
 *
 * This is the core of the DX8→Metal adapter. It implements every method
 * of IDirect3DDevice8 from the d3d8_stub.h interface.
 *
 * Stage 0: All methods are stubs returning D3D_OK with proper state caching.
 * Subsequent stages will fill in real Metal implementations.
 *
 * NOTE: Only methods that exist in d3d8_stub.h IDirect3DDevice8 are marked
 * 'override'. Additional DX8 methods are provided but NOT virtual overrides.
 */
#pragma once

#ifdef __APPLE__

// Include d3d8/win_compat FIRST (before any ObjC framework headers)
#include <windows.h>  // macOS Win32 type shim
#include <d3d8.h>

// Forward declarations for Metal/ObjC types (avoid importing ObjC headers here)
#ifdef __OBJC__
@protocol MTLDevice;
@protocol MTLCommandQueue;
@protocol MTLCommandBuffer;
@protocol MTLRenderCommandEncoder;
@protocol CAMetalDrawable;
@class CAMetalLayer;
#else
typedef void *id;
#endif

#include <map>

class MetalSurface8;

class MetalDevice8 : public IDirect3DDevice8 {
public:
  MetalDevice8();
  virtual ~MetalDevice8();

  /// One-time initialization after construction.
  bool InitMetal(void *windowHandle);

  // ═══════════════════════════════════════════════════
  //  IUnknown — override
  // ═══════════════════════════════════════════════════
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override;
  STDMETHOD_(ULONG, AddRef)() override;
  STDMETHOD_(ULONG, Release)() override;

  // ═══════════════════════════════════════════════════
  //  Methods from d3d8_stub.h IDirect3DDevice8 — override
  // ═══════════════════════════════════════════════════
  STDMETHOD(TestCooperativeLevel)() override;
  STDMETHOD_(UINT, GetAvailableTextureMem)() override;
  STDMETHOD(ResourceManagerDiscardBytes)(DWORD Bytes) override;
  STDMETHOD(GetAdapterIdentifier)(UINT a, DWORD f,
                                  D3DADAPTER_IDENTIFIER8 *i) override;
  STDMETHOD(GetDeviceCaps)(D3DCAPS8 *pCaps) override;
  STDMETHOD(GetDisplayMode)(D3DDISPLAYMODE *pMode) override;

  STDMETHOD(CreateAdditionalSwapChain)(
      D3DPRESENT_PARAMETERS *pPresentationParameters,
      IDirect3DSwapChain8 **pSwapChain) override;
  STDMETHOD(Reset)(D3DPRESENT_PARAMETERS *pPresentationParameters) override;
  STDMETHOD(Present)(const void *pSourceRect, const void *pDestRect,
                     HWND hDestWindowOverride,
                     const void *pDirtyRegion) override;
  STDMETHOD(GetBackBuffer)(UINT BackBuffer, D3DBACKBUFFER_TYPE Type,
                           IDirect3DSurface8 **ppBackBuffer) override;

  STDMETHOD(SetGammaRamp)(DWORD Flags, const D3DGAMMARAMP *pRamp) override;
  STDMETHOD(GetGammaRamp)(D3DGAMMARAMP *pRamp) override;

  STDMETHOD(CreateTexture)(UINT Width, UINT Height, UINT Levels, DWORD Usage,
                           D3DFORMAT Format, D3DPOOL Pool,
                           IDirect3DTexture8 **ppTexture) override;
  STDMETHOD(CreateVolumeTexture)(
      UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage,
      D3DFORMAT Format, D3DPOOL Pool,
      IDirect3DVolumeTexture8 **ppVolumeTexture) override;
  STDMETHOD(CreateCubeTexture)(UINT EdgeLength, UINT Levels, DWORD Usage,
                               D3DFORMAT Format, D3DPOOL Pool,
                               IDirect3DCubeTexture8 **ppCubeTexture) override;
  STDMETHOD(CreateVertexBuffer)(
      UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
      IDirect3DVertexBuffer8 **ppVertexBuffer) override;
  STDMETHOD(CreateIndexBuffer)(UINT Length, DWORD Usage, D3DFORMAT Format,
                               D3DPOOL Pool,
                               IDirect3DIndexBuffer8 **ppIndexBuffer) override;
  STDMETHOD(CreateImageSurface)(UINT Width, UINT Height, D3DFORMAT Format,
                                IDirect3DSurface8 **ppSurface) override;

  STDMETHOD(CopyRects)(IDirect3DSurface8 *pSrc, const void *pSrcRectsArray,
                       UINT cRects, IDirect3DSurface8 *pDst,
                       const void *pDestPointsArray) override;
  STDMETHOD(UpdateTexture)(IDirect3DBaseTexture8 *pSrc,
                           IDirect3DBaseTexture8 *pDst) override;
  STDMETHOD(GetFrontBuffer)(IDirect3DSurface8 *pDestSurface) override;

  STDMETHOD(SetRenderTarget)(IDirect3DSurface8 *pRenderTarget,
                             IDirect3DSurface8 *pNewZStencil) override;
  STDMETHOD(GetRenderTarget)(IDirect3DSurface8 **ppRenderTarget) override;
  STDMETHOD(GetDepthStencilSurface)(
      IDirect3DSurface8 **ppZStencilSurface) override;
  STDMETHOD(SetDepthStencilSurface)(IDirect3DSurface8 *pNewZStencil) override;

  STDMETHOD(BeginScene)() override;
  STDMETHOD(EndScene)() override;
  STDMETHOD(Clear)(DWORD Count, const void *pRects, DWORD Flags, D3DCOLOR Color,
                   float Z, DWORD Stencil) override;

  STDMETHOD(SetTransform)(D3DTRANSFORMSTATETYPE State,
                          const D3DMATRIX *pMatrix) override;
  STDMETHOD(GetTransform)(D3DTRANSFORMSTATETYPE State,
                          D3DMATRIX *pMatrix) override;

  STDMETHOD(SetViewport)(const D3DVIEWPORT8 *pViewport) override;

  STDMETHOD(SetMaterial)(const D3DMATERIAL8 *pMaterial) override;
  STDMETHOD(SetLight)(DWORD Index, const D3DLIGHT8 *pLight) override;
  STDMETHOD(LightEnable)(DWORD Index, BOOL Enable) override;

  STDMETHOD(SetClipPlane)(DWORD Index, const float *pPlane) override;

  STDMETHOD(SetRenderState)(D3DRENDERSTATETYPE State, DWORD Value) override;
  STDMETHOD(GetRenderState)(D3DRENDERSTATETYPE State, DWORD *pValue) override;

  STDMETHOD(SetTexture)(DWORD Stage, IDirect3DBaseTexture8 *pTexture) override;
  STDMETHOD(SetTextureStageState)(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD Value) override;

  STDMETHOD(ValidateDevice)(DWORD *pNumPasses) override;

  STDMETHOD(DrawPrimitive)(DWORD PrimitiveType, UINT StartVertex,
                           UINT PrimitiveCount) override;
  STDMETHOD(DrawIndexedPrimitive)(DWORD PrimitiveType, UINT MinVertexIndex,
                                  UINT NumVertices, UINT StartIndex,
                                  UINT PrimitiveCount) override;
  STDMETHOD(DrawPrimitiveUP)(DWORD PrimitiveType, UINT PrimitiveCount,
                             const void *pVertexStreamZeroData,
                             UINT VertexStreamZeroStride) override;
  STDMETHOD(DrawIndexedPrimitiveUP)(DWORD PrimitiveType, UINT MinVertexIndex,
                                    UINT NumVertexIndices, UINT PrimitiveCount,
                                    const void *pIndexData,
                                    D3DFORMAT IndexDataFormat,
                                    const void *pVertexStreamZeroData,
                                    UINT VertexStreamZeroStride) override;

  STDMETHOD(CreateVertexShader)(const DWORD *pDeclaration,
                                const DWORD *pFunction, DWORD *pHandle,
                                DWORD Usage) override;
  STDMETHOD(SetVertexShader)(DWORD Handle) override;
  STDMETHOD(DeleteVertexShader)(DWORD Handle) override;
  STDMETHOD(SetVertexShaderConstant)(DWORD Register, const void *pConstantData,
                                     DWORD ConstantCount) override;

  STDMETHOD(SetStreamSource)(UINT StreamNumber,
                             IDirect3DVertexBuffer8 *pStreamData,
                             UINT Stride) override;
  STDMETHOD(SetIndices)(IDirect3DIndexBuffer8 *pIndexData,
                        UINT BaseVertexIndex) override;

  STDMETHOD(CreatePixelShader)(const DWORD *pFunction, DWORD *pHandle) override;
  STDMETHOD(SetPixelShader)(DWORD Handle) override;
  STDMETHOD(DeletePixelShader)(DWORD Handle) override;
  STDMETHOD(SetPixelShaderConstant)(DWORD Register, const void *pConstantData,
                                    DWORD ConstantCount) override;

  // ═══════════════════════════════════════════════════
  //  Non-override helper methods (not in d3d8_stub.h)
  // ═══════════════════════════════════════════════════
  HRESULT GetDirect3D(IDirect3D8 **ppD3D8);
  HRESULT GetViewport(D3DVIEWPORT8 *pViewport);
  HRESULT GetMaterial(D3DMATERIAL8 *pMaterial);
  HRESULT GetLight(DWORD Index, D3DLIGHT8 *pLight);
  HRESULT GetLightEnable(DWORD Index, BOOL *pEnable);
  HRESULT GetTexture(DWORD Stage, IDirect3DBaseTexture8 **ppTexture);
  HRESULT GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,
                               DWORD *pValue);
  HRESULT GetStreamSource(UINT StreamNumber,
                          IDirect3DVertexBuffer8 **ppStreamData, UINT *pStride);
  HRESULT GetIndices(IDirect3DIndexBuffer8 **ppIndexData,
                     UINT *pBaseVertexIndex);

  // Metal Accessor
  void *GetMTLDevice() const { return m_Device; }

private:
  ULONG m_RefCount;

  // --- Metal Core Objects (opaque pointers, actual types in .mm) ---
  void *m_Device;       // id<MTLDevice>
  void *m_CommandQueue; // id<MTLCommandQueue>
  void *m_MetalLayer;   // CAMetalLayer*

  // --- Per-Frame State ---
  void *m_CurrentCommandBuffer; // id<MTLCommandBuffer>
  void *m_CurrentDrawable;      // id<CAMetalDrawable>
  void *m_CurrentEncoder;       // id<MTLRenderCommandEncoder>
  bool m_InScene;

  // --- Cached DX8 State ---
  DWORD m_RenderStates[256];

  static const int MAX_TEXTURE_STAGES = 8;
  DWORD m_TextureStageStates[MAX_TEXTURE_STAGES][32];
  IDirect3DBaseTexture8 *m_Textures[MAX_TEXTURE_STAGES];

  D3DMATRIX m_Transforms[260];
  D3DVIEWPORT8 m_Viewport;
  D3DMATERIAL8 m_Material;

  static const int MAX_LIGHTS = 4;
  D3DLIGHT8 m_Lights[MAX_LIGHTS];
  BOOL m_LightEnabled[MAX_LIGHTS];

  IDirect3DVertexBuffer8 *m_StreamSource;
  UINT m_StreamStride;
  IDirect3DIndexBuffer8 *m_IndexBuffer;
  UINT m_BaseVertexIndex;

  DWORD m_VertexShader;
  DWORD m_PixelShader;

  void *m_HWND;
  float m_ScreenWidth;
  float m_ScreenHeight;

  // --- Helper ---
  void *GetPSO(DWORD fvf);         // builds 64-bit key from fvf + blend state
  uint64_t BuildPSOKey(DWORD fvf); // computes PSO cache key
  void *GetDepthStencilState();
  void CreateDepthTexture(UINT width, UINT height);
  void ApplyPerDrawState();           // cull mode, depth/stencil
  void *GetSamplerState(DWORD stage); // Stage 7: sampler cache

  // --- Metal Render Pipeline State ---
  void *m_Library;                       // id<MTLLibrary>
  void *m_FunctionVertex;                // id<MTLFunction>
  void *m_FunctionFragment;              // id<MTLFunction>
  std::map<uint64_t, void *> m_PsoCache; // psoKey -> id<MTLRenderPipelineState>

  // --- Depth/Stencil ---
  void *m_DepthTexture;      // id<MTLTexture> (Depth32Float)
  void *m_DepthStencilState; // id<MTLDepthStencilState> (cached, current)
  bool m_DepthStateDirty;    // re-create DSS when render states change
  std::map<uint32_t, void *>
      m_DepthStencilStateCache; // key -> id<MTLDepthStencilState>

  // --- Sampler State Cache (Stage 7) ---
  std::map<uint32_t, void *> m_SamplerStateCache; // key -> id<MTLSamplerState>

  // --- Default Zero Buffer for missing vertex attributes ---
  void *m_ZeroBuffer; // id<MTLBuffer>, 16 bytes of zeros, bound at index 30

  // --- Default Render Target / Depth Surfaces ---
  MetalSurface8
      *m_DefaultRTSurface; // returned by GetRenderTarget / GetBackBuffer
  MetalSurface8 *m_DefaultDepthSurface; // returned by GetDepthStencilSurface
};

#endif // __APPLE__
