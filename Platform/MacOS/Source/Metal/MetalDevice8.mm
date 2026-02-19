/**
 * MetalDevice8.mm — IDirect3DDevice8 implementation on Apple Metal
 *
 * Stage 0: Skeleton — all methods log and return D3D_OK.
 * BeginScene/EndScene/Present/Clear have real Metal frame lifecycle code.
 */
#ifdef __APPLE__

// Import ObjC/Metal frameworks FIRST, before win_compat.h
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// Now include our header (which includes d3d8.h / win_compat.h)
#import "MetalDevice8.h"
#include "MetalIndexBuffer8.h"
#include "MetalSurface8.h"
#include "MetalTexture8.h"
#include "MetalVertexBuffer8.h"
#include <cstdio>
#include <cstring>

// Global MTLDevice pointer for VB/IB (avoids MTLCreateSystemDefaultDevice)
// Set during MetalDevice8::InitMetal(), cleared in destructor.
void *g_MetalMTLDevice = nullptr;

// D3DXGetFVFVertexSize is inline in d3dx8core.h
#include "d3dx8core.h"

// ─────────────────────────────────────────────────────
//  Helpers: Cast opaque pointers to Metal types
// ─────────────────────────────────────────────────────
#define MTL_DEVICE ((__bridge id<MTLDevice>)m_Device)
#define MTL_QUEUE ((__bridge id<MTLCommandQueue>)m_CommandQueue)
#define MTL_LAYER ((__bridge CAMetalLayer *)m_MetalLayer)
#define MTL_CMD_BUF ((__bridge id<MTLCommandBuffer>)m_CurrentCommandBuffer)
#define MTL_DRAWABLE ((__bridge id<CAMetalDrawable>)m_CurrentDrawable)
#define MTL_ENCODER ((__bridge id<MTLRenderCommandEncoder>)m_CurrentEncoder)

#define SET_MTL(member, val)                                                   \
  do {                                                                         \
    m_##member = (__bridge_retained void *)(val);                              \
  } while (0)
#define CLEAR_MTL(member)                                                      \
  do {                                                                         \
    if (m_##member) {                                                          \
      CFRelease(m_##member);                                                   \
      m_##member = nullptr;                                                    \
    }                                                                          \
  } while (0)

// ─────────────────────────────────────────────────────
//  Construction / Destruction
// ─────────────────────────────────────────────────────

MetalDevice8::MetalDevice8()
    : m_RefCount(1), m_Device(nullptr), m_CommandQueue(nullptr),
      m_MetalLayer(nullptr), m_CurrentCommandBuffer(nullptr),
      m_CurrentDrawable(nullptr), m_CurrentEncoder(nullptr), m_InScene(false),
      m_StreamSource(nullptr), m_StreamStride(0), m_IndexBuffer(nullptr),
      m_BaseVertexIndex(0), m_VertexShader(0), m_PixelShader(0),
      m_HWND(nullptr), m_ScreenWidth(800), m_ScreenHeight(600),
      m_Library(nullptr), m_FunctionVertex(nullptr),
      m_FunctionFragment(nullptr), m_DepthTexture(nullptr),
      m_DepthStencilState(nullptr), m_DepthStateDirty(true),
      m_ZeroBuffer(nullptr), m_DefaultRTSurface(nullptr),
      m_DefaultDepthSurface(nullptr) {
  memset(m_RenderStates, 0, sizeof(m_RenderStates));
  memset(m_TextureStageStates, 0, sizeof(m_TextureStageStates));
  memset(m_Textures, 0, sizeof(m_Textures));
  memset(m_Transforms, 0, sizeof(m_Transforms));
  memset(&m_Viewport, 0, sizeof(m_Viewport));
  memset(&m_Material, 0, sizeof(m_Material));
  memset(m_Lights, 0, sizeof(m_Lights));
  memset(m_LightEnabled, 0, sizeof(m_LightEnabled));

  auto setIdentity = [](D3DMATRIX &m) {
    memset(&m, 0, sizeof(m));
    m._11 = m._22 = m._33 = m._44 = 1.0f;
  };
  setIdentity(m_Transforms[D3DTS_VIEW]);
  setIdentity(m_Transforms[D3DTS_PROJECTION]);
  setIdentity(m_Transforms[D3DTS_WORLD]);

  // DX8 default render states (per spec)
  m_RenderStates[D3DRS_ZENABLE] = TRUE;           // Depth testing on
  m_RenderStates[D3DRS_ZWRITEENABLE] = TRUE;      // Depth writing on
  m_RenderStates[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL; // Standard compare
  m_RenderStates[D3DRS_CULLMODE] = D3DCULL_CCW;   // CCW culling
  m_RenderStates[D3DRS_ALPHABLENDENABLE] = FALSE;
  m_RenderStates[D3DRS_SRCBLEND] = D3DBLEND_ONE;   // DX8 default
  m_RenderStates[D3DRS_DESTBLEND] = D3DBLEND_ZERO; // DX8 default
  m_RenderStates[D3DRS_COLORWRITEENABLE] = 0xF;    // All channels

  // DX8 default texture stage states (per spec)
  // Stage 0: MODULATE color (tex * diffuse), SELECTARG1 alpha (texture alpha)
  m_TextureStageStates[0][D3DTSS_COLOROP] = D3DTOP_MODULATE;
  m_TextureStageStates[0][D3DTSS_COLORARG1] = D3DTA_TEXTURE;
  m_TextureStageStates[0][D3DTSS_COLORARG2] = D3DTA_CURRENT;
  m_TextureStageStates[0][D3DTSS_ALPHAOP] = D3DTOP_SELECTARG1;
  m_TextureStageStates[0][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
  m_TextureStageStates[0][D3DTSS_ALPHAARG2] = D3DTA_CURRENT;
  // Stage 1: DISABLE (default)
  m_TextureStageStates[1][D3DTSS_COLOROP] = D3DTOP_DISABLE;
  m_TextureStageStates[1][D3DTSS_ALPHAOP] = D3DTOP_DISABLE;
  // Default sampler states: WRAP + LINEAR
  for (int s = 0; s < MAX_TEXTURE_STAGES; s++) {
    m_TextureStageStates[s][D3DTSS_ADDRESSU] = D3DTADDRESS_WRAP;
    m_TextureStageStates[s][D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
    m_TextureStageStates[s][D3DTSS_MAGFILTER] = D3DTEXF_LINEAR;
    m_TextureStageStates[s][D3DTSS_MINFILTER] = D3DTEXF_LINEAR;
    m_TextureStageStates[s][D3DTSS_MIPFILTER] = D3DTEXF_NONE;
  }

  // DX8 default lighting render states
  m_RenderStates[D3DRS_LIGHTING] = TRUE;
  m_RenderStates[D3DRS_AMBIENT] = 0x00000000; // Black global ambient
  m_RenderStates[D3DRS_SPECULARENABLE] = FALSE;
  m_RenderStates[D3DRS_NORMALIZENORMALS] = FALSE;
  m_RenderStates[D3DRS_DIFFUSEMATERIALSOURCE] = D3DMCS_MATERIAL;
  m_RenderStates[D3DRS_AMBIENTMATERIALSOURCE] = D3DMCS_MATERIAL;
  m_RenderStates[D3DRS_SPECULARMATERIALSOURCE] = D3DMCS_MATERIAL;
  m_RenderStates[D3DRS_EMISSIVEMATERIALSOURCE] = D3DMCS_MATERIAL;

  // DX8 default fog render states
  m_RenderStates[D3DRS_FOGENABLE] = FALSE;
  m_RenderStates[D3DRS_FOGCOLOR] = 0x00000000;
  m_RenderStates[D3DRS_FOGTABLEMODE] = D3DFOG_NONE;
  m_RenderStates[D3DRS_FOGVERTEXMODE] = D3DFOG_NONE;
  // fogStart=0.0, fogEnd=1.0, fogDensity=1.0 stored as DWORD bit-casts of float
  {
    float fs = 0.0f;
    memcpy(&m_RenderStates[D3DRS_FOGSTART], &fs, 4);
  }
  {
    float fe = 1.0f;
    memcpy(&m_RenderStates[D3DRS_FOGEND], &fe, 4);
  }
  {
    float fd = 1.0f;
    memcpy(&m_RenderStates[D3DRS_FOGDENSITY], &fd, 4);
  }

  // DX8 default material (white diffuse/ambient, no specular/emissive)
  m_Material.Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
  m_Material.Ambient = {1.0f, 1.0f, 1.0f, 1.0f};
  m_Material.Specular = {0.0f, 0.0f, 0.0f, 0.0f};
  m_Material.Emissive = {0.0f, 0.0f, 0.0f, 0.0f};
  m_Material.Power = 0.0f;
}

MetalDevice8::~MetalDevice8() {
  // Release sampler state cache
  for (auto &pair : m_SamplerStateCache) {
    if (pair.second)
      CFRelease(pair.second);
  }
  m_SamplerStateCache.clear();

  // Release depth/stencil state cache
  for (auto &pair : m_DepthStencilStateCache) {
    if (pair.second)
      CFRelease(pair.second);
  }
  m_DepthStencilStateCache.clear();
  m_DepthStencilState = nullptr; // just a borrowed pointer, don't release

  // Release default surfaces
  if (m_DefaultRTSurface) {
    m_DefaultRTSurface->Release();
    m_DefaultRTSurface = nullptr;
  }
  if (m_DefaultDepthSurface) {
    m_DefaultDepthSurface->Release();
    m_DefaultDepthSurface = nullptr;
  }

  // Release depth texture
  if (m_DepthTexture) {
    CFRelease(m_DepthTexture);
    m_DepthTexture = nullptr;
  }

  // Release PSO cache
  for (auto &pair : m_PsoCache) {
    if (pair.second)
      CFRelease(pair.second);
  }
  m_PsoCache.clear();

  // Release shader library and functions
  if (m_FunctionFragment) {
    CFRelease(m_FunctionFragment);
    m_FunctionFragment = nullptr;
  }
  if (m_FunctionVertex) {
    CFRelease(m_FunctionVertex);
    m_FunctionVertex = nullptr;
  }
  if (m_Library) {
    CFRelease(m_Library);
    m_Library = nullptr;
  }

  CLEAR_MTL(CurrentEncoder);
  CLEAR_MTL(CurrentCommandBuffer);
  CLEAR_MTL(CurrentDrawable);
  CLEAR_MTL(CommandQueue);
  g_MetalMTLDevice = nullptr;
  CLEAR_MTL(Device);
  // MetalLayer is owned by the view, we don't release it
  m_MetalLayer = nullptr;
  fprintf(stderr, "[MetalDevice8] Destroyed\n");
}

#include <simd/simd.h>

// --- Shader Data Structures (Must match MacOSShaders.metal) ---
struct MetalUniforms {
  simd::float4x4 world;
  simd::float4x4 view;
  simd::float4x4 projection;
  simd::float2 screenSize;
  int useProjection; // 0=None, 1=3D, 2=2D(ScreenSpace)
  uint32_t shaderSettings;
};

// Stage 7: TextureStageConfig (matches MacOSShaders.metal)
struct TextureStageConfig {
  uint32_t colorOp;
  uint32_t colorArg1;
  uint32_t colorArg2;
  uint32_t alphaOp;
  uint32_t alphaArg1;
  uint32_t alphaArg2;
  uint32_t _pad0;
  uint32_t _pad1;
};

// Stage 7: FragmentUniforms (matches MacOSShaders.metal, buffer 2)
struct FragmentUniforms {
  TextureStageConfig stages[2];
  simd::float4 textureFactor; // D3DRS_TEXTUREFACTOR as RGBA float
  simd::float4 fogColor;
  float fogStart;
  float fogEnd;
  float fogDensity;
  uint32_t fogMode;
  uint32_t alphaTestEnable;
  uint32_t alphaFunc; // D3DCMP enum
  float alphaRef;     // normalized 0..1
  uint32_t hasTexture0;
  uint32_t hasTexture1;
  uint32_t _pad0;
  uint32_t _pad1;
};

// Stage 8: LightData (matches MacOSShaders.metal)
// Per-light parameters for DX8 per-vertex lighting
struct LightData {
  simd::float4 diffuse;
  simd::float4 ambient;
  simd::float4 specular;
  simd::float3 position;
  float range;
  simd::float3 direction;
  float falloff;
  float attenuation0;
  float attenuation1;
  float attenuation2;
  float theta;   // inner cone (radians)
  float phi;     // outer cone (radians)
  uint32_t type; // 1=point, 2=spot, 3=directional
  uint32_t enabled;
  float _pad;
};

// Stage 8: LightingUniforms (matches MacOSShaders.metal, buffer 3)
struct LightingUniforms {
  LightData lights[4];
  simd::float4 materialDiffuse;
  simd::float4 materialAmbient;
  simd::float4 materialSpecular;
  simd::float4 materialEmissive;
  float materialPower;
  simd::float4 globalAmbient;
  uint32_t lightingEnabled;
  uint32_t diffuseSource; // D3DMCS: 0=material, 1=color1, 2=color2
  uint32_t ambientSource;
  uint32_t specularSource;
  uint32_t emissiveSource;
  uint32_t hasNormals; // 1 if FVF has D3DFVF_NORMAL
  // Stage 9: Fog parameters (for vertex fog computation)
  float fogStart;
  float fogEnd;
  float fogDensity;
  uint32_t fogMode; // 0=NONE, 1=EXP, 2=EXP2, 3=LINEAR
};

// FVF bit definitions: see d3d8_stub.h (D3DFVF_XYZ, D3DFVF_XYZRHW, etc.)

// Helper to get FVF from an opaque IDirect3DVertexBuffer8
static DWORD GetBufferFVF(IDirect3DVertexBuffer8 *vb) {
  if (!vb)
    return 0;
  D3DVERTEXBUFFER_DESC desc;
  if (SUCCEEDED(vb->GetDesc(&desc))) {
    return desc.FVF;
  }
  return 0;
}

bool MetalDevice8::InitMetal(void *windowHandle) {
  m_HWND = windowHandle;

  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  if (!device) {
    fprintf(stderr,
            "[MetalDevice8] ERROR: MTLCreateSystemDefaultDevice failed\n");
    return false;
  }
  SET_MTL(Device, device);
  g_MetalMTLDevice = m_Device; // Global access for VB/IB

  // Create a small zero buffer for default vertex attributes (missing FVF
  // components)
  {
    uint8_t zeros[16] = {0};
    id<MTLBuffer> zeroBuf =
        [device newBufferWithBytes:zeros
                            length:sizeof(zeros)
                           options:MTLResourceStorageModeShared];
    m_ZeroBuffer = (__bridge_retained void *)zeroBuf;
  }

  id<MTLCommandQueue> queue = [device newCommandQueue];
  SET_MTL(CommandQueue, queue);

  // Load Shaders (Compile from Source at Runtime)
  NSError *error = nil;
  NSString *shaderSource = nil;
  // Try multiple paths to find the shader source
  NSArray *shaderPaths = @[
    @"Platform/MacOS/Source/Main/MacOSShaders.metal",
    @"../../Platform/MacOS/Source/Main/MacOSShaders.metal",
    @"../Platform/MacOS/Source/Main/MacOSShaders.metal",
  ];

  NSString *shaderPath = nil;
  for (NSString *path in shaderPaths) {
    if ([[NSFileManager defaultManager] fileExistsAtPath:path]) {
      shaderPath = path;
      break;
    }
  }

  if (shaderPath) {
    shaderSource = [NSString stringWithContentsOfFile:shaderPath
                                             encoding:NSUTF8StringEncoding
                                                error:&error];
  } else {
    fprintf(stderr, "[MetalDevice8] WARNING: Could not find MacOSShaders.metal "
                    "in any search path\n");
    fprintf(stderr, "[MetalDevice8] CWD: %s\n",
            [[[NSFileManager defaultManager] currentDirectoryPath] UTF8String]);
  }

  id<MTLLibrary> library = nil;
  if (shaderSource) {
    MTLCompileOptions *opts = [[MTLCompileOptions alloc] init];
    library = [device newLibraryWithSource:shaderSource
                                   options:opts
                                     error:&error];
  }

  if (!library) {
    fprintf(stderr, "[MetalDevice8] ERROR: Failed to compile shaders: %s\n",
            [[error localizedDescription] UTF8String]);
    fprintf(stderr, "Shader path checked: %s\n", [shaderPath UTF8String]);
  } else {
    SET_MTL(Library, library);

    id<MTLFunction> vertFunc = [library newFunctionWithName:@"vertex_main"];
    if (vertFunc)
      SET_MTL(FunctionVertex, vertFunc);

    id<MTLFunction> fragFunc = [library newFunctionWithName:@"fragment_main"];
    if (fragFunc)
      SET_MTL(FunctionFragment, fragFunc);

    if (!vertFunc || !fragFunc) {
      fprintf(
          stderr,
          "[MetalDevice8] ERROR: Failed to find vertex_main/fragment_main\n");
    } else {
      fprintf(stderr, "[MetalDevice8] Shaders compiled successfully.\n");
    }
  }

  CAMetalLayer *layer = [CAMetalLayer layer];
  layer.device = device;
  layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  layer.framebufferOnly = NO;
  m_MetalLayer = (__bridge_retained void *)layer;

  NSWindow *window = (__bridge NSWindow *)windowHandle;
  if (window) {
    window.contentView.layer = layer;
    window.contentView.wantsLayer = YES;
    CGSize viewSize = window.contentView.bounds.size;
    CGFloat scale = window.backingScaleFactor;
    layer.drawableSize =
        CGSizeMake(viewSize.width * scale, viewSize.height * scale);
    m_ScreenWidth = viewSize.width;
    m_ScreenHeight = viewSize.height;

    fprintf(stderr, "[MetalDevice8] Initialized: %gx%g (drawable: %gx%g)\n",
            m_ScreenWidth, m_ScreenHeight, layer.drawableSize.width,
            layer.drawableSize.height);
  } else {
    // Window not yet available — use fallback size
    layer.drawableSize = CGSizeMake(m_ScreenWidth, m_ScreenHeight);
    fprintf(stderr,
            "[MetalDevice8] WARNING: No window handle, using fallback %gx%g\n",
            m_ScreenWidth, m_ScreenHeight);
  }

  // Create depth texture matching the drawable size
  UINT depthW = (UINT)layer.drawableSize.width;
  UINT depthH = (UINT)layer.drawableSize.height;
  if (depthW > 0 && depthH > 0) {
    CreateDepthTexture(depthW, depthH);
  } else {
    fprintf(stderr,
            "[MetalDevice8] WARNING: Skipping depth texture (size 0x0)\n");
  }

  // Create default render target and depth stencil surfaces
  // The engine's DX8Wrapper stores these to pass back to SetRenderTarget.
  UINT surfW = (UINT)m_ScreenWidth;
  UINT surfH = (UINT)m_ScreenHeight;
  if (surfW == 0)
    surfW = 800;
  if (surfH == 0)
    surfH = 600;

  m_DefaultRTSurface = W3DNEW MetalSurface8(this, MetalSurface8::kColor, surfW,
                                            surfH, D3DFMT_A8R8G8B8);
  m_DefaultDepthSurface = W3DNEW MetalSurface8(this, MetalSurface8::kDepth,
                                               surfW, surfH, D3DFMT_D24S8);
  fprintf(stderr,
          "[MetalDevice8] Default surfaces created: RT %ux%u, DS %ux%u\n",
          surfW, surfH, surfW, surfH);

  return true;
}

// ─────────────────────────────────────────────────────
//  Depth Buffer Helpers
// ─────────────────────────────────────────────────────

void MetalDevice8::CreateDepthTexture(UINT width, UINT height) {
  // Release old depth texture if any
  if (m_DepthTexture) {
    CFRelease(m_DepthTexture);
    m_DepthTexture = nullptr;
  }

  // Also need to recreate PSOs since depthAttachmentPixelFormat changes
  for (auto &pair : m_PsoCache) {
    if (pair.second)
      CFRelease(pair.second);
  }
  m_PsoCache.clear();

  MTLTextureDescriptor *depthDesc = [MTLTextureDescriptor
      texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
                                   width:width
                                  height:height
                               mipmapped:NO];
  depthDesc.usage = MTLTextureUsageRenderTarget;
  depthDesc.storageMode = MTLStorageModePrivate;

  id<MTLTexture> depthTex = [MTL_DEVICE newTextureWithDescriptor:depthDesc];
  if (depthTex) {
    depthTex.label = @"MetalDevice8 DepthStencilBuffer";
    m_DepthTexture = (__bridge_retained void *)depthTex;
    fprintf(stderr,
            "[MetalDevice8] Depth+Stencil texture created: %u x %u "
            "(Depth32Float_Stencil8)\n",
            width, height);
  } else {
    fprintf(stderr,
            "[MetalDevice8] ERROR: Failed to create depth texture %u x %u\n",
            width, height);
  }

  // Force depth stencil state recreation
  m_DepthStateDirty = true;
}

static MTLCompareFunction MapD3DCmpToMTL(DWORD d3dCmp) {
  switch (d3dCmp) {
  case D3DCMP_NEVER:
    return MTLCompareFunctionNever;
  case D3DCMP_LESS:
    return MTLCompareFunctionLess;
  case D3DCMP_EQUAL:
    return MTLCompareFunctionEqual;
  case D3DCMP_LESSEQUAL:
    return MTLCompareFunctionLessEqual;
  case D3DCMP_GREATER:
    return MTLCompareFunctionGreater;
  case D3DCMP_NOTEQUAL:
    return MTLCompareFunctionNotEqual;
  case D3DCMP_GREATEREQUAL:
    return MTLCompareFunctionGreaterEqual;
  case D3DCMP_ALWAYS:
    return MTLCompareFunctionAlways;
  default:
    return MTLCompareFunctionLessEqual; // DX8 default
  }
}

// Map D3DSTENCILOP → MTLStencilOperation
static MTLStencilOperation MapD3DStencilOpToMTL(DWORD op) {
  switch (op) {
  case D3DSTENCILOP_KEEP:
    return MTLStencilOperationKeep;
  case D3DSTENCILOP_ZERO:
    return MTLStencilOperationZero;
  case D3DSTENCILOP_REPLACE:
    return MTLStencilOperationReplace;
  case D3DSTENCILOP_INCRSAT:
    return MTLStencilOperationIncrementClamp;
  case D3DSTENCILOP_DECRSAT:
    return MTLStencilOperationDecrementClamp;
  case D3DSTENCILOP_INVERT:
    return MTLStencilOperationInvert;
  case D3DSTENCILOP_INCR:
    return MTLStencilOperationIncrementWrap;
  case D3DSTENCILOP_DECR:
    return MTLStencilOperationDecrementWrap;
  default:
    return MTLStencilOperationKeep;
  }
}

void *MetalDevice8::GetDepthStencilState() {
  if (!m_DepthStateDirty && m_DepthStencilState)
    return m_DepthStencilState;

  // Build cache key from relevant render states (depth + stencil)
  DWORD zEnable = m_RenderStates[D3DRS_ZENABLE];
  DWORD zWrite = m_RenderStates[D3DRS_ZWRITEENABLE];
  DWORD zFunc = m_RenderStates[D3DRS_ZFUNC];
  DWORD stencilEn = m_RenderStates[D3DRS_STENCILENABLE];
  DWORD stencilFunc = m_RenderStates[D3DRS_STENCILFUNC];
  DWORD stencilFail = m_RenderStates[D3DRS_STENCILFAIL];
  DWORD stencilZFail = m_RenderStates[D3DRS_STENCILZFAIL];
  DWORD stencilPass = m_RenderStates[D3DRS_STENCILPASS];

  // 64-bit key: depth bits (low 6) + stencil bits (7..31)
  uint64_t key = (zEnable & 1) | ((zWrite & 1) << 1) | ((zFunc & 0xF) << 2) |
                 ((stencilEn & 1ULL) << 6) | ((stencilFunc & 0xFULL) << 7) |
                 ((stencilFail & 0xFULL) << 11) |
                 ((stencilZFail & 0xFULL) << 15) |
                 ((stencilPass & 0xFULL) << 19);

  auto it = m_DepthStencilStateCache.find((uint32_t)key);
  if (it != m_DepthStencilStateCache.end()) {
    m_DepthStencilState = it->second;
    m_DepthStateDirty = false;
    return m_DepthStencilState;
  }

  MTLDepthStencilDescriptor *dsd = [[MTLDepthStencilDescriptor alloc] init];
  if (zEnable) {
    dsd.depthCompareFunction = MapD3DCmpToMTL(zFunc);
    dsd.depthWriteEnabled = (zWrite != 0);
  } else {
    dsd.depthCompareFunction = MTLCompareFunctionAlways;
    dsd.depthWriteEnabled = NO;
  }

  // Stencil configuration
  if (stencilEn) {
    DWORD readMask = m_RenderStates[D3DRS_STENCILMASK];
    DWORD writeMask = m_RenderStates[D3DRS_STENCILWRITEMASK];

    MTLStencilDescriptor *stencilDesc = [[MTLStencilDescriptor alloc] init];
    stencilDesc.stencilCompareFunction = MapD3DCmpToMTL(stencilFunc);
    stencilDesc.stencilFailureOperation = MapD3DStencilOpToMTL(stencilFail);
    stencilDesc.depthFailureOperation = MapD3DStencilOpToMTL(stencilZFail);
    stencilDesc.depthStencilPassOperation = MapD3DStencilOpToMTL(stencilPass);
    stencilDesc.readMask = readMask & 0xFF;
    stencilDesc.writeMask = writeMask & 0xFF;

    // DX8 doesn't have separate front/back stencil (that's DX9+)
    dsd.frontFaceStencil = stencilDesc;
    dsd.backFaceStencil = stencilDesc;
  }

  id<MTLDepthStencilState> dss =
      [MTL_DEVICE newDepthStencilStateWithDescriptor:dsd];
  if (dss) {
    m_DepthStencilStateCache[(uint32_t)key] = (__bridge_retained void *)dss;
    m_DepthStencilState = (__bridge void *)dss;
  }

  m_DepthStateDirty = false;
  return m_DepthStencilState;
}

// ─────────────────────────────────────────────────────
//  Stage 6: D3DBLEND → MTLBlendFactor mapping
//  Spec: d3d8_stub.h D3DBLEND enum
// ─────────────────────────────────────────────────────
static MTLBlendFactor MapD3DBlendToMTL(DWORD blend) {
  switch (blend) {
  case D3DBLEND_ZERO:
    return MTLBlendFactorZero;
  case D3DBLEND_ONE:
    return MTLBlendFactorOne;
  case D3DBLEND_SRCCOLOR:
    return MTLBlendFactorSourceColor;
  case D3DBLEND_INVSRCCOLOR:
    return MTLBlendFactorOneMinusSourceColor;
  case D3DBLEND_SRCALPHA:
    return MTLBlendFactorSourceAlpha;
  case D3DBLEND_INVSRCALPHA:
    return MTLBlendFactorOneMinusSourceAlpha;
  case D3DBLEND_DESTALPHA:
    return MTLBlendFactorDestinationAlpha;
  case D3DBLEND_INVDESTALPHA:
    return MTLBlendFactorOneMinusDestinationAlpha;
  case D3DBLEND_DESTCOLOR:
    return MTLBlendFactorDestinationColor;
  case D3DBLEND_INVDESTCOLOR:
    return MTLBlendFactorOneMinusDestinationColor;
  case D3DBLEND_SRCALPHASAT:
    return MTLBlendFactorSourceAlphaSaturated;
  default:
    return MTLBlendFactorOne;
  }
}

// ─────────────────────────────────────────────────────
//  Stage 6: D3DCULL → MTLCullMode mapping
//  DX8 uses CW/CCW winding opposite to Metal
// ─────────────────────────────────────────────────────
static MTLCullMode MapD3DCullToMTL(DWORD cull) {
  switch (cull) {
  case D3DCULL_NONE:
    return MTLCullModeNone;
  case D3DCULL_CW:
    return MTLCullModeFront; // DX8 CW = Metal Front
  case D3DCULL_CCW:
    return MTLCullModeBack;
  default:
    return MTLCullModeBack; // DX8 default is CCW
  }
}

// ─────────────────────────────────────────────────────
//  Stage 6: Build 64-bit PSO cache key
//  Layout:  [FVF 20 bits | blendEn 1 | srcBlend 4 | dstBlend 4 | cwMask 4 |
//            srcAlpha 4 | dstAlpha 4 | unused 23]
// ─────────────────────────────────────────────────────
uint64_t MetalDevice8::BuildPSOKey(DWORD fvf) {
  DWORD blendEn = m_RenderStates[D3DRS_ALPHABLENDENABLE] ? 1 : 0;
  DWORD srcBlend = m_RenderStates[D3DRS_SRCBLEND] & 0xF;
  DWORD dstBlend = m_RenderStates[D3DRS_DESTBLEND] & 0xF;
  DWORD cwMask = m_RenderStates[D3DRS_COLORWRITEENABLE] & 0xF;
  if (cwMask == 0)
    cwMask = 0xF; // default: write all channels
  // For separate alpha blend (DX8 doesn't have it, use same)
  DWORD srcAlpha = srcBlend;
  DWORD dstAlpha = dstBlend;

  uint64_t key = 0;
  key |= (uint64_t)(fvf & 0xFFFFF);  // bits 0-19: FVF
  key |= (uint64_t)(blendEn) << 20;  // bit 20: blend enable
  key |= (uint64_t)(srcBlend) << 21; // bits 21-24: src blend
  key |= (uint64_t)(dstBlend) << 25; // bits 25-28: dst blend
  key |= (uint64_t)(cwMask) << 29;   // bits 29-32: color write
  key |= (uint64_t)(srcAlpha) << 33; // bits 33-36: src alpha
  key |= (uint64_t)(dstAlpha) << 37; // bits 37-40: dst alpha
  return key;
}

// ─────────────────────────────────────────────────────
//  Stage 6: Apply per-draw encoder state (cull, depth)
// ─────────────────────────────────────────────────────
void MetalDevice8::ApplyPerDrawState() {
  if (!m_CurrentEncoder)
    return;

  // Cull Mode
  DWORD cullMode = m_RenderStates[D3DRS_CULLMODE];
  [MTL_ENCODER setCullMode:MapD3DCullToMTL(cullMode)];

  // Front face winding: DX8 default is CW, Metal default is CW
  [MTL_ENCODER setFrontFacingWinding:MTLWindingClockwise];

  // Depth/Stencil
  if (m_DepthTexture) {
    void *dss = GetDepthStencilState();
    if (dss) {
      [MTL_ENCODER setDepthStencilState:(__bridge id<MTLDepthStencilState>)dss];
      // Stencil reference value (separate from DSS in Metal)
      if (m_RenderStates[D3DRS_STENCILENABLE]) {
        [MTL_ENCODER setStencilReferenceValue:
                         (uint32_t)(m_RenderStates[D3DRS_STENCILREF] & 0xFF)];
      }
    }
  }
}

// ─────────────────────────────────────────────────────
//  Stage 7: Get or Create MTLSamplerState for a texture stage
// ─────────────────────────────────────────────────────
static MTLSamplerAddressMode MapD3DAddressToMTL(DWORD addr) {
  switch (addr) {
  case D3DTADDRESS_WRAP:
    return MTLSamplerAddressModeRepeat;
  case D3DTADDRESS_CLAMP:
    return MTLSamplerAddressModeClampToEdge;
  case D3DTADDRESS_MIRROR:
    return MTLSamplerAddressModeMirrorRepeat;
  case D3DTADDRESS_BORDER:
    return MTLSamplerAddressModeClampToZero;
  default:
    return MTLSamplerAddressModeRepeat;
  }
}

static MTLSamplerMinMagFilter MapD3DFilterToMTL(DWORD filter) {
  switch (filter) {
  case D3DTEXF_POINT:
    return MTLSamplerMinMagFilterNearest;
  case D3DTEXF_LINEAR:
  case D3DTEXF_ANISOTROPIC:
  case D3DTEXF_FLATCUBIC:
  case D3DTEXF_GAUSSIANCUBIC:
    return MTLSamplerMinMagFilterLinear;
  default:
    return MTLSamplerMinMagFilterLinear;
  }
}

static MTLSamplerMipFilter MapD3DMipFilterToMTL(DWORD filter) {
  switch (filter) {
  case D3DTEXF_NONE:
    return MTLSamplerMipFilterNotMipmapped;
  case D3DTEXF_POINT:
    return MTLSamplerMipFilterNearest;
  case D3DTEXF_LINEAR:
    return MTLSamplerMipFilterLinear;
  default:
    return MTLSamplerMipFilterNotMipmapped;
  }
}

void *MetalDevice8::GetSamplerState(DWORD stage) {
  if (stage >= MAX_TEXTURE_STAGES)
    return nullptr;

  DWORD addrU = m_TextureStageStates[stage][D3DTSS_ADDRESSU];
  DWORD addrV = m_TextureStageStates[stage][D3DTSS_ADDRESSV];
  DWORD magF = m_TextureStageStates[stage][D3DTSS_MAGFILTER];
  DWORD minF = m_TextureStageStates[stage][D3DTSS_MINFILTER];
  DWORD mipF = m_TextureStageStates[stage][D3DTSS_MIPFILTER];

  // Build key: addrU(3) | addrV(3) | mag(3) | min(3) | mip(3) = 15 bits
  uint32_t key = (addrU & 0x7) | ((addrV & 0x7) << 3) | ((magF & 0x7) << 6) |
                 ((minF & 0x7) << 9) | ((mipF & 0x7) << 12);

  auto it = m_SamplerStateCache.find(key);
  if (it != m_SamplerStateCache.end())
    return it->second;

  MTLSamplerDescriptor *sd = [[MTLSamplerDescriptor alloc] init];
  sd.sAddressMode = MapD3DAddressToMTL(addrU);
  sd.tAddressMode = MapD3DAddressToMTL(addrV);
  sd.magFilter = MapD3DFilterToMTL(magF);
  sd.minFilter = MapD3DFilterToMTL(minF);
  sd.mipFilter = MapD3DMipFilterToMTL(mipF);

  id<MTLSamplerState> sampler = [MTL_DEVICE newSamplerStateWithDescriptor:sd];
  if (sampler) {
    m_SamplerStateCache[key] = (__bridge_retained void *)sampler;
    return (__bridge void *)sampler;
  }
  return nullptr;
}

// ─────────────────────────────────────────────────────
//  IUnknown
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::QueryInterface(REFIID riid, void **ppvObj) {
  if (ppvObj)
    *ppvObj = nullptr;
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MetalDevice8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalDevice8::Release() {
  ULONG r = --m_RefCount;
  if (r == 0) {
    delete this;
    return 0;
  }
  return r;
}

// ─────────────────────────────────────────────────────
//  Device Status
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::TestCooperativeLevel() { return D3D_OK; }
STDMETHODIMP_(UINT) MetalDevice8::GetAvailableTextureMem() {
  return 512 * 1024 * 1024;
}
STDMETHODIMP MetalDevice8::ResourceManagerDiscardBytes(DWORD Bytes) {
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetAdapterIdentifier(UINT a, DWORD f,
                                                D3DADAPTER_IDENTIFIER8 *i) {
  if (i)
    memset(i, 0, sizeof(*i));
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetDeviceCaps(D3DCAPS8 *pCaps) {
  if (!pCaps)
    return E_POINTER;
  memset(pCaps, 0, sizeof(*pCaps));
  pCaps->DeviceType = D3DDEVTYPE_HAL;
  pCaps->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
  pCaps->MaxSimultaneousTextures = 8;
  pCaps->MaxTextureBlendStages = 8;
  pCaps->VertexShaderVersion = 0x0101;
  pCaps->PixelShaderVersion = 0x0101;
  pCaps->MaxPrimitiveCount = 0xFFFFFF;
  pCaps->MaxVertexIndex = 0xFFFFFF;
  pCaps->MaxStreams = 8;
  pCaps->MaxActiveLights = 4;
  pCaps->MaxTextureWidth = 4096;
  pCaps->MaxTextureHeight = 4096;
  pCaps->RasterCaps =
      D3DPRASTERCAPS_FOGRANGE | 0x00000100 | 0x00000200 | D3DPRASTERCAPS_ZBIAS;
  pCaps->TextureCaps = 0x00000001 | 0x00000002 | 0x00000004;
  pCaps->TextureOpCaps =
      D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
      D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_ADD |
      D3DTEXOPCAPS_BLENDDIFFUSEALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHA;
  pCaps->PrimitiveMiscCaps = D3DPMISCCAPS_COLORWRITEENABLE;
  pCaps->Caps2 = D3DCAPS2_FULLSCREENGAMMA;
  pCaps->SrcBlendCaps = 0x1FFF;
  pCaps->DestBlendCaps = 0x1FFF;
  pCaps->ZCmpCaps = 0xFF;
  pCaps->AlphaCmpCaps = 0xFF;
  pCaps->StencilCaps = 0xFF;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetDisplayMode(D3DDISPLAYMODE *pMode) {
  if (!pMode)
    return E_POINTER;
  pMode->Width = (UINT)m_ScreenWidth;
  pMode->Height = (UINT)m_ScreenHeight;
  pMode->RefreshRate = 60;
  pMode->Format = D3DFMT_A8R8G8B8;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Swap Chain / Present
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *p,
                                                     IDirect3DSwapChain8 **s) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalDevice8::Reset(D3DPRESENT_PARAMETERS *p) { return D3D_OK; }

STDMETHODIMP MetalDevice8::Present(const void *s, const void *d, HWND w,
                                   const void *r) {
  static int presentCount = 0;
  if (presentCount++ % 120 == 0) {
    fprintf(stderr, "DEBUG Present #%d: encoder=%p drawable=%p cmdBuf=%p\n",
            presentCount, m_CurrentEncoder, m_CurrentDrawable,
            m_CurrentCommandBuffer);
    fflush(stderr);
  }
  if (m_CurrentEncoder) {
    [MTL_ENCODER endEncoding];
    CLEAR_MTL(CurrentEncoder);
  }
  if (m_CurrentDrawable && m_CurrentCommandBuffer) {
    [MTL_CMD_BUF presentDrawable:MTL_DRAWABLE];
  }
  if (m_CurrentCommandBuffer) {
    [MTL_CMD_BUF commit];
    CLEAR_MTL(CurrentCommandBuffer);
  }
  CLEAR_MTL(CurrentDrawable);
  m_InScene = false;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetBackBuffer(UINT i, D3DBACKBUFFER_TYPE t,
                                         IDirect3DSurface8 **b) {
  if (!b)
    return E_POINTER;
  if (m_DefaultRTSurface) {
    m_DefaultRTSurface->AddRef();
    *b = m_DefaultRTSurface;
    return D3D_OK;
  }
  *b = nullptr;
  return E_NOTIMPL;
}

// ─────────────────────────────────────────────────────
//  Gamma
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetGammaRamp(DWORD f, const D3DGAMMARAMP *p) {
  return D3D_OK;
}
STDMETHODIMP MetalDevice8::GetGammaRamp(D3DGAMMARAMP *p) { return D3D_OK; }

// ─────────────────────────────────────────────────────
//  Cursor — no-ops (macOS uses NSCursor natively)
// ─────────────────────────────────────────────────────

STDMETHODIMP_(BOOL) MetalDevice8::ShowCursor(BOOL bShow) { return FALSE; }
STDMETHODIMP MetalDevice8::SetCursorProperties(UINT XHotSpot, UINT YHotSpot,
                                                IDirect3DSurface8 *pCursorBitmap) {
  return D3D_OK;
}
STDMETHODIMP_(void) MetalDevice8::SetCursorPosition(int X, int Y, DWORD Flags) {
  // no-op
}

// ─────────────────────────────────────────────────────
//  Resource Creation
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreateTexture(UINT w, UINT h, UINT l, DWORD u,
                                         D3DFORMAT f, D3DPOOL p,
                                         IDirect3DTexture8 **t) {
  if (!t)
    return E_POINTER;
  *t = W3DNEW MetalTexture8(this, w, h, l, u, f, p);
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::CreateVolumeTexture(UINT w, UINT h, UINT d, UINT l,
                                               DWORD u, D3DFORMAT f, D3DPOOL p,
                                               IDirect3DVolumeTexture8 **t) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalDevice8::CreateCubeTexture(UINT s, UINT l, DWORD u,
                                             D3DFORMAT f, D3DPOOL p,
                                             IDirect3DCubeTexture8 **t) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalDevice8::CreateVertexBuffer(UINT Length, DWORD Usage,
                                              DWORD FVF, D3DPOOL Pool,
                                              IDirect3DVertexBuffer8 **ppVB) {
  if (!ppVB)
    return E_POINTER;
  UINT vertexSize = D3DXGetFVFVertexSize(FVF);
  if (vertexSize == 0)
    vertexSize = 32;
  UINT count = Length / vertexSize;
  *ppVB = new MetalVertexBuffer8(FVF, (unsigned short)count, vertexSize);
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::CreateIndexBuffer(UINT Length, DWORD Usage,
                                             D3DFORMAT Format, D3DPOOL Pool,
                                             IDirect3DIndexBuffer8 **ppIB) {
  if (!ppIB)
    return E_POINTER;
  bool is32bit = (Format == D3DFMT_INDEX32);
  UINT count = Length / (is32bit ? 4 : 2);
  *ppIB = new MetalIndexBuffer8(count, is32bit);
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::CreateImageSurface(UINT w, UINT h, D3DFORMAT f,
                                              IDirect3DSurface8 **s) {
  if (!s)
    return E_POINTER;
  *s = W3DNEW MetalSurface8(this, MetalSurface8::kColor, w, h, f);
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Surface / Texture Operations
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CopyRects(IDirect3DSurface8 *src, const void *sr,
                                     UINT c, IDirect3DSurface8 *dst,
                                     const void *dp) {
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::UpdateTexture(IDirect3DBaseTexture8 *s,
                                         IDirect3DBaseTexture8 *d) {
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetFrontBuffer(IDirect3DSurface8 *d) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Render Target
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetRenderTarget(IDirect3DSurface8 *s,
                                           IDirect3DSurface8 *d) {
  return D3D_OK;
}
STDMETHODIMP MetalDevice8::GetRenderTarget(IDirect3DSurface8 **s) {
  if (!s)
    return E_POINTER;
  if (m_DefaultRTSurface) {
    m_DefaultRTSurface->AddRef();
    *s = m_DefaultRTSurface;
    return D3D_OK;
  }
  *s = nullptr;
  return E_NOTIMPL;
}
STDMETHODIMP MetalDevice8::GetDepthStencilSurface(IDirect3DSurface8 **s) {
  if (!s)
    return E_POINTER;
  if (m_DefaultDepthSurface) {
    m_DefaultDepthSurface->AddRef();
    *s = m_DefaultDepthSurface;
    return D3D_OK;
  }
  *s = nullptr;
  return E_NOTIMPL;
}
STDMETHODIMP MetalDevice8::SetDepthStencilSurface(IDirect3DSurface8 *s) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Scene
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::BeginScene() {
  if (m_InScene)
    return D3D_OK;
  m_InScene = true;

  id<MTLCommandBuffer> cmdBuf = [MTL_QUEUE commandBuffer];
  SET_MTL(CurrentCommandBuffer, cmdBuf);

  id<CAMetalDrawable> drawable = [MTL_LAYER nextDrawable];
  static int beginCount = 0;
  if (beginCount++ % 120 == 0) {
    fprintf(stderr,
            "DEBUG BeginScene #%d: layer=%p drawable=%p cmdBuf=%p "
            "layerSize=%.0fx%.0f\n",
            beginCount, MTL_LAYER, drawable, cmdBuf,
            MTL_LAYER ? MTL_LAYER.drawableSize.width : 0,
            MTL_LAYER ? MTL_LAYER.drawableSize.height : 0);
    fflush(stderr);
  }
  if (!drawable) {
    fprintf(stderr,
            "[MetalDevice8] WARNING: No drawable available (layer=%p)\n",
            MTL_LAYER);
    m_InScene = false;
    return E_FAIL;
  }
  SET_MTL(CurrentDrawable, drawable);

  return D3D_OK;
}

STDMETHODIMP MetalDevice8::EndScene() {
  if (!m_InScene)
    return D3D_OK;
  m_InScene = false;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::Clear(DWORD Count, const void *pRects, DWORD Flags,
                                 D3DCOLOR Color, float Z, DWORD Stencil) {
  static int clearCount = 0;
  if (clearCount++ % 120 == 0) {
    float dr = ((Color >> 16) & 0xFF) / 255.0f;
    float dg = ((Color >> 8) & 0xFF) / 255.0f;
    float db = ((Color >> 0) & 0xFF) / 255.0f;
    printf("DEBUG MetalDevice8::Clear #%d flags=0x%x color=0x%08x (r=%.2f "
           "g=%.2f b=%.2f) drawable=%p cmdBuf=%p\n",
           clearCount, (unsigned)Flags, (unsigned)Color, dr, dg, db,
           m_CurrentDrawable, m_CurrentCommandBuffer);
    fflush(stdout);
  }
  // WW3D calls Clear() BEFORE BeginScene(), so auto-start if needed.
  if (!m_CurrentDrawable) {
    HRESULT bshr = BeginScene();
    if (clearCount % 120 == 0) {
      fprintf(stderr, "DEBUG Clear: auto-BeginScene result=0x%x drawable=%p\n",
              (unsigned)bshr, m_CurrentDrawable);
      fflush(stderr);
    }
  }
  if (!m_CurrentDrawable)
    return D3D_OK;

  if (m_CurrentEncoder) {
    [MTL_ENCODER endEncoding];
    CLEAR_MTL(CurrentEncoder);
  }

  MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor renderPassDescriptor];
  rpd.colorAttachments[0].texture = MTL_DRAWABLE.texture;

  if (Flags & D3DCLEAR_TARGET) {
    float a = ((Color >> 24) & 0xFF) / 255.0f;
    float r = ((Color >> 16) & 0xFF) / 255.0f;
    float g = ((Color >> 8) & 0xFF) / 255.0f;
    float b = ((Color >> 0) & 0xFF) / 255.0f;
    rpd.colorAttachments[0].loadAction = MTLLoadActionClear;
    rpd.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, a);
  } else {
    rpd.colorAttachments[0].loadAction = MTLLoadActionLoad;
  }
  rpd.colorAttachments[0].storeAction = MTLStoreActionStore;

  // --- Depth attachment ---
  if (m_DepthTexture) {
    id<MTLTexture> depthTex = (__bridge id<MTLTexture>)m_DepthTexture;
    rpd.depthAttachment.texture = depthTex;
    rpd.depthAttachment.storeAction = MTLStoreActionStore;

    if (Flags & D3DCLEAR_ZBUFFER) {
      rpd.depthAttachment.loadAction = MTLLoadActionClear;
      rpd.depthAttachment.clearDepth = Z; // DX8 typically passes 1.0
    } else {
      rpd.depthAttachment.loadAction = MTLLoadActionLoad;
    }

    // Stencil attachment (same texture for Depth32Float_Stencil8)
    rpd.stencilAttachment.texture = depthTex;
    rpd.stencilAttachment.storeAction = MTLStoreActionStore;
    if (Flags & D3DCLEAR_STENCIL) {
      rpd.stencilAttachment.loadAction = MTLLoadActionClear;
      rpd.stencilAttachment.clearStencil = Stencil;
    } else {
      rpd.stencilAttachment.loadAction = MTLLoadActionLoad;
    }
  }

  id<MTLRenderCommandEncoder> encoder =
      [MTL_CMD_BUF renderCommandEncoderWithDescriptor:rpd];
  [encoder setLabel:@"MetalDevice8 RenderPass"];
  SET_MTL(CurrentEncoder, encoder);

  // --- Apply Depth Stencil State ---
  if (m_DepthTexture) {
    void *dss = GetDepthStencilState();
    if (dss) {
      [encoder setDepthStencilState:(__bridge id<MTLDepthStencilState>)dss];
    }
  }

  MTLViewport vp;
  vp.originX = m_Viewport.X;
  vp.originY = m_Viewport.Y;
  vp.width =
      m_Viewport.Width > 0 ? m_Viewport.Width : MTL_LAYER.drawableSize.width;
  vp.height =
      m_Viewport.Height > 0 ? m_Viewport.Height : MTL_LAYER.drawableSize.height;
  vp.znear = m_Viewport.MinZ;
  vp.zfar = m_Viewport.MaxZ > 0 ? m_Viewport.MaxZ : 1.0;
  [MTL_ENCODER setViewport:vp];

  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Transforms
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetTransform(D3DTRANSFORMSTATETYPE State,
                                        const D3DMATRIX *pMatrix) {
  if (!pMatrix)
    return E_POINTER;
  if ((int)State >= 0 && (int)State < 260) {
    m_Transforms[(int)State] = *pMatrix;
  }
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetTransform(D3DTRANSFORMSTATETYPE State,
                                        D3DMATRIX *pMatrix) {
  if (!pMatrix)
    return E_POINTER;
  if ((int)State >= 0 && (int)State < 260) {
    *pMatrix = m_Transforms[(int)State];
  }
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Viewport
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetViewport(const D3DVIEWPORT8 *pViewport) {
  if (!pViewport)
    return E_POINTER;
  m_Viewport = *pViewport;

  if (m_CurrentEncoder) {
    MTLViewport vp;
    vp.originX = pViewport->X;
    vp.originY = pViewport->Y;
    vp.width = pViewport->Width;
    vp.height = pViewport->Height;
    vp.znear = pViewport->MinZ;
    vp.zfar = pViewport->MaxZ;
    [MTL_ENCODER setViewport:vp];
  }
  return D3D_OK;
}

HRESULT MetalDevice8::GetViewport(D3DVIEWPORT8 *pViewport) {
  if (!pViewport)
    return E_POINTER;
  *pViewport = m_Viewport;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Material / Lighting
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetMaterial(const D3DMATERIAL8 *p) {
  if (!p)
    return E_POINTER;
  m_Material = *p;
  return D3D_OK;
}

HRESULT MetalDevice8::GetMaterial(D3DMATERIAL8 *p) {
  if (!p)
    return E_POINTER;
  *p = m_Material;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetLight(DWORD i, const D3DLIGHT8 *l) {
  if (i < MAX_LIGHTS && l)
    m_Lights[i] = *l;
  return D3D_OK;
}

HRESULT MetalDevice8::GetLight(DWORD i, D3DLIGHT8 *l) {
  if (i < MAX_LIGHTS && l)
    *l = m_Lights[i];
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::LightEnable(DWORD i, BOOL b) {
  if (i < MAX_LIGHTS)
    m_LightEnabled[i] = b;
  return D3D_OK;
}

HRESULT MetalDevice8::GetLightEnable(DWORD i, BOOL *b) {
  if (i < MAX_LIGHTS && b)
    *b = m_LightEnabled[i];
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Clip Planes
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetClipPlane(DWORD i, const float *p) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Render State
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetRenderState(D3DRENDERSTATETYPE State,
                                          DWORD Value) {
  if ((int)State < 256) {
    DWORD old = m_RenderStates[(int)State];
    m_RenderStates[(int)State] = Value;

    // Mark depth/stencil state dirty if relevant render states changed
    if (old != Value) {
      if (State == D3DRS_ZENABLE || State == D3DRS_ZWRITEENABLE ||
          State == D3DRS_ZFUNC || State == D3DRS_STENCILENABLE ||
          State == D3DRS_STENCILFUNC || State == D3DRS_STENCILFAIL ||
          State == D3DRS_STENCILZFAIL || State == D3DRS_STENCILPASS ||
          State == D3DRS_STENCILMASK || State == D3DRS_STENCILWRITEMASK) {
        m_DepthStateDirty = true;
      }
    }
  }
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetRenderState(D3DRENDERSTATETYPE State,
                                          DWORD *pValue) {
  if (!pValue)
    return E_POINTER;
  if ((int)State < 256)
    *pValue = m_RenderStates[(int)State];
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Textures / Texture Stage States
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetTexture(DWORD Stage,
                                      IDirect3DBaseTexture8 *pTexture) {
  if (Stage < MAX_TEXTURE_STAGES)
    m_Textures[Stage] = pTexture;
  return D3D_OK;
}

HRESULT MetalDevice8::GetTexture(DWORD Stage,
                                 IDirect3DBaseTexture8 **ppTexture) {
  if (!ppTexture)
    return E_POINTER;
  if (Stage < MAX_TEXTURE_STAGES) {
    *ppTexture = m_Textures[Stage];
    if (*ppTexture)
      (*ppTexture)->AddRef();
  } else {
    *ppTexture = nullptr;
  }
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetTextureStageState(DWORD Stage,
                                                D3DTEXTURESTAGESTATETYPE Type,
                                                DWORD Value) {
  if (Stage < MAX_TEXTURE_STAGES && (int)Type < 32) {
    m_TextureStageStates[Stage][(int)Type] = Value;
  }
  return D3D_OK;
}

HRESULT MetalDevice8::GetTextureStageState(DWORD Stage,
                                           D3DTEXTURESTAGESTATETYPE Type,
                                           DWORD *pValue) {
  if (!pValue)
    return E_POINTER;
  if (Stage < MAX_TEXTURE_STAGES && (int)Type < 32) {
    *pValue = m_TextureStageStates[Stage][(int)Type];
  }
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Validate
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::ValidateDevice(DWORD *pNumPasses) {
  if (pNumPasses)
    *pNumPasses = 1;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Drawing — Stage 0 stubs
// ─────────────────────────────────────────────────────

// Helper: Get or Create PSO for FVF + current blend state
void *MetalDevice8::GetPSO(DWORD fvf) {
  // 1. Build key from FVF + blend state
  uint64_t key = BuildPSOKey(fvf);
  auto it = m_PsoCache.find(key);
  if (it != m_PsoCache.end()) {
    return it->second;
  }

  // 2. Create Descriptor
  MTLRenderPipelineDescriptor *pd = [[MTLRenderPipelineDescriptor alloc] init];
  pd.vertexFunction = (__bridge id<MTLFunction>)m_FunctionVertex;
  pd.fragmentFunction = (__bridge id<MTLFunction>)m_FunctionFragment;
  pd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

  // Depth attachment pixel format must match the depth texture
  if (m_DepthTexture) {
    pd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    pd.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
  }

  // --- Stage 6: Dynamic Blend State ---
  DWORD blendEn = m_RenderStates[D3DRS_ALPHABLENDENABLE];
  DWORD srcBlend = m_RenderStates[D3DRS_SRCBLEND];
  DWORD dstBlend = m_RenderStates[D3DRS_DESTBLEND];
  DWORD cwMask = m_RenderStates[D3DRS_COLORWRITEENABLE];
  if (cwMask == 0)
    cwMask = 0xF; // default: write all

  pd.colorAttachments[0].blendingEnabled = (blendEn != 0) ? YES : NO;
  pd.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
  pd.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
  pd.colorAttachments[0].sourceRGBBlendFactor = MapD3DBlendToMTL(srcBlend);
  pd.colorAttachments[0].sourceAlphaBlendFactor = MapD3DBlendToMTL(srcBlend);
  pd.colorAttachments[0].destinationRGBBlendFactor = MapD3DBlendToMTL(dstBlend);
  pd.colorAttachments[0].destinationAlphaBlendFactor =
      MapD3DBlendToMTL(dstBlend);

  // Color write mask: D3DCOLORWRITEENABLE_RED=1, GREEN=2, BLUE=4, ALPHA=8
  MTLColorWriteMask mtlMask = MTLColorWriteMaskNone;
  if (cwMask & 1)
    mtlMask |= MTLColorWriteMaskRed;
  if (cwMask & 2)
    mtlMask |= MTLColorWriteMaskGreen;
  if (cwMask & 4)
    mtlMask |= MTLColorWriteMaskBlue;
  if (cwMask & 8)
    mtlMask |= MTLColorWriteMaskAlpha;
  pd.colorAttachments[0].writeMask = mtlMask;

  // 3. Define Vertex Layout based on FVF
  MTLVertexDescriptor *vd = [MTLVertexDescriptor vertexDescriptor];

  // Stride tracking
  NSUInteger currentOffset = 0;

  // Track which attributes are provided by the FVF
  bool hasPosition = false;
  bool hasDiffuse = false;
  bool hasTexCoord0 = false;
  bool hasNormal = false;
  bool hasSpecular = false;
  bool hasTexCoord1 = false;

  // --- Position ---
  if (fvf & D3DFVF_XYZRHW) {
    vd.attributes[0].format = MTLVertexFormatFloat4;
    vd.attributes[0].offset = currentOffset;
    vd.attributes[0].bufferIndex = 0;
    currentOffset += 16;
    hasPosition = true;
  } else if (fvf & D3DFVF_XYZ) {
    vd.attributes[0].format = MTLVertexFormatFloat3;
    vd.attributes[0].offset = currentOffset;
    vd.attributes[0].bufferIndex = 0;
    currentOffset += 12;
    hasPosition = true;
  }

  // --- Normal --- mapped to attribute(3) for lighting
  if (fvf & D3DFVF_NORMAL) {
    vd.attributes[3].format = MTLVertexFormatFloat3;
    vd.attributes[3].offset = currentOffset;
    vd.attributes[3].bufferIndex = 0;
    currentOffset += 12;
    hasNormal = true;
  }

  // --- Diffuse Color ---
  // D3DCOLOR is 0xAARRGGBB → bytes [BB,GG,RR,AA] in little-endian.
  // MTLVertexFormatUChar4Normalized_BGRA interprets [B,G,R,A] → shader
  // (R,G,B,A).
  if (fvf & D3DFVF_DIFFUSE) {
    vd.attributes[1].format = MTLVertexFormatUChar4Normalized_BGRA;
    vd.attributes[1].offset = currentOffset;
    vd.attributes[1].bufferIndex = 0;
    currentOffset += 4;
    hasDiffuse = true;
  }

  // --- Specular Color --- mapped to attribute(4)
  // Same D3DCOLOR byte order as diffuse.
  if (fvf & 0x080) { // D3DFVF_SPECULAR
    vd.attributes[4].format = MTLVertexFormatUChar4Normalized_BGRA;
    vd.attributes[4].offset = currentOffset;
    vd.attributes[4].bufferIndex = 0;
    currentOffset += 4;
    hasSpecular = true;
  }

  // --- Texture Coordinates ---
  // D3DFVF_TEX* is a counted field (bits 8-11), not bitmask flags
  UINT texCount = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  if (texCount >= 1) {
    vd.attributes[2].format = MTLVertexFormatFloat2; // texCoord0 → attribute(2)
    vd.attributes[2].offset = currentOffset;
    vd.attributes[2].bufferIndex = 0;
    currentOffset += 8;
    hasTexCoord0 = true;
  }
  if (texCount >= 2) {
    vd.attributes[5].format = MTLVertexFormatFloat2; // texCoord1 → attribute(5)
    vd.attributes[5].offset = currentOffset;
    vd.attributes[5].bufferIndex = 0;
    currentOffset += 8;
    hasTexCoord1 = true;
  }
  vd.layouts[0].stride = currentOffset;
  vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

  // --- Provide defaults for missing attributes via a constant zero buffer ---
  // Metal requires all shader-declared attributes to be present in the vertex
  // descriptor. For attributes not in the FVF, we bind them to a separate
  // buffer (index 30) filled with zeros, using stepFunction=Constant so all
  // vertices read the same default (zero) values.
  bool needDefaultBuffer = !hasPosition || !hasDiffuse || !hasTexCoord0 ||
                           !hasNormal || !hasSpecular || !hasTexCoord1;
  if (needDefaultBuffer) {
    // Layout for constant buffer: all missing attributes overlap at offset 0
    // Buffer contains at least 16 bytes of zeros (enough for float4)
    vd.layouts[30].stride = 16;
    vd.layouts[30].stepFunction = MTLVertexStepFunctionConstant;
    vd.layouts[30].stepRate = 0;

    if (!hasPosition) {
      vd.attributes[0].format = MTLVertexFormatFloat3;
      vd.attributes[0].offset = 0;
      vd.attributes[0].bufferIndex = 30;
    }
    if (!hasDiffuse) {
      vd.attributes[1].format = MTLVertexFormatUChar4Normalized_BGRA;
      vd.attributes[1].offset = 0;
      vd.attributes[1].bufferIndex = 30;
    }
    if (!hasTexCoord0) {
      vd.attributes[2].format = MTLVertexFormatFloat2;
      vd.attributes[2].offset = 0;
      vd.attributes[2].bufferIndex = 30;
    }
    if (!hasNormal) {
      vd.attributes[3].format = MTLVertexFormatFloat3;
      vd.attributes[3].offset = 0;
      vd.attributes[3].bufferIndex = 30;
    }
    if (!hasSpecular) {
      vd.attributes[4].format = MTLVertexFormatUChar4Normalized_BGRA;
      vd.attributes[4].offset = 0;
      vd.attributes[4].bufferIndex = 30;
    }
    if (!hasTexCoord1) {
      vd.attributes[5].format = MTLVertexFormatFloat2;
      vd.attributes[5].offset = 0;
      vd.attributes[5].bufferIndex = 30;
    }
  }

  pd.vertexDescriptor = vd;

  NSError *err = nil;
  id<MTLRenderPipelineState> pso = [(__bridge id<MTLDevice>)m_Device
      newRenderPipelineStateWithDescriptor:pd
                                     error:&err];
  if (!pso) {
    fprintf(stderr,
            "[MetalDevice8] Error creating PSO for FVF %x key %llx: %s\n", fvf,
            key, [[err localizedDescription] UTF8String]);
    return nil;
  }

  m_PsoCache[key] = (__bridge_retained void *)pso;
  return (__bridge void *)pso;
}

// ─────────────────────────────────────────────────────
//  Drawing
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::DrawPrimitive(DWORD pt, UINT sv, UINT pc) {
  if (!m_CurrentEncoder || !m_StreamSource)
    return D3D_OK;

  // 1. Get FVF and PSO
  DWORD fvf = GetBufferFVF(m_StreamSource);
  id<MTLRenderPipelineState> pso =
      (__bridge id<MTLRenderPipelineState>)GetPSO(fvf);
  if (!pso)
    return D3D_OK;

  // 2. Set State
  [MTL_ENCODER setRenderPipelineState:pso];

  // 2b. Apply per-draw state (cull mode, depth/stencil)
  ApplyPerDrawState();

  // 3. Bind Vertex Buffer
  MetalVertexBuffer8 *vb = (MetalVertexBuffer8 *)m_StreamSource;
  [MTL_ENCODER setVertexBuffer:(__bridge id<MTLBuffer>)vb->GetMTLBuffer()
                        offset:0
                       atIndex:0];

  // 3b. Bind zero buffer for missing vertex attributes (FVF defaults)
  if (m_ZeroBuffer) {
    [MTL_ENCODER setVertexBuffer:(__bridge id<MTLBuffer>)m_ZeroBuffer
                          offset:0
                         atIndex:30];
  }

  // 4. Bind Vertex Uniforms (buffer 1)
  MetalUniforms u;
  memcpy(&u.world, &m_Transforms[D3DTS_WORLD], 64);
  memcpy(&u.view, &m_Transforms[D3DTS_VIEW], 64);
  memcpy(&u.projection, &m_Transforms[D3DTS_PROJECTION], 64);
  u.screenSize.x = m_ScreenWidth;
  u.screenSize.y = m_ScreenHeight;
  u.useProjection = (fvf & D3DFVF_XYZRHW) ? 2 : 1;
  u.shaderSettings = 0; // legacy (unused by new shader)

  [MTL_ENCODER setVertexBytes:&u length:sizeof(u) atIndex:1];
  [MTL_ENCODER setFragmentBytes:&u length:sizeof(u) atIndex:1];

  // 5. Build Fragment Uniforms (buffer 2) — Stage 7 TSS
  FragmentUniforms fu;
  memset(&fu, 0, sizeof(fu));

  // Copy TSS config for stages 0 and 1
  for (int s = 0; s < 2; s++) {
    fu.stages[s].colorOp = m_TextureStageStates[s][D3DTSS_COLOROP];
    fu.stages[s].colorArg1 = m_TextureStageStates[s][D3DTSS_COLORARG1];
    fu.stages[s].colorArg2 = m_TextureStageStates[s][D3DTSS_COLORARG2];
    fu.stages[s].alphaOp = m_TextureStageStates[s][D3DTSS_ALPHAOP];
    fu.stages[s].alphaArg1 = m_TextureStageStates[s][D3DTSS_ALPHAARG1];
    fu.stages[s].alphaArg2 = m_TextureStageStates[s][D3DTSS_ALPHAARG2];
  }

  // Texture Factor (ARGB DWORD -> float4 RGBA)
  DWORD tf = m_RenderStates[D3DRS_TEXTUREFACTOR];
  fu.textureFactor.x = ((tf >> 16) & 0xFF) / 255.0f; // R
  fu.textureFactor.y = ((tf >> 8) & 0xFF) / 255.0f;  // G
  fu.textureFactor.z = ((tf >> 0) & 0xFF) / 255.0f;  // B
  fu.textureFactor.w = ((tf >> 24) & 0xFF) / 255.0f; // A

  // Alpha Test
  fu.alphaTestEnable = m_RenderStates[D3DRS_ALPHATESTENABLE];
  fu.alphaFunc = m_RenderStates[D3DRS_ALPHAFUNC];
  fu.alphaRef = m_RenderStates[D3DRS_ALPHAREF] / 255.0f;

  // Fog — Stage 9
  {
    DWORD fogEnable = m_RenderStates[D3DRS_FOGENABLE];
    if (fogEnable) {
      // Determine fog mode: prefer table fog, fall back to vertex fog
      uint32_t mode = m_RenderStates[D3DRS_FOGTABLEMODE];
      if (mode == D3DFOG_NONE) {
        mode = m_RenderStates[D3DRS_FOGVERTEXMODE];
      }
      fu.fogMode = mode; // 0=NONE, 1=EXP, 2=EXP2, 3=LINEAR
    } else {
      fu.fogMode = 0;
    }
    // Fog color (ARGB DWORD -> float4 RGBA)
    DWORD fc = m_RenderStates[D3DRS_FOGCOLOR];
    fu.fogColor =
        simd::float4{((fc >> 16) & 0xFF) / 255.0f, ((fc >> 8) & 0xFF) / 255.0f,
                     ((fc >> 0) & 0xFF) / 255.0f, ((fc >> 24) & 0xFF) / 255.0f};
    // Fog start/end/density are stored as DWORD bit-casts of float
    memcpy(&fu.fogStart, &m_RenderStates[D3DRS_FOGSTART], 4);
    memcpy(&fu.fogEnd, &m_RenderStates[D3DRS_FOGEND], 4);
    memcpy(&fu.fogDensity, &m_RenderStates[D3DRS_FOGDENSITY], 4);
  }

  // Texture presence flags
  fu.hasTexture0 = (m_Textures[0] != nullptr) ? 1 : 0;
  fu.hasTexture1 = (m_Textures[1] != nullptr) ? 1 : 0;

  [MTL_ENCODER setFragmentBytes:&fu length:sizeof(fu) atIndex:2];

  // 5b. Build Lighting Uniforms (buffer 3) — Stage 8 Lighting
  LightingUniforms lu;
  memset(&lu, 0, sizeof(lu));

  for (int i = 0; i < MAX_LIGHTS; i++) {
    lu.lights[i].enabled = m_LightEnabled[i] ? 1 : 0;
    if (m_LightEnabled[i]) {
      const D3DLIGHT8 &l = m_Lights[i];
      lu.lights[i].type = (uint32_t)l.Type;
      lu.lights[i].diffuse =
          simd::float4{l.Diffuse.r, l.Diffuse.g, l.Diffuse.b, l.Diffuse.a};
      lu.lights[i].ambient =
          simd::float4{l.Ambient.r, l.Ambient.g, l.Ambient.b, l.Ambient.a};
      lu.lights[i].specular =
          simd::float4{l.Specular.r, l.Specular.g, l.Specular.b, l.Specular.a};
      lu.lights[i].position =
          simd::float3{l.Position.x, l.Position.y, l.Position.z};
      lu.lights[i].direction =
          simd::float3{l.Direction.x, l.Direction.y, l.Direction.z};
      lu.lights[i].range = l.Range;
      lu.lights[i].falloff = l.Falloff;
      lu.lights[i].attenuation0 = l.Attenuation0;
      lu.lights[i].attenuation1 = l.Attenuation1;
      lu.lights[i].attenuation2 = l.Attenuation2;
      lu.lights[i].theta = l.Theta;
      lu.lights[i].phi = l.Phi;
    }
  }

  // Material
  lu.materialDiffuse = simd::float4{m_Material.Diffuse.r, m_Material.Diffuse.g,
                                    m_Material.Diffuse.b, m_Material.Diffuse.a};
  lu.materialAmbient = simd::float4{m_Material.Ambient.r, m_Material.Ambient.g,
                                    m_Material.Ambient.b, m_Material.Ambient.a};
  lu.materialSpecular =
      simd::float4{m_Material.Specular.r, m_Material.Specular.g,
                   m_Material.Specular.b, m_Material.Specular.a};
  lu.materialEmissive =
      simd::float4{m_Material.Emissive.r, m_Material.Emissive.g,
                   m_Material.Emissive.b, m_Material.Emissive.a};
  lu.materialPower = m_Material.Power;

  // Global ambient: D3DRS_AMBIENT is an ARGB DWORD
  DWORD ga = m_RenderStates[D3DRS_AMBIENT];
  lu.globalAmbient =
      simd::float4{((ga >> 16) & 0xFF) / 255.0f, ((ga >> 8) & 0xFF) / 255.0f,
                   ((ga >> 0) & 0xFF) / 255.0f, ((ga >> 24) & 0xFF) / 255.0f};

  // Lighting enable
  lu.lightingEnabled = m_RenderStates[D3DRS_LIGHTING];
  lu.diffuseSource = m_RenderStates[D3DRS_DIFFUSEMATERIALSOURCE];
  lu.ambientSource = m_RenderStates[D3DRS_AMBIENTMATERIALSOURCE];
  lu.specularSource = m_RenderStates[D3DRS_SPECULARMATERIALSOURCE];
  lu.emissiveSource = m_RenderStates[D3DRS_EMISSIVEMATERIALSOURCE];
  lu.hasNormals = (fvf & D3DFVF_NORMAL) ? 1 : 0;

  // Fog parameters for vertex fog computation (Stage 9)
  {
    DWORD fogEnable = m_RenderStates[D3DRS_FOGENABLE];
    if (fogEnable) {
      uint32_t mode = m_RenderStates[D3DRS_FOGTABLEMODE];
      if (mode == D3DFOG_NONE) {
        mode = m_RenderStates[D3DRS_FOGVERTEXMODE];
      }
      lu.fogMode = mode;
    } else {
      lu.fogMode = 0;
    }
    memcpy(&lu.fogStart, &m_RenderStates[D3DRS_FOGSTART], 4);
    memcpy(&lu.fogEnd, &m_RenderStates[D3DRS_FOGEND], 4);
    memcpy(&lu.fogDensity, &m_RenderStates[D3DRS_FOGDENSITY], 4);
  }

  [MTL_ENCODER setVertexBytes:&lu length:sizeof(lu) atIndex:3];

  // 6. Bind Textures and Samplers
  for (int s = 0; s < 2; s++) {
    if (m_Textures[s]) {
      MetalTexture8 *tex = (MetalTexture8 *)m_Textures[s];
      id<MTLTexture> mtlTex = tex->GetMTLTexture();
      if (mtlTex) {
        [MTL_ENCODER setFragmentTexture:mtlTex atIndex:s];
      }
    }
    // Always bind a sampler (even if no texture) to avoid Metal validation
    // errors
    void *samplerState = GetSamplerState(s);
    if (samplerState) {
      [MTL_ENCODER
          setFragmentSamplerState:(__bridge id<MTLSamplerState>)samplerState
                          atIndex:s];
    }
  }

  // 6. Draw
  MTLPrimitiveType mtlPt = MTLPrimitiveTypeTriangle;
  if (pt == D3DPT_TRIANGLELIST)
    mtlPt = MTLPrimitiveTypeTriangle;
  else if (pt == D3DPT_TRIANGLESTRIP)
    mtlPt = MTLPrimitiveTypeTriangleStrip;
  else if (pt == D3DPT_LINELIST)
    mtlPt = MTLPrimitiveTypeLine;
  // ... others

  // Primitive Count to Vertex Count
  UINT vertexCount = 0;
  if (pt == D3DPT_TRIANGLELIST)
    vertexCount = pc * 3;
  else if (pt == D3DPT_TRIANGLESTRIP)
    vertexCount = pc + 2;
  else if (pt == D3DPT_LINELIST)
    vertexCount = pc * 2;

  [MTL_ENCODER drawPrimitives:mtlPt vertexStart:sv vertexCount:vertexCount];
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::DrawIndexedPrimitive(DWORD pt, UINT mi, UINT nv,
                                                UINT si, UINT pc) {
  static int dipCount = 0;
  dipCount++;
  if (dipCount <= 5)
    fprintf(stderr,
            "DEBUG DIP[%d]: ENTER encoder=%p src=%p ib=%p pt=%u pc=%u\n",
            dipCount, m_CurrentEncoder, (void *)m_StreamSource,
            (void *)m_IndexBuffer, (unsigned)pt, pc);
  if (!m_CurrentEncoder || !m_StreamSource || !m_IndexBuffer) {
    return D3D_OK;
  }

  // 1. Get FVF and PSO
  DWORD fvf = GetBufferFVF(m_StreamSource);
  id<MTLRenderPipelineState> pso =
      (__bridge id<MTLRenderPipelineState>)GetPSO(fvf);
  if (!pso) {
    if (dipCount <= 5)
      fprintf(stderr, "DEBUG DIP[%d]: NO PSO for fvf=0x%x\n", dipCount,
              (unsigned)fvf);
    return D3D_OK;
  }

  // 2. Set State
  [MTL_ENCODER setRenderPipelineState:pso];

  // 2b. Apply per-draw state (cull mode, depth/stencil)
  ApplyPerDrawState();

  // 3. Bind VB
  MetalVertexBuffer8 *vb = (MetalVertexBuffer8 *)m_StreamSource;
  [MTL_ENCODER setVertexBuffer:(__bridge id<MTLBuffer>)vb->GetMTLBuffer()
                        offset:0
                       atIndex:0];

  // 3b. Bind zero buffer for missing vertex attributes (FVF defaults)
  if (m_ZeroBuffer) {
    [MTL_ENCODER setVertexBuffer:(__bridge id<MTLBuffer>)m_ZeroBuffer
                          offset:0
                         atIndex:30];
  }

  // 4. Bind Vertex Uniforms (buffer 1)
  MetalUniforms u;
  memcpy(&u.world, &m_Transforms[D3DTS_WORLD], 64);
  memcpy(&u.view, &m_Transforms[D3DTS_VIEW], 64);
  memcpy(&u.projection, &m_Transforms[D3DTS_PROJECTION], 64);
  u.screenSize.x = m_ScreenWidth;
  u.screenSize.y = m_ScreenHeight;
  u.useProjection = (fvf & D3DFVF_XYZRHW) ? 2 : 1;
  u.shaderSettings = 0; // legacy
  [MTL_ENCODER setVertexBytes:&u length:sizeof(u) atIndex:1];
  [MTL_ENCODER setFragmentBytes:&u length:sizeof(u) atIndex:1];

  // 5. Build Fragment Uniforms (buffer 2) — Stage 7 TSS
  FragmentUniforms fu;
  memset(&fu, 0, sizeof(fu));
  for (int s = 0; s < 2; s++) {
    fu.stages[s].colorOp = m_TextureStageStates[s][D3DTSS_COLOROP];
    fu.stages[s].colorArg1 = m_TextureStageStates[s][D3DTSS_COLORARG1];
    fu.stages[s].colorArg2 = m_TextureStageStates[s][D3DTSS_COLORARG2];
    fu.stages[s].alphaOp = m_TextureStageStates[s][D3DTSS_ALPHAOP];
    fu.stages[s].alphaArg1 = m_TextureStageStates[s][D3DTSS_ALPHAARG1];
    fu.stages[s].alphaArg2 = m_TextureStageStates[s][D3DTSS_ALPHAARG2];
  }
  DWORD tf = m_RenderStates[D3DRS_TEXTUREFACTOR];
  fu.textureFactor.x = ((tf >> 16) & 0xFF) / 255.0f;
  fu.textureFactor.y = ((tf >> 8) & 0xFF) / 255.0f;
  fu.textureFactor.z = ((tf >> 0) & 0xFF) / 255.0f;
  fu.textureFactor.w = ((tf >> 24) & 0xFF) / 255.0f;
  fu.alphaTestEnable = m_RenderStates[D3DRS_ALPHATESTENABLE];
  fu.alphaFunc = m_RenderStates[D3DRS_ALPHAFUNC];
  fu.alphaRef = m_RenderStates[D3DRS_ALPHAREF] / 255.0f;
  // Fog — Stage 9
  {
    DWORD fogEnable = m_RenderStates[D3DRS_FOGENABLE];
    if (fogEnable) {
      uint32_t mode = m_RenderStates[D3DRS_FOGTABLEMODE];
      if (mode == D3DFOG_NONE) {
        mode = m_RenderStates[D3DRS_FOGVERTEXMODE];
      }
      fu.fogMode = mode;
    } else {
      fu.fogMode = 0;
    }
    DWORD fc = m_RenderStates[D3DRS_FOGCOLOR];
    fu.fogColor =
        simd::float4{((fc >> 16) & 0xFF) / 255.0f, ((fc >> 8) & 0xFF) / 255.0f,
                     ((fc >> 0) & 0xFF) / 255.0f, ((fc >> 24) & 0xFF) / 255.0f};
    memcpy(&fu.fogStart, &m_RenderStates[D3DRS_FOGSTART], 4);
    memcpy(&fu.fogEnd, &m_RenderStates[D3DRS_FOGEND], 4);
    memcpy(&fu.fogDensity, &m_RenderStates[D3DRS_FOGDENSITY], 4);
  }
  fu.hasTexture0 = (m_Textures[0] != nullptr) ? 1 : 0;
  fu.hasTexture1 = (m_Textures[1] != nullptr) ? 1 : 0;
  [MTL_ENCODER setFragmentBytes:&fu length:sizeof(fu) atIndex:2];

  // 5b. Build Lighting Uniforms (buffer 3) — Stage 8 Lighting
  LightingUniforms lu;
  memset(&lu, 0, sizeof(lu));
  for (int i = 0; i < MAX_LIGHTS; i++) {
    lu.lights[i].enabled = m_LightEnabled[i] ? 1 : 0;
    if (m_LightEnabled[i]) {
      const D3DLIGHT8 &l = m_Lights[i];
      lu.lights[i].type = (uint32_t)l.Type;
      lu.lights[i].diffuse =
          simd::float4{l.Diffuse.r, l.Diffuse.g, l.Diffuse.b, l.Diffuse.a};
      lu.lights[i].ambient =
          simd::float4{l.Ambient.r, l.Ambient.g, l.Ambient.b, l.Ambient.a};
      lu.lights[i].specular =
          simd::float4{l.Specular.r, l.Specular.g, l.Specular.b, l.Specular.a};
      lu.lights[i].position =
          simd::float3{l.Position.x, l.Position.y, l.Position.z};
      lu.lights[i].direction =
          simd::float3{l.Direction.x, l.Direction.y, l.Direction.z};
      lu.lights[i].range = l.Range;
      lu.lights[i].falloff = l.Falloff;
      lu.lights[i].attenuation0 = l.Attenuation0;
      lu.lights[i].attenuation1 = l.Attenuation1;
      lu.lights[i].attenuation2 = l.Attenuation2;
      lu.lights[i].theta = l.Theta;
      lu.lights[i].phi = l.Phi;
    }
  }
  lu.materialDiffuse = simd::float4{m_Material.Diffuse.r, m_Material.Diffuse.g,
                                    m_Material.Diffuse.b, m_Material.Diffuse.a};
  lu.materialAmbient = simd::float4{m_Material.Ambient.r, m_Material.Ambient.g,
                                    m_Material.Ambient.b, m_Material.Ambient.a};
  lu.materialSpecular =
      simd::float4{m_Material.Specular.r, m_Material.Specular.g,
                   m_Material.Specular.b, m_Material.Specular.a};
  lu.materialEmissive =
      simd::float4{m_Material.Emissive.r, m_Material.Emissive.g,
                   m_Material.Emissive.b, m_Material.Emissive.a};
  lu.materialPower = m_Material.Power;
  DWORD ga2 = m_RenderStates[D3DRS_AMBIENT];
  lu.globalAmbient =
      simd::float4{((ga2 >> 16) & 0xFF) / 255.0f, ((ga2 >> 8) & 0xFF) / 255.0f,
                   ((ga2 >> 0) & 0xFF) / 255.0f, ((ga2 >> 24) & 0xFF) / 255.0f};
  lu.lightingEnabled = m_RenderStates[D3DRS_LIGHTING];
  lu.diffuseSource = m_RenderStates[D3DRS_DIFFUSEMATERIALSOURCE];
  lu.ambientSource = m_RenderStates[D3DRS_AMBIENTMATERIALSOURCE];
  lu.specularSource = m_RenderStates[D3DRS_SPECULARMATERIALSOURCE];
  lu.emissiveSource = m_RenderStates[D3DRS_EMISSIVEMATERIALSOURCE];
  lu.hasNormals = (fvf & D3DFVF_NORMAL) ? 1 : 0;

  // Fog parameters for vertex fog computation (Stage 9)
  {
    DWORD fogEnable = m_RenderStates[D3DRS_FOGENABLE];
    if (fogEnable) {
      uint32_t mode = m_RenderStates[D3DRS_FOGTABLEMODE];
      if (mode == D3DFOG_NONE) {
        mode = m_RenderStates[D3DRS_FOGVERTEXMODE];
      }
      lu.fogMode = mode;
    } else {
      lu.fogMode = 0;
    }
    memcpy(&lu.fogStart, &m_RenderStates[D3DRS_FOGSTART], 4);
    memcpy(&lu.fogEnd, &m_RenderStates[D3DRS_FOGEND], 4);
    memcpy(&lu.fogDensity, &m_RenderStates[D3DRS_FOGDENSITY], 4);
  }

  [MTL_ENCODER setVertexBytes:&lu length:sizeof(lu) atIndex:3];

  // 6. Bind Textures and Samplers
  for (int s = 0; s < 2; s++) {
    if (m_Textures[s]) {
      MetalTexture8 *tex = (MetalTexture8 *)m_Textures[s];
      id<MTLTexture> mtlTex = tex->GetMTLTexture();
      if (mtlTex) {
        [MTL_ENCODER setFragmentTexture:mtlTex atIndex:s];
      }
    }
    void *samplerState = GetSamplerState(s);
    if (samplerState) {
      [MTL_ENCODER
          setFragmentSamplerState:(__bridge id<MTLSamplerState>)samplerState
                          atIndex:s];
    }
  }

  // 5. Draw
  MTLPrimitiveType mtlPt = MTLPrimitiveTypeTriangle;
  if (pt == D3DPT_TRIANGLELIST)
    mtlPt = MTLPrimitiveTypeTriangle;
  else if (pt == D3DPT_TRIANGLESTRIP)
    mtlPt = MTLPrimitiveTypeTriangleStrip;

  UINT indexCount = 0;
  if (pt == D3DPT_TRIANGLELIST)
    indexCount = pc * 3;
  else if (pt == D3DPT_TRIANGLESTRIP)
    indexCount = pc + 2;

  MetalIndexBuffer8 *ib = (MetalIndexBuffer8 *)m_IndexBuffer;
  MTLIndexType idxType =
      ib->Is_32Bit() ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
  uint32_t offset = si * (ib->Is_32Bit() ? 4 : 2);

  // m_BaseVertexIndex comes from DX8 SetIndices(ib, BaseVertexIndex).
  // DX8 adds this to every index value before fetching the vertex.
  // Metal's drawIndexedPrimitives:baseVertex does the same thing.
  [MTL_ENCODER drawIndexedPrimitives:mtlPt
                          indexCount:indexCount
                           indexType:idxType
                         indexBuffer:(__bridge id<MTLBuffer>)ib->GetMTLBuffer()
                   indexBufferOffset:offset
                       instanceCount:1
                          baseVertex:(NSInteger)m_BaseVertexIndex
                        baseInstance:0];

  if (dipCount <= 10)
    fprintf(stderr,
            "DEBUG DIP[%d]: DRAW pt=%u fvf=0x%x nv=%u pc=%u idxCnt=%u "
            "baseVtx=%u ibOff=%u tex0=%p tex1=%p\n",
            dipCount, (unsigned)pt, (unsigned)fvf, nv, pc, indexCount,
            (unsigned)m_BaseVertexIndex, offset, (void *)m_Textures[0],
            (void *)m_Textures[1]);

  return D3D_OK;
}

STDMETHODIMP MetalDevice8::DrawPrimitiveUP(DWORD pt, UINT pc, const void *data,
                                           UINT stride) {
  if (!m_CurrentEncoder || !data || pc == 0)
    return D3D_OK;

  // Determine vertex count from primitive type and count
  UINT vertexCount = 0;
  MTLPrimitiveType mtlPrimType;
  switch (pt) {
  case D3DPT_TRIANGLELIST:
    vertexCount = pc * 3;
    mtlPrimType = MTLPrimitiveTypeTriangle;
    break;
  case D3DPT_TRIANGLESTRIP:
    vertexCount = pc + 2;
    mtlPrimType = MTLPrimitiveTypeTriangleStrip;
    break;
  case D3DPT_LINELIST:
    vertexCount = pc * 2;
    mtlPrimType = MTLPrimitiveTypeLine;
    break;
  case D3DPT_LINESTRIP:
    vertexCount = pc + 1;
    mtlPrimType = MTLPrimitiveTypeLineStrip;
    break;
  case D3DPT_POINTLIST:
    vertexCount = pc;
    mtlPrimType = MTLPrimitiveTypePoint;
    break;
  default:
    return D3D_OK;
  }

  // Use current FVF (from SetVertexShader or stream source)
  DWORD fvf = m_VertexShader;
  if (fvf == 0 && m_StreamSource) {
    fvf = GetBufferFVF(m_StreamSource);
  }
  if (fvf == 0) {
    // Default: position + diffuse + tex1 (common for UP draws)
    fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
  }

  id<MTLRenderPipelineState> pso =
      (__bridge id<MTLRenderPipelineState>)GetPSO(fvf);
  if (!pso)
    return D3D_OK;

  [MTL_ENCODER setRenderPipelineState:pso];
  ApplyPerDrawState();

  // Upload vertex data inline (up to 4KB via setVertexBytes)
  UINT dataSize = vertexCount * stride;
  if (dataSize <= 4096) {
    [MTL_ENCODER setVertexBytes:data length:dataSize atIndex:0];
  } else {
    // For larger data, create a temporary buffer
    id<MTLBuffer> tmpBuf =
        [MTL_DEVICE newBufferWithBytes:data
                                length:dataSize
                               options:MTLResourceStorageModeShared];
    if (!tmpBuf)
      return D3D_OK;
    [MTL_ENCODER setVertexBuffer:tmpBuf offset:0 atIndex:0];
  }

  // Bind zero buffer for missing vertex attributes (FVF defaults)
  if (m_ZeroBuffer) {
    [MTL_ENCODER setVertexBuffer:(__bridge id<MTLBuffer>)m_ZeroBuffer
                          offset:0
                         atIndex:30];
  }

  // Uniforms, fragment uniforms, lighting — same as DrawPrimitive
  MetalUniforms u;
  memcpy(&u.world, &m_Transforms[D3DTS_WORLD], 64);
  memcpy(&u.view, &m_Transforms[D3DTS_VIEW], 64);
  memcpy(&u.projection, &m_Transforms[D3DTS_PROJECTION], 64);
  u.screenSize.x = m_ScreenWidth;
  u.screenSize.y = m_ScreenHeight;
  u.useProjection = (fvf & D3DFVF_XYZRHW) ? 2 : 1;
  u.shaderSettings = 0;
  [MTL_ENCODER setVertexBytes:&u length:sizeof(u) atIndex:1];
  [MTL_ENCODER setFragmentBytes:&u length:sizeof(u) atIndex:1];

  // Fragment Uniforms
  FragmentUniforms fu;
  memset(&fu, 0, sizeof(fu));
  for (int s = 0; s < 2; s++) {
    fu.stages[s].colorOp = m_TextureStageStates[s][D3DTSS_COLOROP];
    fu.stages[s].colorArg1 = m_TextureStageStates[s][D3DTSS_COLORARG1];
    fu.stages[s].colorArg2 = m_TextureStageStates[s][D3DTSS_COLORARG2];
    fu.stages[s].alphaOp = m_TextureStageStates[s][D3DTSS_ALPHAOP];
    fu.stages[s].alphaArg1 = m_TextureStageStates[s][D3DTSS_ALPHAARG1];
    fu.stages[s].alphaArg2 = m_TextureStageStates[s][D3DTSS_ALPHAARG2];
  }
  DWORD tf = m_RenderStates[D3DRS_TEXTUREFACTOR];
  fu.textureFactor.x = ((tf >> 16) & 0xFF) / 255.0f;
  fu.textureFactor.y = ((tf >> 8) & 0xFF) / 255.0f;
  fu.textureFactor.z = ((tf >> 0) & 0xFF) / 255.0f;
  fu.textureFactor.w = ((tf >> 24) & 0xFF) / 255.0f;
  fu.alphaTestEnable = m_RenderStates[D3DRS_ALPHATESTENABLE];
  fu.alphaFunc = m_RenderStates[D3DRS_ALPHAFUNC];
  fu.alphaRef = m_RenderStates[D3DRS_ALPHAREF] / 255.0f;
  {
    DWORD fogEnable = m_RenderStates[D3DRS_FOGENABLE];
    if (fogEnable) {
      uint32_t mode = m_RenderStates[D3DRS_FOGTABLEMODE];
      if (mode == D3DFOG_NONE)
        mode = m_RenderStates[D3DRS_FOGVERTEXMODE];
      fu.fogMode = mode;
    } else {
      fu.fogMode = 0;
    }
    DWORD fc = m_RenderStates[D3DRS_FOGCOLOR];
    fu.fogColor =
        simd::float4{((fc >> 16) & 0xFF) / 255.0f, ((fc >> 8) & 0xFF) / 255.0f,
                     ((fc >> 0) & 0xFF) / 255.0f, ((fc >> 24) & 0xFF) / 255.0f};
    memcpy(&fu.fogStart, &m_RenderStates[D3DRS_FOGSTART], 4);
    memcpy(&fu.fogEnd, &m_RenderStates[D3DRS_FOGEND], 4);
    memcpy(&fu.fogDensity, &m_RenderStates[D3DRS_FOGDENSITY], 4);
  }
  fu.hasTexture0 = (m_Textures[0] != nullptr) ? 1 : 0;
  fu.hasTexture1 = (m_Textures[1] != nullptr) ? 1 : 0;
  [MTL_ENCODER setFragmentBytes:&fu length:sizeof(fu) atIndex:2];

  // Lighting Uniforms
  LightingUniforms lu;
  memset(&lu, 0, sizeof(lu));
  lu.lightingEnabled = m_RenderStates[D3DRS_LIGHTING];
  lu.hasNormals = (fvf & D3DFVF_NORMAL) ? 1 : 0;
  {
    DWORD fogEnable = m_RenderStates[D3DRS_FOGENABLE];
    if (fogEnable) {
      uint32_t mode = m_RenderStates[D3DRS_FOGTABLEMODE];
      if (mode == D3DFOG_NONE)
        mode = m_RenderStates[D3DRS_FOGVERTEXMODE];
      lu.fogMode = mode;
    } else {
      lu.fogMode = 0;
    }
    memcpy(&lu.fogStart, &m_RenderStates[D3DRS_FOGSTART], 4);
    memcpy(&lu.fogEnd, &m_RenderStates[D3DRS_FOGEND], 4);
    memcpy(&lu.fogDensity, &m_RenderStates[D3DRS_FOGDENSITY], 4);
  }
  [MTL_ENCODER setVertexBytes:&lu length:sizeof(lu) atIndex:3];

  // Bind textures
  for (int s = 0; s < 2; s++) {
    if (m_Textures[s]) {
      MetalTexture8 *tex = (MetalTexture8 *)m_Textures[s];
      id<MTLTexture> mtlTex = tex->GetMTLTexture();
      if (mtlTex) {
        [MTL_ENCODER setFragmentTexture:mtlTex atIndex:s];
      }
    }
  }
  for (int s = 0; s < 2; s++) {
    void *sam = GetSamplerState(s);
    if (sam) {
      [MTL_ENCODER setFragmentSamplerState:(__bridge id<MTLSamplerState>)sam
                                   atIndex:s];
    }
  }

  // Draw
  [MTL_ENCODER drawPrimitives:mtlPrimType
                  vertexStart:0
                  vertexCount:vertexCount];

  return D3D_OK;
}
STDMETHODIMP
MetalDevice8::DrawIndexedPrimitiveUP(DWORD pt, UINT mvi, UINT nvi, UINT pc,
                                     const void *idata, D3DFORMAT ifmt,
                                     const void *vdata, UINT vstride) {
  // TODO: Implement if needed — currently no callers in the engine
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Vertex Shaders
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreateVertexShader(const DWORD *decl,
                                              const DWORD *func, DWORD *handle,
                                              DWORD usage) {
  if (handle)
    *handle = 0;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetVertexShader(DWORD h) {
  m_VertexShader = h;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::DeleteVertexShader(DWORD h) { return D3D_OK; }

STDMETHODIMP MetalDevice8::SetVertexShaderConstant(DWORD r, const void *d,
                                                   DWORD c) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Stream Source / Indices
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetStreamSource(UINT streamNum,
                                           IDirect3DVertexBuffer8 *vb,
                                           UINT stride) {
  if (streamNum == 0) {
    m_StreamSource = vb;
    m_StreamStride = stride;
  }
  return D3D_OK;
}

HRESULT MetalDevice8::GetStreamSource(UINT streamNum,
                                      IDirect3DVertexBuffer8 **vb,
                                      UINT *stride) {
  if (streamNum == 0) {
    if (vb)
      *vb = m_StreamSource;
    if (stride)
      *stride = m_StreamStride;
  }
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetIndices(IDirect3DIndexBuffer8 *ib, UINT base) {
  m_IndexBuffer = ib;
  m_BaseVertexIndex = base;
  return D3D_OK;
}

HRESULT MetalDevice8::GetIndices(IDirect3DIndexBuffer8 **ib, UINT *base) {
  if (ib)
    *ib = m_IndexBuffer;
  if (base)
    *base = m_BaseVertexIndex;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Pixel Shaders
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreatePixelShader(const DWORD *func, DWORD *handle) {
  if (handle)
    *handle = 0;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetPixelShader(DWORD h) {
  m_PixelShader = h;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::DeletePixelShader(DWORD h) { return D3D_OK; }

STDMETHODIMP MetalDevice8::SetPixelShaderConstant(DWORD r, const void *d,
                                                  DWORD c) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Non-override helpers
// ─────────────────────────────────────────────────────

HRESULT MetalDevice8::GetDirect3D(IDirect3D8 **ppD3D8) {
  if (ppD3D8)
    *ppD3D8 = nullptr;
  return D3D_OK;
}

#endif // __APPLE__
