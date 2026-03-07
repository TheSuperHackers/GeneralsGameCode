/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * D3DXCompat.h - D3DX8 compatibility layer using WWMath
 * 
 * This header provides replacements for D3DX8 math functions using the existing
 * WWMath library (Westwood Math). This eliminates the need for d3dx8.dll at runtime.
 * 
 * Usage: Define NO_D3DX before including D3DX8 headers
 */

#pragma once

#ifdef NO_D3DX

// Prevent min-dx8-sdk headers from defining D3DX functions/types
// by pre-defining their include guards (Option A: Include Guard Coordination)
// This allows our compatibility layer to be the sole provider of D3DX functionality
#ifndef __D3DX8_H__
#define __D3DX8_H__
#endif

#ifndef __D3DX8CORE_H__
#define __D3DX8CORE_H__
#endif

#ifndef __D3DX8EFFECT_H__
#define __D3DX8EFFECT_H__
#endif

#ifndef __D3DX8MATH_H__
#define __D3DX8MATH_H__
#endif

#ifndef __D3DX8MATH_INL__
#define __D3DX8MATH_INL__
#endif

#ifndef __D3DX8MESH_H__
#define __D3DX8MESH_H__
#endif

#ifndef __D3DX8SHAPES_H__
#define __D3DX8SHAPES_H__
#endif

#ifndef __D3DX8TEX_H__
#define __D3DX8TEX_H__
#endif

// Include D3D8 types
#include <d3d8.h>
#include <limits.h>
#include <float.h>

//-----------------------------------------------------------------------------
// D3DX Constants
//-----------------------------------------------------------------------------

// Default values for D3DX functions
#ifndef D3DX_DEFAULT
#define D3DX_DEFAULT            ULONG_MAX
#define D3DX_DEFAULT_FLOAT      FLT_MAX
#endif

// D3DX math constants
#ifndef D3DX_PI
#define D3DX_PI                 ((FLOAT) 3.141592654f)
#define D3DX_1BYPI              ((FLOAT) 0.318309886f)
#define D3DXToRadian(degree)    ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree(radian)    ((radian) * (180.0f / D3DX_PI))
#endif

// D3DX_FILTER flags for texture operations
#ifndef D3DX_FILTER_NONE
#define D3DX_FILTER_NONE        (1 << 0)
#define D3DX_FILTER_POINT       (2 << 0)
#define D3DX_FILTER_LINEAR      (3 << 0)
#define D3DX_FILTER_TRIANGLE    (4 << 0)
#define D3DX_FILTER_BOX         (5 << 0)

#define D3DX_FILTER_MIRROR_U    (1 << 16)
#define D3DX_FILTER_MIRROR_V    (2 << 16)
#define D3DX_FILTER_MIRROR_W    (4 << 16)
#define D3DX_FILTER_MIRROR      (7 << 16)
#define D3DX_FILTER_DITHER      (8 << 16)
#endif

//-----------------------------------------------------------------------------

// WWMath headers for D3DX math function implementations.
// CppMacros.h must be included first because vector3.h -> STLUtils.h uses CPP_11.
// D3DXWrapper.h is force-included via -include flag which runs before
// precompiled headers, so we cannot rely on PCH here.
#ifdef __cplusplus
    #include "Utility/CppMacros.h"
    #include "vector3.h"
    #include "vector4.h"
    #include "matrix3d.h"
    #include "matrix4.h"
#endif

// Forward declare D3DX types (we'll define compatibility layer)
typedef struct D3DXVECTOR3 D3DXVECTOR3;
typedef struct D3DXVECTOR4 D3DXVECTOR4;
typedef struct D3DXMATRIX D3DXMATRIX;

// D3DX8 Vector and Matrix types - map to D3D types
// Note: D3DXVECTOR3 is identical to D3DVECTOR (Direct3D 8 type)
// Note: D3DXVECTOR4 is a simple {x,y,z,w} structure
// Note: D3DXMATRIX is identical to D3DMATRIX (Direct3D 8 type)

#ifdef __cplusplus

struct D3DXVECTOR3
{
    float x, y, z;
    
    D3DXVECTOR3() {}
    D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    D3DXVECTOR3(const Vector3& v) : x(v.X), y(v.Y), z(v.Z) {}
    
    operator Vector3() const { return Vector3(x, y, z); }
    
    // Array access operator for compatibility
    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }
};

struct D3DXVECTOR4
{
    float x, y, z, w;
    
    D3DXVECTOR4() {}
    D3DXVECTOR4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    D3DXVECTOR4(const Vector4& v) : x(v.X), y(v.Y), z(v.Z), w(v.W) {}
    
    operator Vector4() const { return Vector4(x, y, z, w); }
    
    // Conversion to pointer for passing to D3D functions
    operator const float*() const { return &x; }
    operator float*() { return &x; }
    
    // Array access operator for compatibility
    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }
};

struct D3DXMATRIX : public D3DMATRIX
{
    D3DXMATRIX() {}
    
    // Constructor from 16 float values (row-major order)
    D3DXMATRIX(float m00, float m01, float m02, float m03,
                float m10, float m11, float m12, float m13,
                float m20, float m21, float m22, float m23,
                float m30, float m31, float m32, float m33)
    {
        // D3DMATRIX uses _11, _12, ... _44 notation
        _11 = m00; _12 = m01; _13 = m02; _14 = m03;
        _21 = m10; _22 = m11; _23 = m12; _24 = m13;
        _31 = m20; _32 = m21; _33 = m22; _34 = m23;
        _41 = m30; _42 = m31; _43 = m32; _44 = m33;
    }
    
    D3DXMATRIX(const Matrix4x4& m)
    {
        // Matrix4x4 and D3DMATRIX have transposed layouts - use proper conversion
        // See To_D3DMATRIX in matrix4.cpp for reference
        _11 = m[0][0]; _12 = m[1][0]; _13 = m[2][0]; _14 = m[3][0];
        _21 = m[0][1]; _22 = m[1][1]; _23 = m[2][1]; _24 = m[3][1];
        _31 = m[0][2]; _32 = m[1][2]; _33 = m[2][2]; _34 = m[3][2];
        _41 = m[0][3]; _42 = m[1][3]; _43 = m[2][3]; _44 = m[3][3];
    }

    operator Matrix4x4() const
    {
        // D3DMATRIX and Matrix4x4 have transposed layouts - use proper conversion
        // See To_Matrix4x4 in matrix4.cpp for reference
        Matrix4x4 result;
        result[0][0] = _11; result[0][1] = _21; result[0][2] = _31; result[0][3] = _41;
        result[1][0] = _12; result[1][1] = _22; result[1][2] = _32; result[1][3] = _42;
        result[2][0] = _13; result[2][1] = _23; result[2][2] = _33; result[2][3] = _43;
        result[3][0] = _14; result[3][1] = _24; result[3][2] = _34; result[3][3] = _44;
        return result;
    }

    // operator*= for matrix multiplication (native implementation)
    D3DXMATRIX& operator*=(const D3DXMATRIX& other)
    {
        D3DXMATRIX temp;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp.m[i][j] = m[i][0] * other.m[0][j] +
                               m[i][1] * other.m[1][j] +
                               m[i][2] * other.m[2][j] +
                               m[i][3] * other.m[3][j];
            }
        }
        *this = temp;
        return *this;
    }
};

//=============================================================================
// D3DX8 Math Functions - Native Compatibility Layer
//=============================================================================

//-----------------------------------------------------------------------------
// Vector4 Operations
//-----------------------------------------------------------------------------

// D3DXVec4Dot - Compute dot product of two 4D vectors
inline float D3DXVec4Dot(const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2)
{
    if (!pV1 || !pV2)
        return 0.0f;
    
    Vector4 v1(pV1->x, pV1->y, pV1->z, pV1->w);
    Vector4 v2(pV2->x, pV2->y, pV2->z, pV2->w);
    
    return Vector4::Dot_Product(v1, v2);
}

// D3DXVec4Transform - Transform 4D vector by 4x4 matrix
// D3D convention: row vector * matrix, i.e. [x,y,z,w] * M
inline D3DXVECTOR4* D3DXVec4Transform(
    D3DXVECTOR4* pOut,
    const D3DXVECTOR4* pV,
    const D3DXMATRIX* pM)
{
    if (!pOut || !pV || !pM)
        return pOut;

    // D3D uses row vectors: result = [x,y,z,w] * M
    float x = pV->x, y = pV->y, z = pV->z, w = pV->w;
    pOut->x = x * pM->_11 + y * pM->_21 + z * pM->_31 + w * pM->_41;
    pOut->y = x * pM->_12 + y * pM->_22 + z * pM->_32 + w * pM->_42;
    pOut->z = x * pM->_13 + y * pM->_23 + z * pM->_33 + w * pM->_43;
    pOut->w = x * pM->_14 + y * pM->_24 + z * pM->_34 + w * pM->_44;

    return pOut;
}

//-----------------------------------------------------------------------------
// Vector3 Operations
//-----------------------------------------------------------------------------

// D3DXVec3Transform - Transform 3D vector by 4x4 matrix (homogeneous)
// D3D convention: row vector * matrix, i.e. [x,y,z,1] * M
inline D3DXVECTOR4* D3DXVec3Transform(
    D3DXVECTOR4* pOut,
    const D3DXVECTOR3* pV,
    const D3DXMATRIX* pM)
{
    if (!pOut || !pV || !pM)
        return pOut;

    // D3D uses row vectors: result = [x,y,z,1] * M
    float x = pV->x, y = pV->y, z = pV->z;
    pOut->x = x * pM->_11 + y * pM->_21 + z * pM->_31 + pM->_41;
    pOut->y = x * pM->_12 + y * pM->_22 + z * pM->_32 + pM->_42;
    pOut->z = x * pM->_13 + y * pM->_23 + z * pM->_33 + pM->_43;
    pOut->w = x * pM->_14 + y * pM->_24 + z * pM->_34 + pM->_44;

    return pOut;
}

//-----------------------------------------------------------------------------
// Matrix Operations
//-----------------------------------------------------------------------------

// D3DXMatrixTranspose - Transpose a 4x4 matrix
inline D3DXMATRIX* D3DXMatrixTranspose(
    D3DXMATRIX* pOut,
    const D3DXMATRIX* pM)
{
    if (!pOut || !pM)
        return pOut;

    // Native transpose: out[i][j] = in[j][i]
    // Use temp to handle in-place transpose (pOut == pM)
    D3DXMATRIX temp;
    temp._11 = pM->_11; temp._12 = pM->_21; temp._13 = pM->_31; temp._14 = pM->_41;
    temp._21 = pM->_12; temp._22 = pM->_22; temp._23 = pM->_32; temp._24 = pM->_42;
    temp._31 = pM->_13; temp._32 = pM->_23; temp._33 = pM->_33; temp._34 = pM->_43;
    temp._41 = pM->_14; temp._42 = pM->_24; temp._43 = pM->_34; temp._44 = pM->_44;
    *pOut = temp;

    return pOut;
}

// D3DXMatrixInverse - Compute inverse of a 4x4 matrix
// Returns nullptr if the matrix is singular (determinant near zero).
inline D3DXMATRIX* D3DXMatrixInverse(
    D3DXMATRIX* pOut,
    float* pDeterminant,
    const D3DXMATRIX* pM)
{
    if (!pOut || !pM)
        return pOut;

    const float* m = (const float*)pM;
    float v[16], t[6], det;

    // Calculate pairs for first 8 cofactors
    t[0] = m[10] * m[15] - m[11] * m[14];
    t[1] = m[9] * m[15] - m[11] * m[13];
    t[2] = m[9] * m[14] - m[10] * m[13];
    t[3] = m[8] * m[15] - m[11] * m[12];
    t[4] = m[8] * m[14] - m[10] * m[12];
    t[5] = m[8] * m[13] - m[9] * m[12];

    // Calculate first 4 cofactors
    v[0] = m[5] * t[0] - m[6] * t[1] + m[7] * t[2];
    v[4] = -(m[4] * t[0] - m[6] * t[3] + m[7] * t[4]);
    v[8] = m[4] * t[1] - m[5] * t[3] + m[7] * t[5];
    v[12] = -(m[4] * t[2] - m[5] * t[4] + m[6] * t[5]);

    // Calculate determinant
    det = m[0] * v[0] + m[1] * v[4] + m[2] * v[8] + m[3] * v[12];

    if (pDeterminant)
        *pDeterminant = det;

    // Check for singular matrix
    if (fabsf(det) < 1e-10f)
        return nullptr;

    // Calculate pairs for second 8 cofactors
    t[0] = m[2] * m[7] - m[3] * m[6];
    t[1] = m[1] * m[7] - m[3] * m[5];
    t[2] = m[1] * m[6] - m[2] * m[5];
    t[3] = m[0] * m[7] - m[3] * m[4];
    t[4] = m[0] * m[6] - m[2] * m[4];
    t[5] = m[0] * m[5] - m[1] * m[4];

    v[1] = -(m[1] * (m[10] * m[15] - m[11] * m[14]) - m[2] * (m[9] * m[15] - m[11] * m[13]) + m[3] * (m[9] * m[14] - m[10] * m[13]));
    v[5] = m[0] * (m[10] * m[15] - m[11] * m[14]) - m[2] * (m[8] * m[15] - m[11] * m[12]) + m[3] * (m[8] * m[14] - m[10] * m[12]);
    v[9] = -(m[0] * (m[9] * m[15] - m[11] * m[13]) - m[1] * (m[8] * m[15] - m[11] * m[12]) + m[3] * (m[8] * m[13] - m[9] * m[12]));
    v[13] = m[0] * (m[9] * m[14] - m[10] * m[13]) - m[1] * (m[8] * m[14] - m[10] * m[12]) + m[2] * (m[8] * m[13] - m[9] * m[12]);

    v[2] = m[13] * t[0] - m[14] * t[1] + m[15] * t[2];
    v[6] = -(m[12] * t[0] - m[14] * t[3] + m[15] * t[4]);
    v[10] = m[12] * t[1] - m[13] * t[3] + m[15] * t[5];
    v[14] = -(m[12] * t[2] - m[13] * t[4] + m[14] * t[5]);

    v[3] = -(m[9] * t[0] - m[10] * t[1] + m[11] * t[2]);
    v[7] = m[8] * t[0] - m[10] * t[3] + m[11] * t[4];
    v[11] = -(m[8] * t[1] - m[9] * t[3] + m[11] * t[5]);
    v[15] = m[8] * t[2] - m[9] * t[4] + m[10] * t[5];

    // Divide by determinant
    det = 1.0f / det;
    float* out = (float*)pOut;
    for (int i = 0; i < 16; i++)
        out[i] = v[i] * det;

    return pOut;
}

//-----------------------------------------------------------------------------
// Utility Functions
//-----------------------------------------------------------------------------

// D3DXGetErrorStringA - Get error string for D3D error code
inline HRESULT D3DXGetErrorStringA(HRESULT hr, char* pBuffer, UINT BufferLen)
{
    if (!pBuffer || BufferLen == 0)
        return E_INVALIDARG;
    
    const char* errorStr = nullptr;
    
    switch (hr)
    {
        case D3D_OK:
            errorStr = "No error";
            break;
        case D3DERR_WRONGTEXTUREFORMAT:
            errorStr = "Wrong texture format";
            break;
        case D3DERR_UNSUPPORTEDCOLOROPERATION:
            errorStr = "Unsupported color operation";
            break;
        case D3DERR_UNSUPPORTEDCOLORARG:
            errorStr = "Unsupported color argument";
            break;
        case D3DERR_UNSUPPORTEDALPHAOPERATION:
            errorStr = "Unsupported alpha operation";
            break;
        case D3DERR_UNSUPPORTEDALPHAARG:
            errorStr = "Unsupported alpha argument";
            break;
        case D3DERR_TOOMANYOPERATIONS:
            errorStr = "Too many operations";
            break;
        case D3DERR_CONFLICTINGTEXTUREFILTER:
            errorStr = "Conflicting texture filter";
            break;
        case D3DERR_UNSUPPORTEDFACTORVALUE:
            errorStr = "Unsupported factor value";
            break;
        case D3DERR_CONFLICTINGRENDERSTATE:
            errorStr = "Conflicting render state";
            break;
        case D3DERR_UNSUPPORTEDTEXTUREFILTER:
            errorStr = "Unsupported texture filter";
            break;
        case D3DERR_CONFLICTINGTEXTUREPALETTE:
            errorStr = "Conflicting texture palette";
            break;
        case D3DERR_DRIVERINTERNALERROR:
            errorStr = "Driver internal error";
            break;
        case D3DERR_NOTFOUND:
            errorStr = "Not found";
            break;
        case D3DERR_MOREDATA:
            errorStr = "More data";
            break;
        case D3DERR_DEVICELOST:
            errorStr = "Device lost";
            break;
        case D3DERR_DEVICENOTRESET:
            errorStr = "Device not reset";
            break;
        case D3DERR_NOTAVAILABLE:
            errorStr = "Not available";
            break;
        case D3DERR_OUTOFVIDEOMEMORY:
            errorStr = "Out of video memory";
            break;
        case D3DERR_INVALIDDEVICE:
            errorStr = "Invalid device";
            break;
        case D3DERR_INVALIDCALL:
            errorStr = "Invalid call";
            break;
        case D3DERR_DRIVERINVALIDCALL:
            errorStr = "Driver invalid call";
            break;
        case E_OUTOFMEMORY:
            errorStr = "Out of memory";
            break;
        default:
            errorStr = "Unknown error";
            break;
    }
    
    // Copy error string to buffer (ensure null termination)
    strncpy(pBuffer, errorStr, BufferLen - 1);
    pBuffer[BufferLen - 1] = '\0';
    
    return D3D_OK;
}

// D3DXGetFVFVertexSize - Calculate vertex size from FVF flags
inline UINT D3DXGetFVFVertexSize(DWORD FVF)
{
    UINT size = 0;
    
    // Position formats
    if (FVF & D3DFVF_XYZ) size += 12;       // 3 floats
    if (FVF & D3DFVF_XYZRHW) size += 16;    // 4 floats
    if (FVF & D3DFVF_XYZB1) size += 16;     // 3 floats + 1 blend weight
    if (FVF & D3DFVF_XYZB2) size += 20;     // 3 floats + 2 blend weights
    if (FVF & D3DFVF_XYZB3) size += 24;     // 3 floats + 3 blend weights
    if (FVF & D3DFVF_XYZB4) size += 28;     // 3 floats + 4 blend weights
    if (FVF & D3DFVF_XYZB5) size += 32;     // 3 floats + 5 blend weights
    
    // Normal
    if (FVF & D3DFVF_NORMAL) size += 12;    // 3 floats
    
    // Point size
    if (FVF & D3DFVF_PSIZE) size += 4;      // 1 float
    
    // Diffuse color
    if (FVF & D3DFVF_DIFFUSE) size += 4;    // 1 DWORD (D3DCOLOR)
    
    // Specular color
    if (FVF & D3DFVF_SPECULAR) size += 4;   // 1 DWORD (D3DCOLOR)
    
    // Texture coordinates
    UINT texCount = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    for (UINT i = 0; i < texCount; i++)
    {
        DWORD texFormat = (FVF >> (16 + i * 2)) & 0x3;
        switch (texFormat)
        {
            case D3DFVF_TEXTUREFORMAT1: size += 4; break;   // 1 float
            case D3DFVF_TEXTUREFORMAT2: size += 8; break;   // 2 floats
            case D3DFVF_TEXTUREFORMAT3: size += 12; break;  // 3 floats
            case D3DFVF_TEXTUREFORMAT4: size += 16; break;  // 4 floats
            default: size += 8; break;                      // Default to 2 floats
        }
    }
    
    return size;
}

//-----------------------------------------------------------------------------
// Matrix Operations (Additional)
//-----------------------------------------------------------------------------

// D3DXMatrixMultiply - Multiply two matrices
inline D3DXMATRIX* D3DXMatrixMultiply(
    D3DXMATRIX* pOut,
    const D3DXMATRIX* pM1,
    const D3DXMATRIX* pM2)
{
    if (!pOut || !pM1 || !pM2)
        return pOut;

    // Native implementation - handles aliasing via temp
    D3DXMATRIX temp;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp.m[i][j] = pM1->m[i][0] * pM2->m[0][j] +
                           pM1->m[i][1] * pM2->m[1][j] +
                           pM1->m[i][2] * pM2->m[2][j] +
                           pM1->m[i][3] * pM2->m[3][j];
        }
    }
    *pOut = temp;

    return pOut;
}

// D3DXMatrixRotationZ - Create rotation matrix around Z axis
inline D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* pOut, float angle)
{
    if (!pOut)
        return pOut;

    float c = cosf(angle);
    float s = sinf(angle);

    pOut->_11 = c;  pOut->_12 = s;  pOut->_13 = 0; pOut->_14 = 0;
    pOut->_21 = -s; pOut->_22 = c;  pOut->_23 = 0; pOut->_24 = 0;
    pOut->_31 = 0;  pOut->_32 = 0;  pOut->_33 = 1; pOut->_34 = 0;
    pOut->_41 = 0;  pOut->_42 = 0;  pOut->_43 = 0; pOut->_44 = 1;

    return pOut;
}

// D3DXMatrixScaling - Create scaling matrix
inline D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz)
{
    if (!pOut)
        return pOut;

    pOut->_11 = sx; pOut->_12 = 0;  pOut->_13 = 0;  pOut->_14 = 0;
    pOut->_21 = 0;  pOut->_22 = sy; pOut->_23 = 0;  pOut->_24 = 0;
    pOut->_31 = 0;  pOut->_32 = 0;  pOut->_33 = sz; pOut->_34 = 0;
    pOut->_41 = 0;  pOut->_42 = 0;  pOut->_43 = 0;  pOut->_44 = 1;

    return pOut;
}

// D3DXMatrixTranslation - Create translation matrix
inline D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z)
{
    if (!pOut)
        return pOut;

    pOut->_11 = 1; pOut->_12 = 0; pOut->_13 = 0; pOut->_14 = 0;
    pOut->_21 = 0; pOut->_22 = 1; pOut->_23 = 0; pOut->_24 = 0;
    pOut->_31 = 0; pOut->_32 = 0; pOut->_33 = 1; pOut->_34 = 0;
    pOut->_41 = x; pOut->_42 = y; pOut->_43 = z; pOut->_44 = 1;

    return pOut;
}

// D3DXMatrixIdentity - Initialize matrix to identity
inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut)
{
    if (!pOut)
        return pOut;

    pOut->_11 = 1; pOut->_12 = 0; pOut->_13 = 0; pOut->_14 = 0;
    pOut->_21 = 0; pOut->_22 = 1; pOut->_23 = 0; pOut->_24 = 0;
    pOut->_31 = 0; pOut->_32 = 0; pOut->_33 = 1; pOut->_34 = 0;
    pOut->_41 = 0; pOut->_42 = 0; pOut->_43 = 0; pOut->_44 = 1;

    return pOut;
}

//-----------------------------------------------------------------------------
// Shader Functions (Precompiled Shaders)
//-----------------------------------------------------------------------------

// Forward declaration
struct ID3DXBuffer;

// Precompiled shader bytecode (generated from .psh files)
namespace D3DXCompat_Shaders {
    
    // Note: These are simplified PS 1.1 bytecode arrays
    // Generated by scripts/compile_shaders.py from scripts/shaders/water_shader*.psh files
    // May need validation/correction for production use
    
    // River water shader (water_shader1.psh)
    static constexpr DWORD shader1_bytecode[] = {
        0xFFFF0101,     0x00000042,     0x000F0300,     0x00000042,
        0x000F0301,     0x00000042,     0x000F0302,     0x00000042,
        0x000F0303,     0x00000005,     0x000F0000,     0x000F0100,
        0x000F0300,     0x00000005,     0x000F0001,     0x000F0301,
        0x000F0302,     0x00000003,     0x00070000,     0x000F0000,
        0x000F0303,     0x40000005,     0x00080000,     0x000F0000,
        0x000F0303,     0x00000003,     0x00070000,     0x000F0000,
        0x000F0001,     0x0000FFFF,
    };
    
    // Water with environment mapping (water_shader2.psh)
    static constexpr DWORD shader2_bytecode[] = {
        0xFFFF0101,     0x00000042,     0x000F0300,     0x00000042,
        0x000F0301,     0x00000043,     0x000F0302,     0x000F0301,
        0x00000005,     0x000F0000,     0x000F0100,     0x000F0300,
        0x00000005,     0x00070001,     0x000F0302,     0x000F0200,
        0x00000003,     0x00070000,     0x000F0000,     0x000F0001,
        0x0000FFFF,
    };
    
    // Trapezoid water shader (water_shader3.psh)
    static constexpr DWORD shader3_bytecode[] = {
        0xFFFF0101,     0x00000042,     0x000F0300,     0x00000042,
        0x000F0301,     0x00000042,     0x000F0302,     0x00000042,
        0x000F0303,     0x00000005,     0x000F0000,     0x000F0100,
        0x000F0300,     0x00000004,     0x00070000,     0x000F0301,
        0x000F0302,     0x000F0000,     0x00000005,     0x00070000,
        0x000F0000,     0x000F0303,     0x0000FFFF,
    };
    
} // namespace D3DXCompat_Shaders

// ID3DXBuffer interface definition
struct ID3DXBuffer
{
    virtual HRESULT __stdcall QueryInterface(const IID&, void**) = 0;
    virtual ULONG __stdcall AddRef() = 0;
    virtual ULONG __stdcall Release() = 0;
    virtual void* __stdcall GetBufferPointer() = 0;
    virtual DWORD __stdcall GetBufferSize() = 0;
};

// ID3DXBuffer implementation for shader bytecode
class D3DXShaderBuffer : public ID3DXBuffer
{
private:
    const DWORD* m_pData;
    DWORD m_Size;
    mutable LONG m_RefCount;
    
public:
    D3DXShaderBuffer(const DWORD* pData, DWORD size) 
        : m_pData(pData), m_Size(size), m_RefCount(1) {}
    
    virtual ~D3DXShaderBuffer() {}
    
    // IUnknown methods
    virtual HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    
    virtual ULONG __stdcall AddRef()
    {
        return InterlockedIncrement(&m_RefCount);
    }
    
    virtual ULONG __stdcall Release()
    {
        LONG ref = InterlockedDecrement(&m_RefCount);
        if (ref == 0)
            delete this;
        return ref;
    }
    
    // ID3DXBuffer methods
    virtual void* __stdcall GetBufferPointer()
    {
        return (void*)m_pData;
    }
    
    virtual DWORD __stdcall GetBufferSize()
    {
        return m_Size;
    }
};

// D3DXAssembleShader - Returns precompiled bytecode for known water shaders.
// Recognizes the three water shaders used by the game and returns precompiled
// bytecode instead of performing runtime assembly.
inline HRESULT D3DXAssembleShader(
    const char* pSrcData,
    UINT SrcDataLen,
    DWORD Flags,
    void* ppConstants,
    ID3DXBuffer** ppCompiledShader,
    ID3DXBuffer** ppCompilationErrors)
{
    if (!pSrcData || !ppCompiledShader)
        return D3DERR_INVALIDCALL;
    
    *ppCompiledShader = nullptr;
    if (ppCompilationErrors)
        *ppCompilationErrors = nullptr;
    
    // Identify which shader is being assembled by matching key strings
    // This is safe because the game only assembles these specific shaders
    
    // Shader 1: River water (has "+mul r0.a, r0, t3" - co-issued instruction)
    if (strstr(pSrcData, "+mul r0.a") != nullptr)
    {
        *ppCompiledShader = new D3DXShaderBuffer(
            D3DXCompat_Shaders::shader1_bytecode,
            sizeof(D3DXCompat_Shaders::shader1_bytecode));
        return S_OK;
    }
    
    // Shader 2: Water with env mapping (has "texbem")
    if (strstr(pSrcData, "texbem") != nullptr)
    {
        *ppCompiledShader = new D3DXShaderBuffer(
            D3DXCompat_Shaders::shader2_bytecode,
            sizeof(D3DXCompat_Shaders::shader2_bytecode));
        return S_OK;
    }
    
    // Shader 3: Trapezoid water (has "mad" instruction)
    if (strstr(pSrcData, "mad") != nullptr)
    {
        *ppCompiledShader = new D3DXShaderBuffer(
            D3DXCompat_Shaders::shader3_bytecode,
            sizeof(D3DXCompat_Shaders::shader3_bytecode));
        return S_OK;
    }
    
    // Unknown shader - return error
    // The game will handle this gracefully and use fallback rendering
    return E_FAIL;
}

#endif // __cplusplus

//-----------------------------------------------------------------------------
// Texture Functions (Direct3D 8 wrappers and stubs)
// These don't need WWMath
//-----------------------------------------------------------------------------

#ifdef __cplusplus

// D3DXCreateTexture - Direct wrapper for IDirect3DDevice8::CreateTexture
inline HRESULT D3DXCreateTexture(
    LPDIRECT3DDEVICE8 pDevice,
    UINT Width,
    UINT Height,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    LPDIRECT3DTEXTURE8* ppTexture)
{
    if (!pDevice || !ppTexture)
        return D3DERR_INVALIDCALL;
    
    // Direct D3D8 call - no D3DX involved!
    return pDevice->CreateTexture(Width, Height, MipLevels, Usage, Format, Pool, ppTexture);
}

// D3DXCreateCubeTexture - Direct wrapper for IDirect3DDevice8::CreateCubeTexture
inline HRESULT D3DXCreateCubeTexture(
    LPDIRECT3DDEVICE8 pDevice,
    UINT Size,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    LPDIRECT3DCUBETEXTURE8* ppCubeTexture)
{
    if (!pDevice || !ppCubeTexture)
        return D3DERR_INVALIDCALL;
    
    // Direct D3D8 call
    return pDevice->CreateCubeTexture(Size, MipLevels, Usage, Format, Pool, ppCubeTexture);
}

// D3DXCreateVolumeTexture - Direct wrapper for IDirect3DDevice8::CreateVolumeTexture
inline HRESULT D3DXCreateVolumeTexture(
    LPDIRECT3DDEVICE8 pDevice,
    UINT Width,
    UINT Height,
    UINT Depth,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture)
{
    if (!pDevice || !ppVolumeTexture)
        return D3DERR_INVALIDCALL;
    
    // Direct D3D8 call
    return pDevice->CreateVolumeTexture(Width, Height, Depth, MipLevels, Usage, Format, Pool, ppVolumeTexture);
}

// D3DXCreateTextureFromFileExA - Stub (zero callers in codebase)
// Returns D3DERR_NOTAVAILABLE causing fallback to MissingTexture.
inline HRESULT D3DXCreateTextureFromFileExA(
    LPDIRECT3DDEVICE8 pDevice,
    LPCSTR pSrcFile,
    UINT Width,
    UINT Height,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    DWORD Filter,
    DWORD MipFilter,
    D3DCOLOR ColorKey,
    void* pSrcInfo,
    PALETTEENTRY* pPalette,
    LPDIRECT3DTEXTURE8* ppTexture)
{
    // NOTE: Zero usage in codebase (verified via grep).
    // Returning D3DERR_NOTAVAILABLE causes fallback to MissingTexture.
    // Stub function acceptable for unused functionality.
    return D3DERR_NOTAVAILABLE;
}

// D3DXLoadSurfaceFromSurface - Copy surface data using D3D8's native CopyRects
inline HRESULT D3DXLoadSurfaceFromSurface(
    LPDIRECT3DSURFACE8 pDestSurface,
    const PALETTEENTRY* pDestPalette,
    const RECT* pDestRect,
    LPDIRECT3DSURFACE8 pSrcSurface,
    const PALETTEENTRY* pSrcPalette,
    const RECT* pSrcRect,
    DWORD Filter,
    D3DCOLOR ColorKey)
{
    if (!pDestSurface || !pSrcSurface)
        return D3DERR_INVALIDCALL;
    
    // Get D3D8 device from source surface
    IDirect3DDevice8* pDevice = nullptr;
    HRESULT hr = pSrcSurface->GetDevice(&pDevice);
    if (FAILED(hr))
        return hr;
    
    // Convert destination RECT to POINT for CopyRects API
    POINT destPoint = {0, 0};
    if (pDestRect) {
        destPoint.x = pDestRect->left;
        destPoint.y = pDestRect->top;
    }
    
    // Use D3D8's native hardware-accelerated CopyRects
    // This is the same API used by DX8Wrapper::_Copy_DX8_Rects (14 uses in codebase)
    hr = pDevice->CopyRects(
        pSrcSurface,
        pSrcRect,                        // Source rect (nullptr = entire surface)
        pSrcRect ? 1 : 0,                // Number of rects (0 = full surface)
        pDestSurface,
        pDestRect ? &destPoint : nullptr // Dest point (nullptr = 0,0)
    );
    
    pDevice->Release();
    return hr;
}

// D3DXFilterTexture - Stub (no-op, textures use pre-existing mipmaps)
inline HRESULT D3DXFilterTexture(
    LPDIRECT3DBASETEXTURE8 pTexture,
    const PALETTEENTRY* pPalette,
    UINT SrcLevel,
    DWORD Filter)
{
    // Stub: No-op, textures use pre-existing mipmaps
    return D3D_OK;
}

//-----------------------------------------------------------------------------
// Font Functions (Stubs for unused functionality)
//-----------------------------------------------------------------------------

// Forward declaration for D3DX Font interface
struct ID3DXFont;
typedef struct ID3DXFont *LPD3DXFONT;

// D3DXCreateFont - Stub (WorldBuilder only, not in MinGW build scope)
inline HRESULT D3DXCreateFont(
    LPDIRECT3DDEVICE8 pDevice,
    HFONT hFont,
    LPD3DXFONT* ppFont)
{
    // WorldBuilder not in build scope - stub acceptable
    if (ppFont) *ppFont = nullptr;
    return D3DERR_NOTAVAILABLE;
}

#endif // __cplusplus

#endif // NO_D3DX
