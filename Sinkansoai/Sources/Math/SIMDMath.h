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

        // General 4x4 Matrix Inverse using Cramer's rule
        static Matrix4x4 Inverse(const Matrix4x4& M)
        {
            const float* m = &M.m[0][0];
            float inv[16];

            inv[0] = m[5]  * m[10] * m[15] - 
                     m[5]  * m[11] * m[14] - 
                     m[9]  * m[6]  * m[15] + 
                     m[9]  * m[7]  * m[14] +
                     m[13] * m[6]  * m[11] - 
                     m[13] * m[7]  * m[10];

            inv[4] = -m[4]  * m[10] * m[15] + 
                      m[4]  * m[11] * m[14] + 
                      m[8]  * m[6]  * m[15] - 
                      m[8]  * m[7]  * m[14] - 
                      m[12] * m[6]  * m[11] + 
                      m[12] * m[7]  * m[10];

            inv[8] = m[4]  * m[9]  * m[15] - 
                     m[4]  * m[11] * m[13] - 
                     m[8]  * m[5]  * m[15] + 
                     m[8]  * m[7]  * m[13] + 
                     m[12] * m[5]  * m[11] - 
                     m[12] * m[7]  * m[9];

            inv[12] = -m[4]  * m[9]  * m[14] + 
                       m[4]  * m[10] * m[13] +
                       m[8]  * m[5]  * m[14] - 
                       m[8]  * m[6]  * m[13] - 
                       m[12] * m[5]  * m[10] + 
                       m[12] * m[6]  * m[9];

            inv[1] = -m[1]  * m[10] * m[15] + 
                      m[1]  * m[11] * m[14] + 
                      m[9]  * m[2]  * m[15] - 
                      m[9]  * m[3]  * m[14] - 
                      m[13] * m[2]  * m[11] + 
                      m[13] * m[3]  * m[10];

            inv[5] = m[0]  * m[10] * m[15] - 
                     m[0]  * m[11] * m[14] - 
                     m[8]  * m[2]  * m[15] + 
                     m[8]  * m[3]  * m[14] + 
                     m[12] * m[2]  * m[11] - 
                     m[12] * m[3]  * m[10];

            inv[9] = -m[0]  * m[9]  * m[15] + 
                      m[0]  * m[11] * m[13] + 
                      m[8]  * m[1]  * m[15] - 
                      m[8]  * m[3]  * m[13] - 
                      m[12] * m[1]  * m[11] + 
                      m[12] * m[3]  * m[9];

            inv[13] = m[0]  * m[9]  * m[14] - 
                      m[0]  * m[10] * m[13] - 
                      m[8]  * m[1]  * m[14] + 
                      m[8]  * m[2]  * m[13] + 
                      m[12] * m[1]  * m[10] - 
                      m[12] * m[2]  * m[9];

            inv[2] = m[1]  * m[6] * m[15] - 
                     m[1]  * m[7] * m[14] - 
                     m[5]  * m[2] * m[15] + 
                     m[5]  * m[3] * m[14] + 
                     m[13] * m[2] * m[7] - 
                     m[13] * m[3] * m[6];

            inv[6] = -m[0]  * m[6] * m[15] + 
                      m[0]  * m[7] * m[14] + 
                      m[4]  * m[2] * m[15] - 
                      m[4]  * m[3] * m[14] - 
                      m[12] * m[2] * m[7] + 
                      m[12] * m[3] * m[6];

            inv[10] = m[0]  * m[5] * m[15] - 
                      m[0]  * m[7] * m[13] - 
                      m[4]  * m[1] * m[15] + 
                      m[4]  * m[3] * m[13] + 
                      m[12] * m[1] * m[7] - 
                      m[12] * m[3] * m[5];

            inv[14] = -m[0]  * m[5] * m[14] + 
                       m[0]  * m[6] * m[13] + 
                       m[4]  * m[1] * m[14] - 
                       m[4]  * m[2] * m[13] - 
                       m[12] * m[1] * m[6] + 
                       m[12] * m[2] * m[5];

            inv[3] = -m[1] * m[6] * m[11] + 
                      m[1] * m[7] * m[10] + 
                      m[5] * m[2] * m[11] - 
                      m[5] * m[3] * m[10] - 
                      m[9] * m[2] * m[7] + 
                      m[9] * m[3] * m[6];

            inv[7] = m[0] * m[6] * m[11] - 
                     m[0] * m[7] * m[10] - 
                     m[4] * m[2] * m[11] + 
                     m[4] * m[3] * m[10] + 
                     m[8] * m[2] * m[7] - 
                     m[8] * m[3] * m[6];

            inv[11] = -m[0] * m[5] * m[11] + 
                       m[0] * m[7] * m[9] + 
                       m[4] * m[1] * m[11] - 
                       m[4] * m[3] * m[9] - 
                       m[8] * m[1] * m[7] + 
                       m[8] * m[3] * m[5];

            inv[15] = m[0] * m[5] * m[10] - 
                      m[0] * m[6] * m[9] - 
                      m[4] * m[1] * m[10] + 
                      m[4] * m[2] * m[9] + 
                      m[8] * m[1] * m[6] - 
                      m[8] * m[2] * m[5];

            float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

            if (fabsf(det) < 1e-10f)
            {
                return Identity();
            }

            float invDet = 1.0f / det;

            Matrix4x4 R;
            for (int i = 0; i < 16; i++)
            {
                (&R.m[0][0])[i] = inv[i] * invDet;
            }

            return R;
        }

        // Matrix multiplication (A * B)
        // For row-vectors: v * (A * B) = (v * A) * B
        inline Matrix4x4 operator*(const Matrix4x4& B) const
        {
            Matrix4x4 res;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    res.m[i][j] = m[i][0] * B.m[0][j] +
                                  m[i][1] * B.m[1][j] +
                                  m[i][2] * B.m[2][j] +
                                  m[i][3] * B.m[3][j];
                }
            }
            return res;
        }

        // Transpose
        static Matrix4x4 Transpose(const Matrix4x4& M)
        {
            Matrix4x4 R;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    R.m[i][j] = M.m[j][i];
                }
            }
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
        Vector3 result;
        result.x = v.x * M._11 + v.y * M._21 + v.z * M._31 + M._41;
        result.y = v.x * M._12 + v.y * M._22 + v.z * M._32 + M._42;
        result.z = v.x * M._13 + v.y * M._23 + v.z * M._33 + M._43;
        return result;
    }

    // Transform a vector (w=0) by a matrix (ignores translation)
    inline Vector3 TransformVector(const Vector3& v, const Matrix4x4& M)
    {
        Vector3 result;
        result.x = v.x * M._11 + v.y * M._21 + v.z * M._31;
        result.y = v.x * M._12 + v.y * M._22 + v.z * M._32;
        result.z = v.x * M._13 + v.y * M._23 + v.z * M._33;
        return result;
    }

    // Transform Vector4 by matrix
    inline Vector4 Transform(const Vector4& v, const Matrix4x4& M)
    {
        Vector4 result;
        result.x = v.x * M._11 + v.y * M._21 + v.z * M._31 + v.w * M._41;
        result.y = v.x * M._12 + v.y * M._22 + v.z * M._32 + v.w * M._42;
        result.z = v.x * M._13 + v.y * M._23 + v.z * M._33 + v.w * M._43;
        result.w = v.x * M._14 + v.y * M._24 + v.z * M._34 + v.w * M._44;
        return result;
    }

} // namespace SIMDMath
