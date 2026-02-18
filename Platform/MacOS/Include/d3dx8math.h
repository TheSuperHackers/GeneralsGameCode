/* macOS shim: d3dx8math.h — D3DX math types and functions for macOS */
#pragma once

#include "d3d8_stub.h"
#include <math.h>

/*──────────────────── D3DX Math Types ────────────────────*/

struct D3DXVECTOR2 {
  float x, y;
  D3DXVECTOR2() : x(0), y(0) {}
  D3DXVECTOR2(float _x, float _y) : x(_x), y(_y) {}
};

struct D3DXVECTOR3 : public D3DVECTOR {
  D3DXVECTOR3() { x = y = z = 0.0f; }
  D3DXVECTOR3(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
  }
  D3DXVECTOR3(const D3DVECTOR &v) {
    x = v.x;
    y = v.y;
    z = v.z;
  }
};

struct D3DXVECTOR4 {
  float x, y, z, w;
  D3DXVECTOR4() : x(0), y(0), z(0), w(0) {}
  D3DXVECTOR4(float _x, float _y, float _z, float _w)
      : x(_x), y(_y), z(_z), w(_w) {}
};

struct D3DXMATRIX : public D3DMATRIX {
  D3DXMATRIX() { memset(this, 0, sizeof(*this)); }
  D3DXMATRIX(float _11, float _12, float _13, float _14, float _21, float _22,
             float _23, float _24, float _31, float _32, float _33, float _34,
             float _41, float _42, float _43, float _44) {
    m[0][0] = _11;
    m[0][1] = _12;
    m[0][2] = _13;
    m[0][3] = _14;
    m[1][0] = _21;
    m[1][1] = _22;
    m[1][2] = _23;
    m[1][3] = _24;
    m[2][0] = _31;
    m[2][1] = _32;
    m[2][2] = _33;
    m[2][3] = _34;
    m[3][0] = _41;
    m[3][1] = _42;
    m[3][2] = _43;
    m[3][3] = _44;
  }
  float &operator()(int row, int col) { return m[row][col]; }
  float operator()(int row, int col) const { return m[row][col]; }
};

struct D3DXQUATERNION {
  float x, y, z, w;
  D3DXQUATERNION() : x(0), y(0), z(0), w(1) {}
  D3DXQUATERNION(float _x, float _y, float _z, float _w)
      : x(_x), y(_y), z(_z), w(_w) {}
};

struct D3DXCOLOR {
  float r, g, b, a;
  D3DXCOLOR() : r(0), g(0), b(0), a(0) {}
  D3DXCOLOR(float _r, float _g, float _b, float _a)
      : r(_r), g(_g), b(_b), a(_a) {}
};

typedef D3DXVECTOR2 *LPD3DXVECTOR2;
typedef D3DXVECTOR3 *LPD3DXVECTOR3;
typedef D3DXVECTOR4 *LPD3DXVECTOR4;
typedef D3DXMATRIX *LPD3DXMATRIX;
typedef D3DXQUATERNION *LPD3DXQUATERNION;

/*──────────────────── D3DX Math Functions ────────────────────*/

/* Vec4 dot product */
inline FLOAT D3DXVec4Dot(const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2) {
  return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}

/* Vec4 transform by matrix */
inline D3DXVECTOR4 *D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV,
                                      const D3DXMATRIX *pM) {
  D3DXVECTOR4 tmp;
  tmp.x = pV->x * pM->m[0][0] + pV->y * pM->m[1][0] + pV->z * pM->m[2][0] +
          pV->w * pM->m[3][0];
  tmp.y = pV->x * pM->m[0][1] + pV->y * pM->m[1][1] + pV->z * pM->m[2][1] +
          pV->w * pM->m[3][1];
  tmp.z = pV->x * pM->m[0][2] + pV->y * pM->m[1][2] + pV->z * pM->m[2][2] +
          pV->w * pM->m[3][2];
  tmp.w = pV->x * pM->m[0][3] + pV->y * pM->m[1][3] + pV->z * pM->m[2][3] +
          pV->w * pM->m[3][3];
  *pOut = tmp;
  return pOut;
}

/* Vec3 transform by matrix (result is Vec4) */
inline D3DXVECTOR4 *D3DXVec3Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR3 *pV,
                                      const D3DXMATRIX *pM) {
  D3DXVECTOR4 v4(pV->x, pV->y, pV->z, 1.0f);
  return D3DXVec4Transform(pOut, &v4, pM);
}

/* Vec3 transform coord (divides by w) */
inline D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *pOut,
                                           const D3DXVECTOR3 *pV,
                                           const D3DXMATRIX *pM) {
  D3DXVECTOR4 tmp;
  D3DXVECTOR4 v4(pV->x, pV->y, pV->z, 1.0f);
  D3DXVec4Transform(&tmp, &v4, pM);
  float invW = (tmp.w != 0.0f) ? (1.0f / tmp.w) : 1.0f;
  pOut->x = tmp.x * invW;
  pOut->y = tmp.y * invW;
  pOut->z = tmp.z * invW;
  return pOut;
}

/* Vec3 normalize */
inline D3DXVECTOR3 *D3DXVec3Normalize(D3DXVECTOR3 *pOut,
                                      const D3DXVECTOR3 *pV) {
  float len = sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
  if (len > 0.0f) {
    float invLen = 1.0f / len;
    pOut->x = pV->x * invLen;
    pOut->y = pV->y * invLen;
    pOut->z = pV->z * invLen;
  } else {
    pOut->x = pOut->y = pOut->z = 0.0f;
  }
  return pOut;
}

/* Matrix multiply */
inline D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1,
                                      const D3DXMATRIX *pM2) {
  D3DXMATRIX tmp;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      tmp.m[i][j] = 0;
      for (int k = 0; k < 4; k++)
        tmp.m[i][j] += pM1->m[i][k] * pM2->m[k][j];
    }
  }
  *pOut = tmp;
  return pOut;
}

/* Matrix identity */
inline D3DXMATRIX *D3DXMatrixIdentity(D3DXMATRIX *pOut) {
  memset(pOut, 0, sizeof(D3DXMATRIX));
  pOut->m[0][0] = pOut->m[1][1] = pOut->m[2][2] = pOut->m[3][3] = 1.0f;
  return pOut;
}

/* operator* for D3DXMATRIX */
inline D3DXMATRIX operator*(const D3DXMATRIX &a, const D3DXMATRIX &b) {
  D3DXMATRIX out;
  D3DXMatrixMultiply(&out, &a, &b);
  return out;
}
