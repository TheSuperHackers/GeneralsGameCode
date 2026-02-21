#include <metal_stdlib>
using namespace metal;

// ─────────────────────────────────────────────────────
//  Vertex Uniforms (buffer 1)
// ─────────────────────────────────────────────────────
struct Uniforms {
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float2 screenSize;
    int useProjection;      // 0=passthrough, 1=3D, 2=2D screen space
    uint shaderSettings;    // legacy bitfield (texturing bit etc.)
};

// ─────────────────────────────────────────────────────
//  Stage 7: Fragment Uniforms (buffer 2)
//  Mirrors FragmentUniforms in MetalDevice8.mm
// ─────────────────────────────────────────────────────

// D3DTOP enum values (must match d3d8_stub.h)
#define D3DTOP_DISABLE           1
#define D3DTOP_SELECTARG1        2
#define D3DTOP_SELECTARG2        3
#define D3DTOP_MODULATE          4
#define D3DTOP_MODULATE2X        5
#define D3DTOP_MODULATE4X        6
#define D3DTOP_ADD               7
#define D3DTOP_ADDSIGNED         8
#define D3DTOP_ADDSIGNED2X       9
#define D3DTOP_SUBTRACT         10
#define D3DTOP_ADDSMOOTH        11
#define D3DTOP_BLENDDIFFUSEALPHA  12
#define D3DTOP_BLENDTEXTUREALPHA  13
#define D3DTOP_BLENDFACTORALPHA   14
#define D3DTOP_BLENDCURRENTALPHA  16
#define D3DTOP_MODULATEALPHA_ADDCOLOR   18
#define D3DTOP_MODULATECOLOR_ADDALPHA   19
#define D3DTOP_MODULATEINVALPHA_ADDCOLOR 20
#define D3DTOP_DOTPRODUCT3       22

// D3DTA source selectors (low 4 bits)
#define D3DTA_DIFFUSE  0
#define D3DTA_CURRENT  1
#define D3DTA_TEXTURE  2
#define D3DTA_TFACTOR  3
#define D3DTA_SPECULAR 4
// D3DTA modifiers (high bits)
#define D3DTA_COMPLEMENT     0x10
#define D3DTA_ALPHAREPLICATE 0x20

// D3DCMP enum values
#define D3DCMP_NEVER        1
#define D3DCMP_LESS         2
#define D3DCMP_EQUAL        3
#define D3DCMP_LESSEQUAL    4
#define D3DCMP_GREATER      5
#define D3DCMP_NOTEQUAL     6
#define D3DCMP_GREATEREQUAL 7
#define D3DCMP_ALWAYS       8

struct TextureStageConfig {
    uint colorOp;     // D3DTOP enum
    uint colorArg1;   // D3DTA enum
    uint colorArg2;   // D3DTA enum
    uint alphaOp;     // D3DTOP enum
    uint alphaArg1;   // D3DTA enum
    uint alphaArg2;   // D3DTA enum
    uint _pad0;
    uint _pad1;
};

struct FragmentUniforms {
    TextureStageConfig stages[2];
    float4 textureFactor;   // D3DRS_TEXTUREFACTOR (ARGB as float4)
    float4 fogColor;
    float  fogStart;
    float  fogEnd;
    float  fogDensity;
    uint   fogMode;
    uint   alphaTestEnable;
    uint   alphaFunc;        // D3DCMP enum
    float  alphaRef;         // normalized 0..1
    uint   hasTexture0;
    uint   hasTexture1;
    uint   _pad0;
    uint   _pad1;
};

// ─────────────────────────────────────────────────────
//  Stage 8: Lighting Uniforms (buffer 3)
//  Mirrors LightData / LightingUniforms in MetalDevice8.mm
// ─────────────────────────────────────────────────────

#define D3DLIGHT_POINT       1
#define D3DLIGHT_SPOT        2
#define D3DLIGHT_DIRECTIONAL 3

#define D3DMCS_MATERIAL 0
#define D3DMCS_COLOR1   1
#define D3DMCS_COLOR2   2

// D3DFOG modes
#define D3DFOG_NONE   0
#define D3DFOG_EXP    1
#define D3DFOG_EXP2   2
#define D3DFOG_LINEAR 3

struct LightData {
    float4 diffuse;
    float4 ambient;
    float4 specular;
    float3 position;
    float  range;
    float3 direction;
    float  falloff;
    float  attenuation0;
    float  attenuation1;
    float  attenuation2;
    float  theta;
    float  phi;
    uint   type;     // 1=point, 2=spot, 3=directional
    uint   enabled;
    float  _pad;
};

struct LightingUniforms {
    LightData lights[4];
    float4 materialDiffuse;
    float4 materialAmbient;
    float4 materialSpecular;
    float4 materialEmissive;
    float  materialPower;
    float4 globalAmbient;
    uint   lightingEnabled;
    uint   diffuseSource;
    uint   ambientSource;
    uint   specularSource;
    uint   emissiveSource;
    uint   hasNormals;
    // Stage 9: Fog parameters (for vertex fog computation)
    float  fogStart;
    float  fogEnd;
    float  fogDensity;
    uint   fogMode; // 0=NONE, 1=EXP, 2=EXP2, 3=LINEAR
};

// ─────────────────────────────────────────────────────
//  Vertex I/O
// ─────────────────────────────────────────────────────

struct VertexIn {
    float4 position     [[attribute(0)]]; // float3 (auto-padded w=1) or float4 (XYZRHW)
    float4 color        [[attribute(1)]]; // diffuse vertex color (BGRA normalized)
    float2 texCoord     [[attribute(2)]]; // UV set 0
    float3 normal       [[attribute(3)]]; // vertex normal (for lighting)
    float4 specVtxColor [[attribute(4)]]; // specular vertex color (BGRA normalized)
    float2 texCoord2    [[attribute(5)]]; // UV set 1 (multi-texturing)
};

struct VertexOut {
    float4 position [[position]];
    float4 color;
    float4 specularColor;
    float2 texCoord;
    float2 texCoord2;
    float fogFactor;
};

// ─────────────────────────────────────────────────────
//  Vertex Shader (with DX8 per-vertex lighting)
// ─────────────────────────────────────────────────────

vertex VertexOut vertex_main(VertexIn in [[stage_in]],
                            constant Uniforms &uniforms [[buffer(1)]],
                            constant LightingUniforms &lighting [[buffer(3)]]) {
    VertexOut out;
    
    float4 pos = float4(in.position.xyz, 1.0);
    
    if (uniforms.useProjection == 1) {
        // 3D Mode
        out.position = uniforms.projection * uniforms.view * uniforms.world * pos;
    } else if (uniforms.useProjection == 2) {
        // 2D Mode (Screen Space / XYZRHW)
        // XYZRHW: x,y are screen coords, z is depth [0..1], w is 1/z (rhw)
        float2 screenPos = (in.position.xy / uniforms.screenSize) * 2.0 - 1.0;
        float z = in.position.z; // depth value from vertex
        float rhw = in.position.w; // reciprocal homogeneous w
        out.position = float4(screenPos.x, -screenPos.y, z, 1.0);
    } else {
        out.position = pos;
    }
    
    out.texCoord = in.texCoord;
    out.texCoord2 = in.texCoord2;
    out.specularColor = float4(0.0, 0.0, 0.0, 0.0);
    
    // For 2D/screen-space vertices (XYZRHW), skip fog entirely.
    // The world/view transforms are meaningless for screen-space coords,
    // and applying fog to UI elements would make them invisible.
    if (uniforms.useProjection == 2) {
        out.fogFactor = 1.0; // fully visible, no fog
    } else {
        // 3D fog distance — computed properly using DX8 fog formulas
        float4 viewPos = uniforms.view * uniforms.world * pos;
        float dist = length(viewPos.xyz); // distance from camera in view space
        
        // Compute fog factor based on fog mode from LightingUniforms
        // fogFactor: 1.0 = no fog (fully visible), 0.0 = fully fogged
        if (lighting.fogMode == D3DFOG_LINEAR) {
            float fogRange = lighting.fogEnd - lighting.fogStart;
            if (fogRange > 0.0001) {
                out.fogFactor = clamp((lighting.fogEnd - dist) / fogRange, 0.0, 1.0);
            } else {
                out.fogFactor = (dist < lighting.fogEnd) ? 1.0 : 0.0;
            }
        } else if (lighting.fogMode == D3DFOG_EXP) {
            out.fogFactor = exp(-lighting.fogDensity * dist);
            out.fogFactor = clamp(out.fogFactor, 0.0, 1.0);
        } else if (lighting.fogMode == D3DFOG_EXP2) {
            float exponent = lighting.fogDensity * dist;
            out.fogFactor = exp(-(exponent * exponent));
            out.fogFactor = clamp(out.fogFactor, 0.0, 1.0);
        } else {
            // D3DFOG_NONE — no fog
            out.fogFactor = 1.0; // fully visible
        }
    }
    
    // ─── Per-vertex lighting ───
    if (lighting.lightingEnabled == 0 || lighting.hasNormals == 0) {
        // Lighting disabled or no normals: pass through vertex color
        out.color = in.color;
        return out;
    }
    
    // Transform vertex position and normal to view space
    float4x4 worldView = uniforms.view * uniforms.world;
    float3 posVS = (worldView * pos).xyz;
    
    // Transform normal to view space using upper-left 3x3 of worldView
    // (For non-uniform scale, we'd need inverse-transpose, but DX8 FFP
    //  uses the world-view matrix directly and relies on D3DRS_NORMALIZENORMALS)
    float3 N = normalize(float3x3(worldView[0].xyz, worldView[1].xyz, worldView[2].xyz) * in.normal);
    
    // Resolve material source colors per DX8 spec
    // D3DMCS_MATERIAL=0, D3DMCS_COLOR1=1, D3DMCS_COLOR2=2
    float4 vertColor1 = in.color;
    float4 vertColor2 = in.specVtxColor; // specular vertex color from attribute(4)
    
    float4 matDiffuse  = (lighting.diffuseSource  == D3DMCS_COLOR1) ? vertColor1 :
                         (lighting.diffuseSource  == D3DMCS_COLOR2) ? vertColor2 :
                         lighting.materialDiffuse;
    
    float4 matAmbient  = (lighting.ambientSource  == D3DMCS_COLOR1) ? vertColor1 :
                         (lighting.ambientSource  == D3DMCS_COLOR2) ? vertColor2 :
                         lighting.materialAmbient;
    
    float4 matSpecular = (lighting.specularSource == D3DMCS_COLOR1) ? vertColor1 :
                         (lighting.specularSource == D3DMCS_COLOR2) ? vertColor2 :
                         lighting.materialSpecular;
    
    float4 matEmissive = (lighting.emissiveSource == D3DMCS_COLOR1) ? vertColor1 :
                         (lighting.emissiveSource == D3DMCS_COLOR2) ? vertColor2 :
                         lighting.materialEmissive;
    
    // Accumulate lighting components
    float4 totalDiffuse  = float4(0.0);
    float4 totalSpecular = float4(0.0);
    float4 totalAmbient  = lighting.globalAmbient; // start with global ambient Ga
    
    for (int i = 0; i < 4; i++) {
        if (lighting.lights[i].enabled == 0) continue;
        
        constant LightData &light = lighting.lights[i];
        
        float3 Ldir;    // direction FROM vertex TO light (or light direction for directional)
        float atten = 1.0;
        float spot  = 1.0;
        
        if (light.type == D3DLIGHT_DIRECTIONAL) {
            // Directional: Ldir comes from DX8 as direction the light shines
            // In view space: transform then negate
            Ldir = -normalize(float3x3(uniforms.view[0].xyz, uniforms.view[1].xyz, uniforms.view[2].xyz) * light.direction);
            atten = 1.0;
        } else {
            // Point or Spot light
            float3 lightPosVS = (uniforms.view * float4(light.position, 1.0)).xyz;
            float3 toLight = lightPosVS - posVS;
            float d = length(toLight);
            
            // Range check
            if (d > light.range && light.range > 0.0) continue;
            
            Ldir = toLight / max(d, 0.0001);
            
            // Attenuation: 1 / (a0 + a1*d + a2*d²)
            float denominator = light.attenuation0 + light.attenuation1 * d + light.attenuation2 * d * d;
            atten = 1.0 / max(denominator, 0.0001);
            atten = min(atten, 1.0);
            
            if (light.type == D3DLIGHT_SPOT) {
                // Spotlight cone
                float3 spotDirVS = normalize(float3x3(uniforms.view[0].xyz, uniforms.view[1].xyz, uniforms.view[2].xyz) * light.direction);
                float rho = dot(-Ldir, spotDirVS);
                float cosHalfTheta = cos(light.theta * 0.5);
                float cosHalfPhi   = cos(light.phi * 0.5);
                
                if (rho <= cosHalfPhi) {
                    spot = 0.0;
                } else if (rho >= cosHalfTheta) {
                    spot = 1.0;
                } else {
                    float num = rho - cosHalfPhi;
                    float den = cosHalfTheta - cosHalfPhi;
                    spot = pow(max(num / max(den, 0.0001), 0.0), max(light.falloff, 0.0001));
                }
            }
        }
        
        // Ambient contribution from this light
        totalAmbient += light.ambient;
        
        // Diffuse: N·L (clamped)
        float NdotL = max(dot(N, Ldir), 0.0);
        totalDiffuse += light.diffuse * NdotL * atten * spot;
        
        // Specular: (N·H)^power
        if (NdotL > 0.0 && lighting.materialPower > 0.0) {
            // Halfway vector: H = normalize(Ldir + V)
            // For non-local viewer (DX8 default): V = (0,0,1) in view space
            float3 V = float3(0.0, 0.0, 1.0);
            float3 H = normalize(Ldir + V);
            float NdotH = max(dot(N, H), 0.0);
            float specPow = pow(NdotH, lighting.materialPower);
            totalSpecular += light.specular * specPow * atten * spot;
        }
    }
    
    // Final color = emissive + ambient * materialAmbient + diffuse * materialDiffuse
    float4 finalColor;
    finalColor.rgb = matEmissive.rgb
                   + matAmbient.rgb * totalAmbient.rgb
                   + matDiffuse.rgb * totalDiffuse.rgb;
    finalColor.a = matDiffuse.a; // DX8: output alpha = material diffuse alpha
    finalColor = clamp(finalColor, 0.0, 1.0);
    
    out.color = finalColor;
    out.specularColor = float4(matSpecular.rgb * totalSpecular.rgb, 0.0);
    out.specularColor = clamp(out.specularColor, 0.0, 1.0);
    
    return out;
}

// ─────────────────────────────────────────────────────
//  Stage 7: TSS Argument Resolver
//  Selects the color/alpha source based on D3DTA value
// ─────────────────────────────────────────────────────
float4 resolveArg(uint argId,
                  float4 texColor0,
                  float4 texColor1,
                  float4 diffuse,
                  float4 specular,
                  float4 current,
                  float4 tFactor,
                  uint   stageIndex) {
    uint source = argId & 0xF;  // low 4 bits = source selector
    float4 val;
    
    switch (source) {
        case D3DTA_DIFFUSE:  val = diffuse;  break;
        case D3DTA_CURRENT:  val = current;  break;
        case D3DTA_TEXTURE:  val = (stageIndex == 0) ? texColor0 : texColor1; break;
        case D3DTA_TFACTOR:  val = tFactor;  break;
        case D3DTA_SPECULAR: val = specular; break;
        default:             val = current;  break;
    }
    
    // Apply modifiers
    if (argId & D3DTA_COMPLEMENT)     val = 1.0 - val;
    if (argId & D3DTA_ALPHAREPLICATE) val = float4(val.a, val.a, val.a, val.a);
    
    return val;
}

// ─────────────────────────────────────────────────────
//  Stage 7: TSS Operation Evaluator
//  Computes result for both color (.rgb) and alpha (.a)
// ─────────────────────────────────────────────────────
float4 evaluateOp(uint op, float4 arg1, float4 arg2) {
    switch (op) {
        case D3DTOP_DISABLE:
            return arg1; // Shouldn't normally be called for disabled stages
        case D3DTOP_SELECTARG1:
            return arg1;
        case D3DTOP_SELECTARG2:
            return arg2;
        case D3DTOP_MODULATE:
            return arg1 * arg2;
        case D3DTOP_MODULATE2X:
            return clamp(arg1 * arg2 * 2.0, 0.0, 1.0);
        case D3DTOP_MODULATE4X:
            return clamp(arg1 * arg2 * 4.0, 0.0, 1.0);
        case D3DTOP_ADD:
            return clamp(arg1 + arg2, 0.0, 1.0);
        case D3DTOP_ADDSIGNED:
            return clamp(arg1 + arg2 - 0.5, 0.0, 1.0);
        case D3DTOP_ADDSIGNED2X:
            return clamp((arg1 + arg2 - 0.5) * 2.0, 0.0, 1.0);
        case D3DTOP_SUBTRACT:
            return clamp(arg1 - arg2, 0.0, 1.0);
        case D3DTOP_ADDSMOOTH:
            return clamp(arg1 + arg2 - arg1 * arg2, 0.0, 1.0);
        case D3DTOP_BLENDDIFFUSEALPHA: {
            // Uses arg1's alpha as blend factor... but actually it should be
            // the diffuse alpha. We handle this specially in fragment_main.
            return arg1; // Placeholder
        }
        case D3DTOP_BLENDTEXTUREALPHA: {
            return arg1; // Handled in fragment_main with texture alpha
        }
        case D3DTOP_BLENDFACTORALPHA: {
            return arg1; // Handled in fragment_main with factor alpha
        }
        case D3DTOP_BLENDCURRENTALPHA: {
            return arg1; // Handled in fragment_main with current alpha
        }
        case D3DTOP_MODULATEALPHA_ADDCOLOR:
            return float4(arg1.rgb + arg1.a * arg2.rgb, arg1.a);
        case D3DTOP_MODULATECOLOR_ADDALPHA:
            return float4(arg1.rgb * arg2.rgb + arg1.a, arg1.a);
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
            return float4((1.0 - arg1.a) * arg2.rgb + arg1.rgb, arg1.a);
        case D3DTOP_DOTPRODUCT3: {
            float d = dot(arg1.rgb * 2.0 - 1.0, arg2.rgb * 2.0 - 1.0);
            return float4(d, d, d, d);
        }
        default:
            return arg1 * arg2; // Fallback: modulate
    }
}

// ─────────────────────────────────────────────────────
//  Stage 7: Blend operation helper
//  For BLENDDIFFUSEALPHA, BLENDTEXTUREALPHA, etc.
// ─────────────────────────────────────────────────────
float4 evaluateBlendOp(uint op, float4 arg1, float4 arg2,
                       float4 diffuse, float4 texColor0, float4 tFactor,
                       float4 current) {
    float alpha;
    switch (op) {
        case D3DTOP_BLENDDIFFUSEALPHA:
            alpha = diffuse.a;
            return float4(mix(arg2.rgb, arg1.rgb, alpha),
                         mix(arg2.a, arg1.a, alpha));
        case D3DTOP_BLENDTEXTUREALPHA:
            alpha = texColor0.a;
            return float4(mix(arg2.rgb, arg1.rgb, alpha),
                         mix(arg2.a, arg1.a, alpha));
        case D3DTOP_BLENDFACTORALPHA:
            alpha = tFactor.a;
            return float4(mix(arg2.rgb, arg1.rgb, alpha),
                         mix(arg2.a, arg1.a, alpha));
        case D3DTOP_BLENDCURRENTALPHA:
            alpha = current.a;
            return float4(mix(arg2.rgb, arg1.rgb, alpha),
                         mix(arg2.a, arg1.a, alpha));
        default:
            return evaluateOp(op, arg1, arg2);
    }
}

// ─────────────────────────────────────────────────────
//  Alpha test comparison
// ─────────────────────────────────────────────────────
bool alphaTestPass(uint func, float alphaVal, float ref) {
    switch (func) {
        case D3DCMP_NEVER:        return false;
        case D3DCMP_LESS:         return alphaVal <  ref;
        case D3DCMP_EQUAL:        return abs(alphaVal - ref) < 0.004;
        case D3DCMP_LESSEQUAL:    return alphaVal <= ref;
        case D3DCMP_GREATER:      return alphaVal >  ref;
        case D3DCMP_NOTEQUAL:     return abs(alphaVal - ref) >= 0.004;
        case D3DCMP_GREATEREQUAL: return alphaVal >= ref;
        case D3DCMP_ALWAYS:       return true;
        default:                  return true;
    }
}

// ─────────────────────────────────────────────────────
//  Stage 7: Fragment Shader with full TSS support
// ─────────────────────────────────────────────────────
fragment float4 fragment_main(VertexOut in [[stage_in]],
                             constant Uniforms &uniforms [[buffer(1)]],
                             constant FragmentUniforms &fragUniforms [[buffer(2)]],
                             texture2d<float> tex0 [[texture(0)]],
                             texture2d<float> tex1 [[texture(1)]],
                             sampler sampler0 [[sampler(0)]],
                             sampler sampler1 [[sampler(1)]]) {
    
    // Sample textures
    float4 texColor0 = (fragUniforms.hasTexture0 != 0) ? tex0.sample(sampler0, in.texCoord)
                                                        : float4(1.0);
    float4 texColor1 = (fragUniforms.hasTexture1 != 0) ? tex1.sample(sampler1, in.texCoord2)
                                                        : float4(1.0);
    
    float4 diffuse = in.color;
    
    // TEMPORARY: bypass TSS for 3D draws to get visible terrain
    // The TSS pipeline currently produces black output for 3D draws.
    // TODO: investigate terrain texture upload and TSS pipeline
    if (uniforms.useProjection == 1) {
        // DX8 terrain vertices have alpha=0 (used for multi-pass blending).
        // Force opaque since we bypass the blend pipeline.
        return float4(diffuse.rgb, 1.0);
    }

    // Normal 2D path with full TSS processing
    float4 specular = in.specularColor;
    float4 tFactor = fragUniforms.textureFactor;
    float4 current = diffuse;

    // --- Process Stage 0 ---
    uint colorOp0 = fragUniforms.stages[0].colorOp;
    if (colorOp0 != D3DTOP_DISABLE) {
        float4 cArg1 = resolveArg(fragUniforms.stages[0].colorArg1, texColor0, texColor1, diffuse, specular, current, tFactor, 0);
        float4 cArg2 = resolveArg(fragUniforms.stages[0].colorArg2, texColor0, texColor1, diffuse, specular, current, tFactor, 0);
        float4 colorResult = evaluateBlendOp(colorOp0, cArg1, cArg2, diffuse, texColor0, tFactor, current);
        uint alphaOp0 = fragUniforms.stages[0].alphaOp;
        float4 aArg1 = resolveArg(fragUniforms.stages[0].alphaArg1, texColor0, texColor1, diffuse, specular, current, tFactor, 0);
        float4 aArg2 = resolveArg(fragUniforms.stages[0].alphaArg2, texColor0, texColor1, diffuse, specular, current, tFactor, 0);
        float4 alphaResult = evaluateBlendOp(alphaOp0, aArg1, aArg2, diffuse, texColor0, tFactor, current);
        current = float4(colorResult.rgb, alphaResult.a);
    }

    // --- Process Stage 1 ---
    uint colorOp1 = fragUniforms.stages[1].colorOp;
    if (colorOp1 != D3DTOP_DISABLE) {
        float4 c1a1 = resolveArg(fragUniforms.stages[1].colorArg1, texColor0, texColor1, diffuse, specular, current, tFactor, 1);
        float4 c1a2 = resolveArg(fragUniforms.stages[1].colorArg2, texColor0, texColor1, diffuse, specular, current, tFactor, 1);
        float4 cr1 = evaluateBlendOp(colorOp1, c1a1, c1a2, diffuse, texColor0, tFactor, current);
        uint alphaOp1 = fragUniforms.stages[1].alphaOp;
        float4 a1a1 = resolveArg(fragUniforms.stages[1].alphaArg1, texColor0, texColor1, diffuse, specular, current, tFactor, 1);
        float4 a1a2 = resolveArg(fragUniforms.stages[1].alphaArg2, texColor0, texColor1, diffuse, specular, current, tFactor, 1);
        float4 ar1 = evaluateBlendOp(alphaOp1, a1a1, a1a2, diffuse, texColor0, tFactor, current);
        current = float4(cr1.rgb, ar1.a);
    }

    // --- Alpha Test ---
    if (fragUniforms.alphaTestEnable != 0) {
        if (!alphaTestPass(fragUniforms.alphaFunc, current.a, fragUniforms.alphaRef)) {
            discard_fragment();
        }
    }

    // --- Fog ---
    if (fragUniforms.fogMode != 0) {
        current.rgb = mix(fragUniforms.fogColor.rgb, current.rgb, in.fogFactor);
    }

    // Add specular
    current.rgb = clamp(current.rgb + specular.rgb, 0.0, 1.0);

    return current;
}
