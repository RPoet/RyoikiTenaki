#pragma once

#include <cmath>
#include <algorithm>
#include <immintrin.h> // Intel intrinsics (SSE, AVX)

namespace SIMDMath
{
    // ============================================================================
    // SIMDMath Library
    // Convention: Row-Major matrices, Row-Vectors (v * M), Left-Handed coordinate system
    // This matches DirectX/HLSL conventions.
    // ============================================================================

    static const float PI = 3.14159265358979323846f;
    static const float DegToRad = PI / 180.0f;
    static const float RadToDeg = 180.0f / PI;

    // Forward declarations
    struct Vector3;
    struct Vector4;
    struct Matrix4x4;
    struct Quaternion;

    // --- Vector2 ---
    struct Vector2
    {
        float x, y;
        Vector2() : x(0), y(0) {}
        Vector2(float _x, float _y) : x(_x), y(_y) {}
        
        inline Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
        inline Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
        inline Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
        inline Vector2 operator/(float scalar) const { return Vector2(x / scalar, y / scalar); }
    };

    // --- Vector3 ---
    // Aligned to 16 bytes for SIMD operations even though it only uses xyz.
    struct alignas(16) Vector3
    {
        union
        {
            struct { float x, y, z; };
            float data[3];
            __m128 mmvalue; // Internal SIMD register (4th component is padding, set to 0)
        };

        Vector3() : mmvalue(_mm_setzero_ps()) {}
        Vector3(float _x, float _y, float _z) : mmvalue(_mm_set_ps(0.0f, _z, _y, _x)) {}
        explicit Vector3(__m128 v) : mmvalue(v) {}

        static Vector3 Zero() { return Vector3(0, 0, 0); }
        static Vector3 One() { return Vector3(1, 1, 1); }
        static Vector3 Forward() { return Vector3(0, 0, 1); }  // +Z in LH
        static Vector3 Up() { return Vector3(0, 1, 0); }
        static Vector3 Right() { return Vector3(1, 0, 0); }

        inline Vector3 operator+(const Vector3& other) const { return Vector3(_mm_add_ps(mmvalue, other.mmvalue)); }
        inline Vector3 operator-(const Vector3& other) const { return Vector3(_mm_sub_ps(mmvalue, other.mmvalue)); }
        inline Vector3 operator*(float scalar) const { return Vector3(_mm_mul_ps(mmvalue, _mm_set1_ps(scalar))); }
        inline Vector3 operator/(float scalar) const { return Vector3(_mm_div_ps(mmvalue, _mm_set1_ps(scalar))); }

        inline Vector3& operator+=(const Vector3& other) { mmvalue = _mm_add_ps(mmvalue, other.mmvalue); return *this; }
        inline Vector3& operator-=(const Vector3& other) { mmvalue = _mm_sub_ps(mmvalue, other.mmvalue); return *this; }
        inline Vector3& operator*=(float scalar) { mmvalue = _mm_mul_ps(mmvalue, _mm_set1_ps(scalar)); return *this; }

        static float Dot(const Vector3& a, const Vector3& b)
        {
            // x*x + y*y + z*z (ignore w which should be 0)
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        static Vector3 Cross(const Vector3& a, const Vector3& b)
        {
            // (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x)
            return Vector3(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }

        static Vector3 Normalize(const Vector3& v)
        {
            float lenSq = Dot(v, v);
            if (lenSq < 1e-8f) return Zero();
            float invLen = 1.0f / std::sqrt(lenSq);
            return v * invLen;
        }

        float Length() const { return std::sqrt(Dot(*this, *this)); }
        float LengthSquared() const { return Dot(*this, *this); }
    };

    // --- Vector4 ---
    struct alignas(16) Vector4
    {
        union
        {
            struct { float x, y, z, w; };
            float data[4];
            __m128 mmvalue;
        };

        Vector4() : mmvalue(_mm_setzero_ps()) {}
        Vector4(float _x, float _y, float _z, float _w) : mmvalue(_mm_set_ps(_w, _z, _y, _x)) {}
        explicit Vector4(__m128 v) : mmvalue(v) {}
        
        static Vector4 Zero() { return Vector4(0,0,0,0); }

        inline Vector4 operator+(const Vector4& other) const { return Vector4(_mm_add_ps(mmvalue, other.mmvalue)); }
        inline Vector4 operator-(const Vector4& other) const { return Vector4(_mm_sub_ps(mmvalue, other.mmvalue)); }
        inline Vector4 operator*(float scalar) const { return Vector4(_mm_mul_ps(mmvalue, _mm_set1_ps(scalar))); }
        inline Vector4 operator/(float scalar) const { return Vector4(_mm_div_ps(mmvalue, _mm_set1_ps(scalar))); }
    };

    // --- Matrix4x4 ---
    // Row-Major storage: m[row][col], r[row] contains the entire row as __m128
    // Memory layout: [_11,_12,_13,_14, _21,_22,_23,_24, _31,_32,_33,_34, _41,_42,_43,_44]
    // Convention: Row-vector on left (v * M)
    struct alignas(16) Matrix4x4
    {
        union
        {
            struct
            {
                float _11, _12, _13, _14;  // Row 0
                float _21, _22, _23, _24;  // Row 1
                float _31, _32, _33, _34;  // Row 2
                float _41, _42, _43, _44;  // Row 3
            };
            float m[4][4];  // m[row][col]
            __m128 r[4];    // r[row]
        };

        Matrix4x4()
        {
            r[0] = _mm_setzero_ps();
            r[1] = _mm_setzero_ps();
            r[2] = _mm_setzero_ps();
            r[3] = _mm_setzero_ps();
        }

        // Identity matrix
        static Matrix4x4 Identity()
        {
            Matrix4x4 res;
            res._11 = 1.0f; res._12 = 0.0f; res._13 = 0.0f; res._14 = 0.0f;
            res._21 = 0.0f; res._22 = 1.0f; res._23 = 0.0f; res._24 = 0.0f;
            res._31 = 0.0f; res._32 = 0.0f; res._33 = 1.0f; res._34 = 0.0f;
            res._41 = 0.0f; res._42 = 0.0f; res._43 = 0.0f; res._44 = 1.0f;
            return res;
        }
        
        // Translation matrix (translation in row 3 for row-vector convention)
        static Matrix4x4 Translation(float x, float y, float z)
        {
            Matrix4x4 res = Identity();
            res._41 = x; res._42 = y; res._43 = z;
            return res;
        }

        // Scale matrix
        static Matrix4x4 Scale(float x, float y, float z)
        {
            Matrix4x4 res = Identity();
            res._11 = x; res._22 = y; res._33 = z;
            return res;
        }

        // Rotation around X-axis (Left-Handed, row-major)
        // Positive angle rotates Y toward Z
        static Matrix4x4 RotationX(float angleRad)
        {
            Matrix4x4 res = Identity();
            float c = cosf(angleRad);
            float s = sinf(angleRad);
            // | 1   0     0    0 |
            // | 0   c    -s    0 |
            // | 0   s     c    0 |
            // | 0   0     0    1 |
            res._22 = c;  res._23 = -s;
            res._32 = s;  res._33 = c;
            return res;
        }

        // Rotation around Y-axis (Left-Handed, row-major)
        // Positive angle rotates Z toward X
        static Matrix4x4 RotationY(float angleRad)
        {
            Matrix4x4 res = Identity();
            float c = cosf(angleRad);
            float s = sinf(angleRad);
            // |  c   0   s   0 |
            // |  0   1   0   0 |
            // | -s   0   c   0 |
            // |  0   0   0   1 |
            res._11 = c;  res._13 = s;
            res._31 = -s; res._33 = c;
            return res;
        }

        // Rotation around Z-axis (Left-Handed, row-major)
        // Positive angle rotates X toward Y
        static Matrix4x4 RotationZ(float angleRad)
        {
            Matrix4x4 res = Identity();
            float c = cosf(angleRad);
            float s = sinf(angleRad);
            // | c  -s   0   0 |
            // | s   c   0   0 |
            // | 0   0   1   0 |
            // | 0   0   0   1 |
            res._11 = c;  res._12 = -s;
            res._21 = s;  res._22 = c;
            return res;
        }

        // Rotation from Euler angles (matches XMMatrixRotationRollPitchYaw)
        // Order: Roll (Z) * Pitch (X) * Yaw (Y)
        static Matrix4x4 RotationRollPitchYaw(float pitch, float yaw, float roll)
        {
            // Combined rotation: first Yaw (around Y), then Pitch (around X), then Roll (around Z)
            // For row-vectors: v * Ry * Rx * Rz
            // Which means: RotationY(yaw) * RotationX(pitch) * RotationZ(roll)
            return RotationY(yaw) * RotationX(pitch) * RotationZ(roll);
        }

        // Left-Handed Perspective Projection (matches XMMatrixPerspectiveFovLH)
        static Matrix4x4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
        {
            float SinFov = sinf(0.5f * FovAngleY);
            float CosFov = cosf(0.5f * FovAngleY);
            float Height = CosFov / SinFov;  // cot(fov/2)
            float Width = Height / AspectRatio;
            float fRange = FarZ / (FarZ - NearZ);

            Matrix4x4 M;
            // Row-major perspective matrix for LH:
            // | w   0   0           0 |
            // | 0   h   0           0 |
            // | 0   0   f/(f-n)     1 |
            // | 0   0   -nf/(f-n)   0 |
            M._11 = Width;
            M._12 = 0.0f;
            M._13 = 0.0f;
            M._14 = 0.0f;

            M._21 = 0.0f;
            M._22 = Height;
            M._23 = 0.0f;
            M._24 = 0.0f;

            M._31 = 0.0f;
            M._32 = 0.0f;
            M._33 = fRange;
            M._34 = 1.0f;

            M._41 = 0.0f;
            M._42 = 0.0f;
            M._43 = -fRange * NearZ;
            M._44 = 0.0f;
            
            return M;
        }

        // Helpers for 3D cross/dot in SIMD (w is ignored)
        static inline __m128 Cross3(__m128 a, __m128 b)
        {
            const __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
            const __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
            const __m128 c = _mm_sub_ps(_mm_mul_ps(a, b_yzx), _mm_mul_ps(a_yzx, b));
            return _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
        }

        static inline float Dot3(__m128 a, __m128 b)
        {
            const __m128 mul = _mm_mul_ps(a, b);
            const __m128 shuf1 = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 1, 0, 3));
            const __m128 sum1 = _mm_add_ps(mul, shuf1);
            const __m128 shuf2 = _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(1, 0, 3, 2));
            const __m128 sum2 = _mm_add_ps(sum1, shuf2);
            return _mm_cvtss_f32(sum2);
        }

        static inline bool IsAffine(const Matrix4x4& M, float eps = 1e-6f)
        {
            return (fabsf(M._14) < eps) && (fabsf(M._24) < eps) && (fabsf(M._34) < eps) && (fabsf(M._44 - 1.0f) < eps);
        }

        // Affine inverse (row-vector convention):
        // [ R  0 ]^{-1} = [ R^{-1}  0 ]
        // [ T  1 ]        [ -T*R^{-1} 1 ]
        static Matrix4x4 InverseAffine(const Matrix4x4& M)
        {
            // Columns of R (3x3)
            const __m128 c0 = _mm_set_ps(0.0f, M._31, M._21, M._11);
            const __m128 c1 = _mm_set_ps(0.0f, M._32, M._22, M._12);
            const __m128 c2 = _mm_set_ps(0.0f, M._33, M._23, M._13);

            __m128 ic0 = Cross3(c1, c2);
            __m128 ic1 = Cross3(c2, c0);
            __m128 ic2 = Cross3(c0, c1);

            const float det = Dot3(c0, ic0);
            if (fabsf(det) < 1e-10f)
            {
                return Identity();
            }

            const __m128 invDet = _mm_set1_ps(1.0f / det);
            ic0 = _mm_mul_ps(ic0, invDet);
            ic1 = _mm_mul_ps(ic1, invDet);
            ic2 = _mm_mul_ps(ic2, invDet);

            alignas(16) float v0[4];
            alignas(16) float v1[4];
            alignas(16) float v2[4];
            _mm_store_ps(v0, ic0);
            _mm_store_ps(v1, ic1);
            _mm_store_ps(v2, ic2);

            Matrix4x4 R = Identity();
            // Set columns into row-major storage
            R._11 = v0[0]; R._21 = v0[1]; R._31 = v0[2];
            R._12 = v1[0]; R._22 = v1[1]; R._32 = v1[2];
            R._13 = v2[0]; R._23 = v2[1]; R._33 = v2[2];

            // Translation row: -T * R^{-1}
            const __m128 T = _mm_set_ps(0.0f, M._43, M._42, M._41);
            const float tx = Dot3(T, ic0);
            const float ty = Dot3(T, ic1);
            const float tz = Dot3(T, ic2);
            R._41 = -tx;
            R._42 = -ty;
            R._43 = -tz;
            R._44 = 1.0f;

            return R;
        }

        // General 4x4 inverse via Gauss-Jordan elimination (SIMD row ops)
        static Matrix4x4 InverseGeneral(const Matrix4x4& M)
        {
            __m128 rowM[4] = { M.r[0], M.r[1], M.r[2], M.r[3] };
            Matrix4x4 I = Identity();
            __m128 rowI[4] = { I.r[0], I.r[1], I.r[2], I.r[3] };

            alignas(16) float tmp[4];
            for (int i = 0; i < 4; ++i)
            {
                // Pivot selection
                int pivot = i;
                float pivotVal = 0.0f;
                for (int r = i; r < 4; ++r)
                {
                    _mm_store_ps(tmp, rowM[r]);
                    const float val = fabsf(tmp[i]);
                    if (val > pivotVal)
                    {
                        pivotVal = val;
                        pivot = r;
                    }
                }

                if (pivotVal < 1e-10f)
                {
                    return Identity();
                }

                if (pivot != i)
                {
                    std::swap(rowM[i], rowM[pivot]);
                    std::swap(rowI[i], rowI[pivot]);
                }

                _mm_store_ps(tmp, rowM[i]);
                const float pivotElem = tmp[i];
                const __m128 invPivot = _mm_set1_ps(1.0f / pivotElem);
                rowM[i] = _mm_mul_ps(rowM[i], invPivot);
                rowI[i] = _mm_mul_ps(rowI[i], invPivot);

                for (int r = 0; r < 4; ++r)
                {
                    if (r == i) continue;
                    _mm_store_ps(tmp, rowM[r]);
                    const float factor = tmp[i];
                    const __m128 f = _mm_set1_ps(factor);
                    rowM[r] = _mm_sub_ps(rowM[r], _mm_mul_ps(f, rowM[i]));
                    rowI[r] = _mm_sub_ps(rowI[r], _mm_mul_ps(f, rowI[i]));
                }
            }

            Matrix4x4 R;
            R.r[0] = rowI[0];
            R.r[1] = rowI[1];
            R.r[2] = rowI[2];
            R.r[3] = rowI[3];
            return R;
        }

        // Inverse selector: use affine fast-path when possible, otherwise general
        static Matrix4x4 Inverse(const Matrix4x4& M)
        {
            if (IsAffine(M))
            {
                return InverseAffine(M);
            }
            return InverseGeneral(M);
        }

        // Matrix multiplication (A * B)
        // For row-vectors: v * (A * B) = (v * A) * B
        inline Matrix4x4 operator*(const Matrix4x4& B) const
        {
            Matrix4x4 res;
            const __m128 b0 = B.r[0];
            const __m128 b1 = B.r[1];
            const __m128 b2 = B.r[2];
            const __m128 b3 = B.r[3];

            for (int i = 0; i < 4; i++)
            {
                const __m128 a = r[i];
                const __m128 a0 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0));
                const __m128 a1 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1));
                const __m128 a2 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 2, 2));
                const __m128 a3 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 3, 3));

                const __m128 r01 = _mm_add_ps(_mm_mul_ps(a0, b0), _mm_mul_ps(a1, b1));
                const __m128 r23 = _mm_add_ps(_mm_mul_ps(a2, b2), _mm_mul_ps(a3, b3));
                res.r[i] = _mm_add_ps(r01, r23);
            }
            return res;
        }

        // Transpose
        static Matrix4x4 Transpose(const Matrix4x4& M)
        {
            Matrix4x4 R;
            __m128 r0 = M.r[0];
            __m128 r1 = M.r[1];
            __m128 r2 = M.r[2];
            __m128 r3 = M.r[3];
            _MM_TRANSPOSE4_PS(r0, r1, r2, r3);
            R.r[0] = r0;
            R.r[1] = r1;
            R.r[2] = r2;
            R.r[3] = r3;
            return R;
        }
    };
    
    // --- Quaternion ---
    struct alignas(16) Quaternion
    {
        union {
            struct { float x, y, z, w; };
            float data[4];
            __m128 mmvalue;
        };

        Quaternion() : x(0), y(0), z(0), w(1) {} // Identity
        Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
        
        static Quaternion Identity() { return Quaternion(0, 0, 0, 1); }
        
        // Create quaternion from Euler angles (Pitch, Yaw, Roll)
        // Matches DirectX convention
        static Quaternion FromRollPitchYaw(float pitch, float yaw, float roll)
        {
            float cy = cosf(yaw * 0.5f);
            float sy = sinf(yaw * 0.5f);
            float cp = cosf(pitch * 0.5f);
            float sp = sinf(pitch * 0.5f);
            float cr = cosf(roll * 0.5f);
            float sr = sinf(roll * 0.5f);

            Quaternion q;
            q.w = cr * cp * cy + sr * sp * sy;
            q.x = sr * cp * cy - cr * sp * sy;
            q.y = cr * sp * cy + sr * cp * sy;
            q.z = cr * cp * sy - sr * sp * cy;
            return q;
        }

        // Convert quaternion to rotation matrix
        Matrix4x4 ToMatrix() const
        {
            Matrix4x4 R = Matrix4x4::Identity();
            
            float xx = x * x;
            float yy = y * y;
            float zz = z * z;
            float xy = x * y;
            float xz = x * z;
            float yz = y * z;
            float wx = w * x;
            float wy = w * y;
            float wz = w * z;

            R._11 = 1.0f - 2.0f * (yy + zz);
            R._12 = 2.0f * (xy - wz);
            R._13 = 2.0f * (xz + wy);

            R._21 = 2.0f * (xy + wz);
            R._22 = 1.0f - 2.0f * (xx + zz);
            R._23 = 2.0f * (yz - wx);

            R._31 = 2.0f * (xz - wy);
            R._32 = 2.0f * (yz + wx);
            R._33 = 1.0f - 2.0f * (xx + yy);

            return R;
        }
    };
    
    // ============================================================================
    // Transform Helper Functions
    // These use ROW-VECTOR convention: result = v * M
    // ============================================================================

    // Transform a point (w=1) by a matrix
    inline Vector3 TransformPoint(const Vector3& v, const Matrix4x4& M)
    {
        const __m128 vec = _mm_set_ps(1.0f, v.z, v.y, v.x);
        const __m128 x = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0));
        const __m128 y = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1));
        const __m128 z = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2));
        const __m128 w = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3));

        const __m128 r01 = _mm_add_ps(_mm_mul_ps(x, M.r[0]), _mm_mul_ps(y, M.r[1]));
        const __m128 r23 = _mm_add_ps(_mm_mul_ps(z, M.r[2]), _mm_mul_ps(w, M.r[3]));
        const __m128 res = _mm_add_ps(r01, r23);

        alignas(16) float out[4];
        _mm_store_ps(out, res);
        return Vector3(out[0], out[1], out[2]);
    }

    // Transform a vector (w=0) by a matrix (ignores translation)
    inline Vector3 TransformVector(const Vector3& v, const Matrix4x4& M)
    {
        const __m128 vec = _mm_set_ps(0.0f, v.z, v.y, v.x);
        const __m128 x = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0));
        const __m128 y = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1));
        const __m128 z = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2));

        const __m128 r01 = _mm_add_ps(_mm_mul_ps(x, M.r[0]), _mm_mul_ps(y, M.r[1]));
        const __m128 res = _mm_add_ps(r01, _mm_mul_ps(z, M.r[2]));

        alignas(16) float out[4];
        _mm_store_ps(out, res);
        return Vector3(out[0], out[1], out[2]);
    }

    // Transform Vector4 by matrix
    inline Vector4 Transform(const Vector4& v, const Matrix4x4& M)
    {
        const __m128 vec = v.mmvalue;
        const __m128 x = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0));
        const __m128 y = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1));
        const __m128 z = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2));
        const __m128 w = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3));

        const __m128 r01 = _mm_add_ps(_mm_mul_ps(x, M.r[0]), _mm_mul_ps(y, M.r[1]));
        const __m128 r23 = _mm_add_ps(_mm_mul_ps(z, M.r[2]), _mm_mul_ps(w, M.r[3]));
        return Vector4(_mm_add_ps(r01, r23));
    }

} // namespace SIMDMath
