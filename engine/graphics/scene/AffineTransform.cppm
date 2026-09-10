module;
#include <array>
#include <cmath>
#include <cstddef>
#if defined(_M_X64) || defined(__SSE2__)
#include <xmmintrin.h>
#endif
export module Graphics.Scene.AffineTransform;
export import Graphics.Scene.RenderScene;

namespace Graphics {
export RenderTransform Affine_Identity() noexcept {
    return {{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
}

// Row-addressable application matrices need no dependency on their declaring
// library here. Only affine rows cross this boundary; the last row is explicit.
export template<class Matrix>
RenderTransform Import_Affine_Transform(const Matrix& source) noexcept {
    auto result=Affine_Identity();
    for(unsigned row=0;row<3;++row)
        for(unsigned column=0;column<4;++column)
            result.matrix[row*4+column]=source[row][column];
    return result;
}

export template<class Matrix>
Matrix Export_Affine_Transform(const RenderTransform& source) noexcept {
    Matrix result;
    for(unsigned row=0;row<3;++row)
        for(unsigned column=0;column<4;++column)
            result[row][column]=source.matrix[row*4+column];
    return result;
}

export RenderTransform Multiply_Affine(const RenderTransform& left,const RenderTransform& right) noexcept {
    auto result=Affine_Identity();
    const auto& a=left.matrix;const auto& b=right.matrix;
#if defined(_M_X64) || defined(__SSE2__)
    const auto first=_mm_loadu_ps(b.data());
    const auto second=_mm_loadu_ps(b.data()+4);
    const auto third=_mm_loadu_ps(b.data()+8);
    for(unsigned row=0;row<3;++row) {
        const auto base=row*4;
        const auto x=_mm_mul_ps(_mm_set1_ps(a[base]),first);
        const auto y=_mm_mul_ps(_mm_set1_ps(a[base+1]),second);
        const auto z=_mm_mul_ps(_mm_set1_ps(a[base+2]),third);
        _mm_storeu_ps(result.matrix.data()+base,_mm_add_ps(_mm_add_ps(x,y),z));
        // Preserve the scalar evaluation order. Only translation has a fourth
        // term; adding zero to the other lanes could change their signed zeros.
        result.matrix[base+3]+=a[base+3];
    }
#else
    for(unsigned row=0;row<3;++row) {
        const auto base=row*4;
        for(unsigned column=0;column<3;++column)
            result.matrix[base+column]=a[base]*b[column]+a[base+1]*b[4+column]+a[base+2]*b[8+column];
        result.matrix[base+3]=a[base]*b[3]+a[base+1]*b[7]+a[base+2]*b[11]+a[base+3];
    }
#endif
    return result;
}

// Inverts a full affine transform. On a singular or near-singular basis the
// destination is left untouched, allowing callers to retain their last valid
// cached inverse.
export bool Try_Invert_Affine(const RenderTransform& source,RenderTransform& destination) noexcept {
    const auto& m=source.matrix;
    const float a00=m[0],a01=m[1],a02=m[2];
    const float a10=m[4],a11=m[5],a12=m[6];
    const float a20=m[8],a21=m[9],a22=m[10];
    const float tx=m[3],ty=m[7],tz=m[11];

    const float cofactor00=a11*a22-a12*a21;
    const float cofactor01=a02*a21-a01*a22;
    const float cofactor02=a01*a12-a02*a11;
    const float cofactor10=a12*a20-a10*a22;
    const float cofactor11=a00*a22-a02*a20;
    const float cofactor12=a02*a10-a00*a12;
    const float cofactor20=a10*a21-a11*a20;
    const float cofactor21=a01*a20-a00*a21;
    const float cofactor22=a00*a11-a01*a10;
    const float determinant=a00*cofactor00+a01*cofactor10+a02*cofactor20;
    if (std::fabs(determinant)<1e-8f) return false;

    const float inverse_determinant=1.0f/determinant;
    auto& out=destination.matrix;
    out[0]=cofactor00*inverse_determinant;
    out[1]=cofactor01*inverse_determinant;
    out[2]=cofactor02*inverse_determinant;
    out[4]=cofactor10*inverse_determinant;
    out[5]=cofactor11*inverse_determinant;
    out[6]=cofactor12*inverse_determinant;
    out[8]=cofactor20*inverse_determinant;
    out[9]=cofactor21*inverse_determinant;
    out[10]=cofactor22*inverse_determinant;
    out[3]=-(out[0]*tx+out[1]*ty+out[2]*tz);
    out[7]=-(out[4]*tx+out[5]*ty+out[6]*tz);
    out[11]=-(out[8]*tx+out[9]*ty+out[10]*tz);
    out[12]=0;out[13]=0;out[14]=0;out[15]=1;
    return true;
}

export void Translate_Affine(RenderTransform& transform,const std::array<float,3>& translation) noexcept {
    auto& m=transform.matrix;
    for(unsigned row=0;row<3;++row) {
        const auto base=row*4;
        m[base+3]+=m[base]*translation[0]+m[base+1]*translation[1]+m[base+2]*translation[2];
    }
}

// Sampling owns quaternion normalization/interpolation. Do not renormalize
// here: authored values and sampled approximations must retain their matrix.
export RenderTransform Quaternion_Affine(const std::array<float,4>& q) noexcept {
    auto result=Affine_Identity();auto& m=result.matrix;
    m[0]=float(1.0-2.0*(q[1]*q[1]+q[2]*q[2]));
    m[1]=float(2.0*(q[0]*q[1]-q[2]*q[3]));
    m[2]=float(2.0*(q[2]*q[0]+q[1]*q[3]));
    m[4]=float(2.0*(q[0]*q[1]+q[2]*q[3]));
    m[5]=float(1.0-2.0f*(q[2]*q[2]+q[0]*q[0]));
    m[6]=float(2.0*(q[1]*q[2]-q[0]*q[3]));
    m[8]=float(2.0*(q[2]*q[0]-q[1]*q[3]));
    m[9]=float(2.0*(q[1]*q[2]+q[0]*q[3]));
    m[10]=float(1.0-2.0*(q[1]*q[1]+q[0]*q[0]));
    return result;
}
}
