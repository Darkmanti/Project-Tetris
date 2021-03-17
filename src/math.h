#pragma once
// inspired by hypermath
// https://github.com/swarzzy/sokoban/blob/master/src/hypermath.h
#include <stdint.h>
#include <math.h>

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef float		f32;
typedef double		f64;

const f32 PI_32 = 3.14159265358979323846f;
const f64 PI_64 = 3.14159265358979323846;


// FUN to TETRIS



// Other math fun

inline f32 Sqrt(f32 val)
{
    return sqrtf(val);
}

inline f32 Sqr(f32 val)
{
    return val * val;
}

inline f32 Pow(f32 base, f32 exp)
{
    return powf(base, exp);
}

inline f32 Sin(f32 rad)
{
    return sinf(rad);
}

inline f32 Cos(f32 rad)
{
    return cosf(rad);
}

inline f32 Tan(f32 rad)
{
    return tanf(rad);
}

inline f32 Acos(f32 val)
{
    return acosf(val);
}

inline f32 Atan(f32 val)
{
    return atanf(val);
}

inline f32 ToRad(f32 deg)
{
    return PI_32 / 180.0f * deg;
}

inline f32 ToDeg(f32 rad)
{
    return 180.0f / PI_32 * rad;
}

inline f32 Abs(f32 val)
{
    return val >= 0.0f ? val : -val;
}

inline i32 Abs(i32 val)
{
    return val >= 0 ? val : -val;
}

inline i32 Round(f32 val)
{
    return (i32)roundf(val);
}

inline i32 Ceil(f32 val)
{
    return (i32)ceilf(val);
}

inline i32 Floor(f32 val)
{
    return (i32)floorf(val);
}

// NOTE: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
inline u32 RoundUoToNextPowerOfTwo(u32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

// VEC and MATrix Math fun and structure
// NOTE : matrix index on columns

union v2
{
    struct
    {
        f32 x;
        f32 y;
    };
    struct
    {
        f32 u;
        f32 v;
    };
    f32 data[2];
};

union v3
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };
    struct
    {
        v2 xy;
        f32 z;
    };
    f32 data[3];
};

union v4
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    struct
    {
        v2 xy;
        v2 zw;
    };
    struct 
    {
        v2 xy;
        f32 z;
        f32 w;
    };
    struct
    {
        v3 xyz;
        f32 w;
    };
    f32 data[4];
};

union m4
{
    v4 columns[4];
    f32 data[16];
    struct
    {
        f32 _11, _21, _31, _41;
        f32 _12, _22, _32, _42;
        f32 _13, _23, _33, _43;
        f32 _14, _24, _34, _44;
    };
};

union m3
{
    v3 columns[3];
    f32 data[9];
    struct
    {
        f32 _11, _21, _31;
        f32 _12, _22, _32;
        f32 _13, _23, _33;
    };
};

union m2
{
    v2 columns[2];
    f32 data[4];
    struct
    {
        f32 _11, _21;
        f32 _12, _22;
    };
};

// TODO: Think about simple name, when this will be use
union Quaternion
{
    struct
    {
        f32 x, y, z, w;
    };
    struct
    {
        v3 xyz;
        f32 w;
    };
};

// Vector fun

inline v2 V2(f32 x, f32 y)
{
    return v2{ x, y };
}

inline v2 V2(f32 val)
{
    return v2{ val, val };
}

inline v3 V3(f32 x, f32 y, f32 z)
{
    return v3{ x, y, z };
}

inline v3 V3(f32 val)
{
    return v3{ val, val, val };
}

inline v3 V3(v2 xy, f32 z)
{
    return v3{ xy.x, xy.y, z };
}

inline v3 V3(v4 xyzw)
{
    return v3{ xyzw.x, xyzw.y, xyzw.z };
}

inline v4 V4(f32 x, f32 y, f32 z, f32 w)
{
    return v4{ x, y, z, w };
}

inline v4 V4(v2 xy, f32 z, f32 w)
{
    return v4{ xy.x, xy.y, z, w };
}

inline v4 V4(v3 xyz, f32 w)
{
    return v4{ xyz.x, xyz.y, xyz.z, w };
}

inline v2 operator+(v2 r, v2 l)
{
    return v2{ r.x + l.x, r.y + l.y };
}

inline v2 operator+(v2 l, f32 q)
{
    return v2{ l.x + q, l.y + q };
}

inline v2& operator+=(v2& l, v2 r)
{
    l.x += r.x;
    l.y += r.y;
    return l;
}

inline v2& operator+=(v2& l, f32 q)
{
    l.x += q;
    l.y += q;
    return l;
}

inline v2 operator-(v2 l, v2 r)
{
    return v2{ l.x - r.x, l.y - r.y };
}

inline v2 operator-(v2 l, f32 q)
{
    return v2{l.x - q, l.y - q};
}

inline v2& operator-=(v2& l, v2 r)
{
    l.x -= r.x;
    l.y -= r.y;
    return l;
}

inline v2& operator-=(v2& l, f32 q)
{
    l.x -= q;
    l.y -= q;
    return l;
}

inline v2& operator-(v2 l)
{
    l.x = -l.x;
    l.y = -l.y;
    return l;
}

inline v2 operator*(v2 l, f32 q)
{
    return v2{ l.x * q, l.y * q };
}

inline v2& operator*=(v2& l, f32 q)
{
    l.x *= q;
    l.y *= q;
    return l;
}

inline v2 operator/(v2 l, f32 q)
{
    return v2{ l.x / q, l.y / q };
}

inline v2& operator/=(v2& l, f32 q)
{
    l.x /= q;
    l.y /= q;
    return l;
}

inline v3 operator+(v3 l, v3 r)
{
    return v3{ l.x + r.x, l.y + r.y, l.z + r.z };
}

inline v3 operator+(v3 l, f32 q)
{
    return v3{ l.x + q, l.y + q, l.z + q };
}

inline v3& operator+=(v3 l, v3 r)
{
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
    return l;
}

inline v3& operator+=(v3 l, f32 q)
{
    l.x += q;
    l.y += q;
    l.z += q;
    return l;
}

inline v3 operator-(v3 l, v3 r)
{
    return v3{ l.x - r.x, l.y - r.y, l.z - r.z };
}

inline v3 operator-(v3 l, f32 q)
{
    return v3{ l.x - q, l.y - q, l.z - q };
}

inline v3& operator-=(v3 l, v3 r)
{
    l.x -= r.x;
    l.y -= r.y;
    l.z -= r.z;
    return l;
}

inline v3& operator-=(v3 l, f32 q)
{
    l.x -= q;
    l.y -= q;
    l.z -= q;
    return l;
}

inline v3 operator-(v3 l)
{
    l.x = -l.x;
    l.y = -l.y;
    l.z = -l.z;
    return l;
}

inline v3 operator*(v3 l, f32 q)
{
    return v3{ l.x * q, l.y * q, l.z * q };
}

inline v3& operator*=(v3& l, f32 q)
{
    l.x *= q;
    l.y *= q;
    l.z *= q;
    return l;
}

inline v3 operator/(v3 l, f32 q)
{
    return v3{ l.x / q, l.y / q, l.z / q };
}

inline v3& operator/=(v3& l, f32 q)
{
    l.x /= q;
    l.y /= q;
    l.z /= q;
    return l;
}

inline v4 operator+(v4 l, v4 r)
{
    return v4{ l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w };
}

inline v4 operator+(v4 l, f32 q)
{
    return v4{ l.x + q, l.y + q, l.z + q, l.w + q };
}

inline v4& operator+=(v4& l, v4 r)
{
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
    l.w += r.w;
    return l;
}

inline v4& operator+=(v4& l, f32 q)
{
    l.x += q;
    l.y += q;
    l.z += q;
    l.w += q;
    return l;
}

inline v4 operator-(v4 l, v4 r)
{
    return v4{ l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w };
}

inline v4 operator-(v4 l, f32 q)
{
    return v4{ l.x - q, l.y - q, l.z - q, l.w - q };
}

inline v4& operator-=(v4& l, v4 r)
{
    l.x -= r.x;
    l.y -= r.y;
    l.z -= r.z;
    l.w -= r.w;
    return l;
}

inline v4& operator-=(v4& l, f32 q)
{
    l.x -= q;
    l.y -= q;
    l.z -= q;
    l.w -= q;
    return l;
}

inline v4 operator*(v4 l, f32 q)
{
    return v4{ l.x * q, l.y * q, l.z * q, l.w * q };
}

inline v4& operator*=(v4& l, f32 q)
{
    l.x *= q;
    l.y *= q;
    l.z *= q;
    l.w *= q;
    return l;
}

inline v4 operator/(v4 l, f32 q)
{
    return v4{ l.x / q, l.y / q, l.z / q, l.w / q };
}

inline v4& operator/=(v4& l, f32 q)
{
    l.x /= q;
    l.y /= q;
    l.z /= q;
    l.w /= q;
    return l;
}

inline f32 Length(v2 v)
{
    return Sqrt(v.x * v.x + v.y * v.y);
}

inline f32 Length(v3 v)
{
    return Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline f32 Length(v4 v)
{
    return Sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

inline f32 Distance(v2 l, v2 r)
{
    return Sqrt(Sqr(r.x - l.x) + Sqr(r.y - l.y));
}

inline f32 Distance(v3 l, v3 r)
{
    return Sqrt(Sqr(r.x - l.x) + Sqr(r.y - l.y) + Sqr(r.z - l.z));
}

inline f32 Distance(v4 l, v4 r)
{
    return Sqrt(Sqr(r.x - l.x) + Sqr(r.y - l.y) + Sqr(r.z - l.z) + Sqr(r.w - l.w));
}

inline v2 Normalize(v2 v)
{
    f32 invLength = 1.0f / Length(v);
    return v2{ v.x * invLength , v.y * invLength };
}

inline v3 Normalize(v3 v)
{
    f32 invLength = 1.0f / Length(v);
    return v3{ v.x * invLength , v.y * invLength, v.z * invLength };
}

inline v4 Normalize(v4 v)
{
    f32 invLength = 1.0f / Length(v);
    return v4{ v.x * invLength , v.y * invLength, v.z * invLength, v.w * invLength };
}

inline f32 Dot(v2 l, v2 r)
{
    return l.x * r.x + l.y * r.y;
}

inline f32 Dot(v3 l, v3 r)
{
    return l.x * r.x + l.y * r.y + l.z * r.z;
}

inline f32 Dot(v4 l, v4 r)
{
    return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
}

inline f32 AngleVecDeg(v2 l, v2 r)
{
    l = Normalize(l);
    r = Normalize(r);
    return ToDeg(Acos(Dot(l, r)));
}

inline f32 AngleVecDeg(v3 l, v3 r)
{
    l = Normalize(l);
    r = Normalize(r);
    return ToDeg(Acos(Dot(l, r)));
}

inline f32 AngleVecDeg(v4 l, v4 r)
{
    l = Normalize(l);
    r = Normalize(r);
    return ToDeg(Acos(Dot(l, r)));
}

inline v3 Cross(v3 l, v3 r)
{
    return v3{ l.y * r.z - l.z * r.y,
               l.z * r.x - l.x * r.z,
               l.x * r.y - l.y * r.x };
}

inline v2 RotateAxisDeg(v2 v, f32 deg)
{
    f32 rad = ToRad(deg);
    return v2{ v.x * Cos(rad) - v.y * Sin(rad),
               v.x * Sin(rad) - v.y * Cos(rad)};
}

// Matrix fun
// NOTE : matrix index on columns

inline m2 operator+(m2 a, m2 b)
{
    m2 m = {};
    m._11 = a._11 + b._11;
    m._12 = a._12 + b._12;
    m._21 = a._21 + b._21;
    m._22 = a._22 + b._22;
    return m;
}

inline m2& operator+=(m2& a, m2 b)
{
    a._11 += b._11;
    a._12 += b._12;
    a._21 += b._21;
    a._22 += b._22;
    return a;
}

inline m2 operator-(m2 a, m2 b)
{
    m2 m = {};
    m._11 = a._11 - b._11;
    m._12 = a._12 - b._12;
    m._21 = a._21 - b._21;
    m._22 = a._22 - b._22;
    return m;
}

inline m2& operator-=(m2& a, m2 b)
{
    a._11 -= b._11;
    a._12 -= b._12;
    a._21 -= b._21;
    a._22 -= b._22;
    return a;
}

inline m2 operator*(f32 q, m2 a)
{
    m2 m = {};
    m._11 = a._11 * q;
    m._12 = a._12 * q;
    m._21 = a._21 * q;
    m._22 = a._22 * q;
    return m;
}

inline m2& operator*=(m2& a, f32 q)
{
    a._11 *= q;
    a._12 *= q;
    a._21 *= q;
    a._22 *= q;
    return a;
}

inline m2 operator*(m2 a, m2 b)
{
    m2 m = {};
    m._11 = a._11 * b._11 + a._12 * b._21;
    m._12 = a._11 * b._12 + a._12 * b._22;
    m._21 = a._21 * b._11 + a._22 * b._21;
    m._22 = a._21 * b._12 + a._22 * b._22;
    return m;
}

inline v2 operator*(m2 a, v2 b)
{
    v2 v = {};
    v.x = a._11 * b.x + a._12 * b.y;
    v.y = a._21 * b.x + a._22 * b.y;
    return v;
}

inline m2& operator*=(m2& a, m2 b)
{
    m2 m = {};
    m._11 = a._11 * b._11 + a._12 * b._21;
    m._12 = a._11 * b._12 + a._12 * b._22;
    m._21 = a._21 * b._11 + a._22 * b._21;
    m._22 = a._21 * b._12 + a._22 * b._22;
    a = m;
    return a;
}

inline m3 operator+(m3 a, m3 b)
{
    m3 m = {};
    m._11 = a._11 + b._11;
    m._12 = a._12 + b._12;
    m._13 = a._13 + b._13;
    m._21 = a._21 + b._21;
    m._22 = a._22 + b._22;
    m._23 = a._23 + b._23;
    m._31 = a._31 + b._31;
    m._32 = a._32 + b._32;
    m._33 = a._33 + b._33;
    return m;
}

inline m3& operator+=(m3& a, m3 b)
{
    a._11 += b._11;
    a._12 += b._12;
    a._13 += b._13;
    a._21 += b._21;
    a._22 += b._22;
    a._23 += b._23;
    a._31 += b._31;
    a._32 += b._32;
    a._33 += b._33;
    return a;
}

inline m3 operator-(m3 a, m3 b)
{
    m3 m = {};
    m._11 = a._11 - b._11;
    m._12 = a._12 - b._12;
    m._13 = a._13 - b._13;
    m._21 = a._21 - b._21;
    m._22 = a._22 - b._22;
    m._23 = a._23 - b._23;
    m._31 = a._31 - b._31;
    m._32 = a._32 - b._32;
    m._33 = a._33 - b._33;
    return m;
}

inline m3& operator-=(m3& a, m3 b)
{
    a._11 -= b._11;
    a._12 -= b._12;
    a._13 -= b._13;
    a._21 -= b._21;
    a._22 -= b._22;
    a._23 -= b._23;
    a._31 -= b._31;
    a._32 -= b._32;
    a._33 -= b._33;
    return a;
}

inline m3 operator*(f32 q, m3 a)
{
    m3 m = {};
    m._11 = a._11 * q;
    m._12 = a._12 * q;
    m._13 = a._13 * q;
    m._21 = a._21 * q;
    m._22 = a._22 * q;
    m._23 = a._23 * q;
    m._31 = a._31 * q;
    m._32 = a._32 * q;
    m._33 = a._33 * q;
    return m;
}

inline m3 operator*(m3 a, m3 b)
{
    m3 m = {};
    m._11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31;
    m._12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32;
    m._13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33;

    m._21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31;
    m._22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32;
    m._23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33;

    m._31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31;
    m._32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32;
    m._33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33;
    return m;
}

inline v3 operator*(m3 a, v3 b)
{
    v3 v = {};
    v.x = a._11 * b.x + a._12 * b.y + a._13 * b.z;
    v.y = a._21 * b.x + a._22 * b.y + a._23 * b.z;
    v.z = a._31 * b.x + a._32 * b.y + a._33 * b.z;
    return v;
}

inline m3& operator*=(m3& a, f32 q)
{
    a._11 *= q;
    a._12 *= q;
    a._13 *= q;
    a._21 *= q;
    a._22 *= q;
    a._23 *= q;
    a._31 *= q;
    a._32 *= q;
    a._33 *= q;
    return a;
}

inline m4 operator+(m4 a, m4 b)
{
    m4 m = {};
    m._11 = a._11 + b._11;
    m._12 = a._12 + b._12;
    m._13 = a._13 + b._13;
    m._14 = a._14 + b._14;
    m._21 = a._21 + b._21;
    m._22 = a._22 + b._22;
    m._23 = a._23 + b._23;
    m._24 = a._24 + b._24;
    m._31 = a._31 + b._31;
    m._32 = a._32 + b._32;
    m._33 = a._33 + b._33;
    m._34 = a._34 + b._34;
    return m;
}

inline m4& operator+=(m4& a, m4 b)
{
    a._11 = a._11 + b._11;
    a._12 = a._12 + b._12;
    a._13 = a._13 + b._13;
    a._14 = a._14 + b._14;
    a._21 = a._21 + b._21;
    a._22 = a._22 + b._22;
    a._23 = a._23 + b._23;
    a._24 = a._24 + b._24;
    a._31 = a._31 + b._31;
    a._32 = a._32 + b._32;
    a._33 = a._33 + b._33;
    a._34 = a._34 + b._34;
    return a;
}

inline m4 operator-(m4 a, m4 b)
{
    m4 m = {};
    m._11 = a._11 - b._11;
    m._12 = a._12 - b._12;
    m._13 = a._13 - b._13;
    m._14 = a._14 - b._14;
    m._21 = a._21 - b._21;
    m._22 = a._22 - b._22;
    m._23 = a._23 - b._23;
    m._24 = a._24 - b._24;
    m._31 = a._31 - b._31;
    m._32 = a._32 - b._32;
    m._33 = a._33 - b._33;
    m._34 = a._34 - b._34;
    return m;
}

inline m4& operator-=(m4& a, m4 b)
{
    a._11 -= b._11;
    a._12 -= b._12;
    a._13 -= b._13;
    a._14 -= b._14;
    a._21 -= b._21;
    a._22 -= b._22;
    a._23 -= b._23;
    a._24 -= b._24;
    a._31 -= b._31;
    a._32 -= b._32;
    a._33 -= b._33;
    a._34 -= b._34;
    return a;
}

inline m4 operator*(f32 q, m4 a)
{
    m4 m = {};
    m._11 = a._11 * q;
    m._12 = a._12 * q;
    m._13 = a._13 * q;
    m._14 = a._14 * q;
    m._21 = a._21 * q;
    m._22 = a._22 * q;
    m._23 = a._23 * q;
    m._24 = a._24 * q;
    m._31 = a._31 * q;
    m._32 = a._32 * q;
    m._33 = a._33 * q;
    m._34 = a._34 * q;
    m._41 = a._41 * q;
    m._42 = a._42 * q;
    m._43 = a._43 * q;
    m._44 = a._44 * q;
    return m;
}

inline m4 operator*(m4 a, m4 b)
{
    m4 m = {};
    m._11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31 + a._14 * b._41;
    m._12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32 + a._14 * b._42;
    m._13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33 + a._14 * b._43;
    m._14 = a._11 * b._14 + a._12 * b._24 + a._13 * b._34 + a._14 * b._44;

    m._21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31 + a._24 * b._41;
    m._22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32 + a._24 * b._42;
    m._23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33 + a._24 * b._43;
    m._24 = a._21 * b._14 + a._22 * b._24 + a._23 * b._34 + a._24 * b._44;

    m._31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31 + a._34 * b._41;
    m._32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32 + a._34 * b._42;
    m._33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33 + a._34 * b._43;
    m._34 = a._31 * b._14 + a._32 * b._24 + a._33 * b._34 + a._34 * b._44;

    m._41 = a._41 * b._11 + a._42 * b._21 + a._43 * b._31 + a._44 * b._41;
    m._42 = a._41 * b._12 + a._42 * b._22 + a._43 * b._32 + a._44 * b._42;
    m._43 = a._41 * b._13 + a._42 * b._23 + a._43 * b._33 + a._44 * b._43;
    m._44 = a._41 * b._14 + a._42 * b._24 + a._43 * b._34 + a._44 * b._44;
    return m;
}

inline v4 operator*(m4 a, v4 b)
{
    v4 v = {};
    v.x = a._11 * b.x + a._12 * b.y + a._13 * b.z + a._14 * b.w;
    v.y = a._21 * b.x + a._22 * b.y + a._23 * b.z + a._24 * b.w;
    v.z = a._31 * b.x + a._32 * b.y + a._33 * b.z + a._34 * b.w;
    v.w = a._41 * b.x + a._42 * b.y + a._43 * b.z + a._44 * b.w;
    return v;
}

inline m4& operator*=(m4& a, f32 q)
{
    a._11 *= q;
    a._12 *= q;
    a._13 *= q;
    a._14 *= q;
    a._21 *= q;
    a._22 *= q;
    a._23 *= q;
    a._24 *= q;
    a._31 *= q;
    a._32 *= q;
    a._33 *= q;
    a._34 *= q;
    return a;
}

inline m2 Identity2()
{
    return m2{ 1.0f, 0.0f, 
               0.0f, 1.0f };
}

inline m3 Identity3()
{
    return m3{ 1.0f, 0.0f, 0.0f, 
               0.0f, 1.0f, 0.0f, 
               0.0f, 0.0f, 1.0f };
}

inline m4 Identity4()
{
    return m4{ 1.0f, 0.0f, 0.0f, 0.0f, 
               0.0f, 1.0f, 0.0f, 0.0f, 
               0.0f, 0.0f, 1.0f, 0.0f, 
               0.0f, 0.0f, 0.0f, 1.0f };
}

// TODO: more form initialise matrix
inline m3 M3(m4 m)
{
    return m3{ m._11, m._21, m._31,
               m._12, m._22, m._32,
               m._13, m._23, m._33 };
}

inline m4 M4(m3 m)
{
    return m4{ m._11, m._21, m._31, 0.0f,
               m._12, m._22, m._32, 0.0f,
               m._13, m._23, m._33, 0.0f,
               0.0f,  0.0f,  0.0f,  1.0f };
}

inline m2 Transpose(m2 m)
{
    return m2{ m._11, m._12,
               m._21, m._22 };
}

inline m3 Transpose(m3 m)
{
    return m3{ m._11, m._12, m._13,
               m._21, m._22, m._23,
               m._31, m._32, m._33 };
}

inline m4 Transpose(m4 m)
{
    return m4{ m._11, m._12, m._13, m._14,
               m._21, m._22, m._23, m._24,
               m._31, m._32, m._33, m._34,
               m._41, m._42, m._43, m._44 };
}

inline f32 Determinant(m2 m)
{
    return m._11 * m._22 - m._21 * m._12;
}

inline f32 Determinant(m3 m)
{
    return (m._11 * m._22 * m._33) + (m._21 * m._32 * m._13) +
           (m._12 * m._23 * m._31) - (m._31 * m._22 * m._13) -
           (m._21 * m._12 * m._33) - (m._32 * m._23 * m._11);
}

inline f32 Determinant(m4 m)
{
    f32 A_11 = Determinant(m3{ m._22, m._32, m._42, m._23, m._33, m._43, m._24, m._34, m._44 });
    f32 A_12 = -1 * Determinant(m3{ m._21, m._31, m._41, m._23, m._33, m._43, m._24, m._34, m._44 });
    f32 A_13 = Determinant(m3{ m._21, m._31, m._41, m._22, m._32, m._42, m._24, m._34, m._44 });
    f32 A_14 = -1 * Determinant(m3{ m._21, m._31, m._41, m._22, m._32, m._42, m._23, m._33, m._43 });

    return (m._11 * A_11) + (m._12 * A_12) + (m._13 * A_13) + (m._14 * A_14);
}

inline m3 Inverse(m3 m)
{
    f32 a11 = m._22 * m._33 - m._23 * m._32;
    f32 a12 = -(m._21 * m._33 - m._23 * m._31);
    f32 a13 = m._21 * m._32 - m._22 * m._31;

    f32 det = (m._11 * a11 + m._12 * a12 + m._13 * a13);

    f32 a21 = -(m._12 * m._33 - m._32 * m._13);
    f32 a22 = m._11 * m._33 - m._13 * m._31;
    f32 a23 = -(m._11 * m._32 - m._12 * m._31);

    f32 a31 = m._12 * m._23 - m._22 * m._13;
    f32 a32 = -(m._11 * m._23 - m._21 * m._13);
    f32 a33 = m._11 * m._22 - m._21 * m._12;

    f32 oneOverDet = 1.0f / det;

    m3 inv = {};
    inv._11 = a11 * oneOverDet;
    inv._12 = a21 * oneOverDet;
    inv._13 = a31 * oneOverDet;
    inv._21 = a12 * oneOverDet;
    inv._22 = a22 * oneOverDet;
    inv._23 = a32 * oneOverDet;
    inv._31 = a13 * oneOverDet;
    inv._32 = a23 * oneOverDet;
    inv._33 = a33 * oneOverDet;

    return inv;
}

inline m4 Inverse(m4 m)
{
    f32 a11_22 = m._33 * m._44 - m._34 * m._43;
    f32 a11_23 = m._32 * m._44 - m._34 * m._42;
    f32 a11_24 = m._32 * m._43 - m._33 * m._42;

    f32 A11 = m._22 * a11_22 - m._23 * a11_23 + m._24 * a11_24;

    f32 a12_21 = m._33 * m._44 - m._34 * m._43;
    f32 a12_23 = m._31 * m._44 - m._34 * m._41;
    f32 a12_24 = m._31 * m._43 - m._33 * m._41;

    f32 A12 = -(m._21 * a12_21 - m._23 * a12_23 + m._24 * a12_24);

    f32 a13_21 = m._32 * m._44 - m._34 * m._42;
    f32 a13_22 = m._31 * m._44 - m._34 * m._41;
    f32 a13_24 = m._31 * m._42 - m._32 * m._41;

    f32 A13 = m._21 * a13_21 - m._22 * a13_22 + m._24 * a13_24;

    f32 a14_21 = m._32 * m._43 - m._33 * m._42;
    f32 a14_22 = m._31 * m._43 - m._33 * m._41;
    f32 a14_23 = m._31 * m._42 - m._32 * m._41;

    f32 A14 = -(m._21 * a14_21 - m._22 * a14_22 + m._23 * a14_23);

    f32 a21_12 = m._33 * m._44 - m._34 * m._43;
    f32 a21_13 = m._32 * m._44 - m._34 * m._42;
    f32 a21_14 = m._32 * m._43 - m._33 * m._42;

    f32 A21 = -(m._12 * a21_12 - m._13 * a21_13 + m._14 * a21_14);

    f32 a22_11 = m._33 * m._44 - m._34 * m._43;
    f32 a22_13 = m._31 * m._44 - m._34 * m._41;
    f32 a22_14 = m._31 * m._43 - m._33 * m._41;

    f32 A22 = m._11 * a22_11 - m._13 * a22_13 + m._14 * a22_14;

    f32 a23_11 = m._32 * m._44 - m._34 * m._42;
    f32 a23_12 = m._31 * m._44 - m._34 * m._41;
    f32 a23_14 = m._31 * m._42 - m._32 * m._41;

    f32 A23 = -(m._11 * a23_11 - m._12 * a23_12 + m._14 * a23_14);

    f32 a24_11 = m._32 * m._43 - m._33 * m._42;
    f32 a24_12 = m._31 * m._43 - m._33 * m._41;
    f32 a24_13 = m._31 * m._42 - m._32 * m._41;

    f32 A24 = m._11 * a24_11 - m._12 * a24_12 + m._13 * a24_13;

    f32 a31_12 = m._23 * m._44 - m._24 * m._43;
    f32 a31_13 = m._22 * m._44 - m._24 * m._42;
    f32 a31_14 = m._22 * m._43 - m._23 * m._42;

    f32 A31 = m._12 * a31_12 - m._13 * a31_13 + m._14 * a31_14;

    f32 a32_11 = m._23 * m._44 - m._24 * m._43;
    f32 a32_13 = m._21 * m._44 - m._24 * m._41;
    f32 a32_14 = m._21 * m._43 - m._23 * m._41;

    f32 A32 = -(m._11 * a32_11 - m._13 * a32_13 + m._14 * a32_14);

    f32 a33_11 = m._22 * m._44 - m._24 * m._42;
    f32 a33_12 = m._21 * m._44 - m._24 * m._41;
    f32 a33_14 = m._21 * m._42 - m._22 * m._41;

    f32 A33 = m._11 * a33_11 - m._12 * a33_12 + m._14 * a33_14;

    f32 a34_11 = m._22 * m._43 - m._23 * m._42;
    f32 a34_12 = m._21 * m._43 - m._23 * m._41;
    f32 a34_13 = m._21 * m._42 - m._22 * m._41;

    f32 A34 = -(m._11 * a34_11 - m._12 * a34_12 + m._13 * a34_13);

    f32 a41_12 = m._23 * m._34 - m._24 * m._33;
    f32 a41_13 = m._22 * m._34 - m._24 * m._32;
    f32 a41_14 = m._22 * m._33 - m._23 * m._32;

    f32 A41 = -(m._12 * a41_12 - m._13 * a41_13 + m._14 * a41_14);

    f32 a42_11 = m._23 * m._34 - m._24 * m._33;
    f32 a42_13 = m._21 * m._34 - m._24 * m._31;
    f32 a42_14 = m._21 * m._33 - m._23 * m._31;

    f32 A42 = m._11 * a42_11 - m._13 * a42_13 + m._14 * a42_14;

    f32 a43_11 = m._22 * m._34 - m._24 * m._32;
    f32 a43_12 = m._21 * m._34 - m._24 * m._31;
    f32 a43_14 = m._21 * m._32 - m._22 * m._31;

    f32 A43 = -(m._11 * a43_11 - m._12 * a43_12 + m._14 * a43_14);

    f32 a44_11 = m._22 * m._33 - m._23 * m._32;
    f32 a44_12 = m._21 * m._33 - m._23 * m._31;
    f32 a44_13 = m._21 * m._32 - m._22 * m._31;

    f32 A44 = m._11 * a44_11 - m._12 * a44_12 + m._13 * a44_13;

    f32 det = m._11 * A11 + m._12 * A12 + m._13 * A13 + m._14 * A14;

    f32 oneOverDet = 1.0f / det;
    m4 result = {};
    result._11 = A11 * oneOverDet;
    result._12 = A21 * oneOverDet;
    result._13 = A31 * oneOverDet;
    result._14 = A41 * oneOverDet;
    result._21 = A12 * oneOverDet;
    result._22 = A22 * oneOverDet;
    result._23 = A32 * oneOverDet;
    result._24 = A42 * oneOverDet;
    result._31 = A13 * oneOverDet;
    result._32 = A23 * oneOverDet;
    result._33 = A33 * oneOverDet;
    result._34 = A43 * oneOverDet;
    result._41 = A14 * oneOverDet;
    result._42 = A24 * oneOverDet;
    result._43 = A34 * oneOverDet;
    result._44 = A44 * oneOverDet;

    return result;
}

inline m4 Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 n, f32 f)
{
    m4 m = {};
    m._11 = 2.0f / (right - left);
    m._22 = 2.0f / (top - bottom);
    m._33 = -2.0f / (f - n);
    m._14 = -((right + left) / (right - left));
    m._24 = -((top + bottom) / (top - bottom));
    m._34 = -((f + n) / (f - n));
    m._44 = 1.0f;
    return m;
}

// NOTE: http://perry.cz/articles/ProjectionMatrix.xhtml#fig_LH_vs_RH
inline m4 PerspectiveOpenGLRH(f32 fovDeg, f32 width, f32 height, f32 n, f32 f)
{
    m4 m = {};
    f32 AR = width / height;
    f32 tanHalfFov = Tan(ToRad(fovDeg) * 0.5f);

    // TODO: it requires a solution
    // the expression in the comment stretches the image into a rectangle at 16:9
    m._11 = 1.0f / (AR * tanHalfFov); // AR * (1.0f / tanHalfFov);
    m._22 = 1.0f / tanHalfFov;
    m._33 = -(f + n) / (f - n);
    m._34 = (-2 * f * n) / (f - n);
    m._43 = -1.0f;

    return m;
}

inline m4 Translation(v3 trans)
{
    m4 result = Identity4();
    result._14 = trans.x;
    result._24 = trans.y;
    result._34 = trans.z;
    return result;
}

inline v3 GetPosition(m4 m)
{
    return v3{ m._14, m._24, m._34 };
}

inline m4 Translate(m4 m, v3 trans)
{
    m4 result = m;

    // can replace to 
    /*v4 vec = V4(trans, 1);
    vec = m * vec;
    result.columns[3] = vec;*/
    
    result.columns[3] = ((m.columns[0] * trans.x) + (m.columns[1] * trans.y)) +
                        ((m.columns[2] * trans.z) + (m.columns[3]));
    return result;
}

inline m4 Scaling(v3 scale)
{
    m4 result = {};
    result._11 = scale.x;
    result._22 = scale.y;
    result._33 = scale.z;
    result._44 = 1.0f;
    return result;
}

inline m4 Scale(m4 m, v3 scale)
{
    m4 result = m;
    result.columns[0] = m.columns[0] * scale.x;
    result.columns[1] = m.columns[1] * scale.y;
    result.columns[2] = m.columns[2] * scale.z;
    return result;
}

// NOTE : http://perry.cz/articles/ProjectionMatrix.xhtml
inline m4 LookAtDirRH(v3 position, v3 dir, v3 up)
{
    v3 zAxis = Normalize(dir);
    v3 xAxis = Normalize(Cross(zAxis, up));
    v3 yAxis = Cross(xAxis, zAxis);

    m4 m = {};
    m._11 = xAxis.x;
    m._12 = xAxis.y;
    m._13 = xAxis.z;
    m._14 = Dot(xAxis, position);

    m._21 = yAxis.x;
    m._22 = yAxis.y;
    m._23 = yAxis.z;
    m._24 = Dot(yAxis, position);

    m._31 = zAxis.x;
    m._32 = zAxis.y;
    m._33 = zAxis.z;
    m._34 = Dot(zAxis, position);

    m._41 = 0.0f;
    m._42 = 0.0f;
    m._43 = 0.0f;
    m._44 = 1.0f;

    return m;
}

inline m4 Rotation(f32 angleDeg, v3 axis)
{
    m4 m = {};

    f32 c = Cos(ToRad(angleDeg));
    f32 oc = 1.0f - c;
    f32 s = Sin(ToRad(angleDeg));
    v3 an = Normalize(axis);

    m._11 = c + an.x * an.x * oc;
    m._12 = (an.x * an.y * oc) + (an.z * s);
    m._13 = (an.x * an.z * oc) - (an.y * s);

    m._21 = (an.x * an.y * oc) - (an.z * s);
    m._22 = c + an.y * an.y * oc;
    m._23 = (an.y * an.z * oc) + (an.x * s);

    m._31 = (an.x * an.z * oc) + (an.y * s);
    m._32 = (an.y * an.z * oc) - (an.x * s);
    m._33 = c + an.z * an.z * oc;

    m._44 = 1.0f;
    return m;
}

inline m4 Rotate(m4 m, f32 angleDeg, v3 axis)
{
    m4 rotation = {};

    f32 c = Cos(ToRad(angleDeg));
    f32 oc = 1.0f - c;
    f32 s = Sin(ToRad(angleDeg));
    v3 an = Normalize(axis);

    rotation._11 = c + an.x * an.x * oc;
    rotation._12 = (an.x * an.y * oc) + (an.z * s);
    rotation._13 = (an.x * an.z * oc) - (an.y * s);

    rotation._21 = (an.x * an.y * oc) - (an.z * s);
    rotation._22 = c + an.y * an.y * oc;
    rotation._23 = (an.y * an.z * oc) + (an.x * s);

    rotation._31 = (an.x * an.z * oc) + (an.y * s);
    rotation._32 = (an.y * an.z * oc) - (an.x * s);
    rotation._33 = c + an.z * an.z * oc;

    m4 result = {};
    // MUL M3M3
    result._11 = m._11 * rotation._11 + m._12 * rotation._21 + m._13 * rotation._31;
    result._12 = m._11 * rotation._12 + m._12 * rotation._22 + m._13 * rotation._32;
    result._13 = m._11 * rotation._13 + m._12 * rotation._23 + m._13 * rotation._33;

    result._21 = m._21 * rotation._11 + m._22 * rotation._21 + m._23 * rotation._31;
    result._22 = m._21 * rotation._12 + m._22 * rotation._22 + m._23 * rotation._32;
    result._23 = m._21 * rotation._13 + m._22 * rotation._23 + m._23 * rotation._33;

    result._31 = m._31 * rotation._11 + m._32 * rotation._21 + m._33 * rotation._31;
    result._32 = m._31 * rotation._12 + m._32 * rotation._22 + m._33 * rotation._32;
    result._33 = m._31 * rotation._13 + m._32 * rotation._23 + m._33 * rotation._33;

    result.columns[3] = m.columns[3];
    result._41 = m._41;
    result._42 = m._42;
    result._43 = m._43;
    result._44 = m._44;

    return result;
}