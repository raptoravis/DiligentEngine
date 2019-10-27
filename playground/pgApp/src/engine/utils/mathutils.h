#pragma once

#include "BasicMath.h"

namespace ade
{
constexpr float kPi = 3.1415926535897932384626433832795f;
constexpr float kPi2 = 6.2831853071795864769252867665590f;
constexpr float kInvPi = 1.0f / kPi;
constexpr float kPiHalf = 1.5707963267948966192313216916398f;
constexpr float kPiQuarter = 0.7853981633974483096156608458199f;
constexpr float kSqrt2 = 1.4142135623730950488016887242097f;

constexpr float kNearZero = 1.0f / float(1 << 28);
constexpr float kFloatMin = 1.175494e-38f;
constexpr float kFloatMax = 3.402823e+38f;

template <typename Ty>
inline void swap(Ty& _a, Ty& _b)
{
    Ty tmp = _a;
    _a = _b;
    _b = tmp;
}

#undef min
#undef max

template <typename Ty>
inline constexpr Ty min(const Ty& _a, const Ty& _b)
{
    return _a < _b ? _a : _b;
}

template <typename Ty>
inline constexpr Ty max(const Ty& _a, const Ty& _b)
{
    return _a > _b ? _a : _b;
}

template <typename Ty>
inline constexpr Ty min(const Ty& _a, const Ty& _b, const Ty& _c)
{
    return min(min(_a, _b), _c);
}

template <typename Ty>
inline constexpr Ty max(const Ty& _a, const Ty& _b, const Ty& _c)
{
    return max(max(_a, _b), _c);
}

template <typename Ty>
inline constexpr Ty mid(const Ty& _a, const Ty& _b, const Ty& _c)
{
    return max(min(_a, _b), min(max(_a, _b), _c));
}

template <typename Ty>
inline constexpr Ty clamp(const Ty& _a, const Ty& _min, const Ty& _max)
{
    return max(min(_a, _max), _min);
}

inline float lerp(float _a, float _b, float _t)
{
    return _a + (_b - _a) * _t;
}


inline Diligent::float3 fromLatLong(float _u, float _v)
{
    Diligent::float3 result;
    const float phi = _u * kPi2;
    const float theta = _v * kPi;

    const float st = sin(theta);
    const float sp = sin(phi);
    const float ct = cos(theta);
    const float cp = cos(phi);

    result.x = -st * sp;
    result.y = ct;
    result.z = -st * cp;
    return result;
}

inline void toLatLong(float* _outU, float* _outV, const Diligent::float3 _dir)
{
    const float phi = atan2(_dir.x, _dir.z);
    const float theta = acos(_dir.y);

    *_outU = (kPi + phi) / kPi2;
    *_outV = theta * kInvPi;
}

	inline  float toRad(float _deg)
{
    return _deg * kPi / 180.0f;
}

inline  float toDeg(float _rad)
{
    return _rad * 180.0f / kPi;
}

inline  uint32_t floatToBits(float _a)
{
    union {
        float f;
        uint32_t ui;
    } u = { _a };
    return u.ui;
}

inline  float bitsToFloat(uint32_t _a)
{
    union {
        uint32_t ui;
        float f;
    } u = { _a };
    return u.f;
}


inline Diligent::float3 neg(const Diligent::float3& _a)
{
    return {
        -_a.x,
        -_a.y,
        -_a.z,
    };
}

inline Diligent::float3 add(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return {
        _a.x + _b.x,
        _a.y + _b.y,
        _a.z + _b.z,
    };
}

inline Diligent::float3 add(const Diligent::float3& _a, float _b)
{
    return {
        _a.x + _b,
        _a.y + _b,
        _a.z + _b,
    };
}

inline Diligent::float3 sub(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return {
        _a.x - _b.x,
        _a.y - _b.y,
        _a.z - _b.z,
    };
}

inline Diligent::float3 sub(const Diligent::float3& _a, float _b)
{
    return {
        _a.x - _b,
        _a.y - _b,
        _a.z - _b,
    };
}

inline Diligent::float3 mul(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return {
        _a.x * _b.x,
        _a.y * _b.y,
        _a.z * _b.z,
    };
}

inline Diligent::float3 mul(const Diligent::float3& _a, float _b)
{
    return {
        _a.x * _b,
        _a.y * _b,
        _a.z * _b,
    };
}

inline Diligent::float3 mad(const Diligent::float3& _a, const float _b, const Diligent::float3& _c)
{
    return add(mul(_a, _b), _c);
}

inline Diligent::float3 mad(const Diligent::float3& _a, const Diligent::float3& _b,
                            const Diligent::float3& _c)
{
    return add(mul(_a, _b), _c);
}

inline float dot(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
}

inline Diligent::float3 cross(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return {
        _a.y * _b.z - _a.z * _b.y,
        _a.z * _b.x - _a.x * _b.z,
        _a.x * _b.y - _a.y * _b.x,
    };
}

inline float length(const Diligent::float3& _a)
{
    return sqrt(dot(_a, _a));
}

inline float distanceSq(const Diligent::float3& _a, const Diligent::float3& _b)
{
    const Diligent::float3& ba = sub(_b, _a);
    return dot(ba, ba);
}

inline float distance(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return length(sub(_b, _a));
}

inline Diligent::float3 lerp(const Diligent::float3& _a, const Diligent::float3& _b, float _t)
{
    return {
        lerp(_a.x, _b.x, _t),
        lerp(_a.y, _b.y, _t),
        lerp(_a.z, _b.z, _t),
    };
}

inline Diligent::float3 lerp(const Diligent::float3& _a, const Diligent::float3& _b,
                             const Diligent::float3& _t)
{
    return {
        lerp(_a.x, _b.x, _t.x),
        lerp(_a.y, _b.y, _t.y),
        lerp(_a.z, _b.z, _t.z),
    };
}

inline Diligent::float3 normalize(const Diligent::float3& _a)
{
    const float invLen = 1.0f / length(_a);
    const Diligent::float3& result = mul(_a, invLen);
    return result;
}

inline Diligent::float3 min(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return {
        std::min(_a.x, _b.x),
        std::min(_a.y, _b.y),
        std::min(_a.z, _b.z),
    };
}

inline Diligent::float3 max(const Diligent::float3& _a, const Diligent::float3& _b)
{
    return {
        std::max(_a.x, _b.x),
        std::max(_a.y, _b.y),
        std::max(_a.z, _b.z),
    };
}

inline Diligent::float3 rcp(const Diligent::float3& _a)
{
    return {
        1.0f / _a.x,
        1.0f / _a.y,
        1.0f / _a.z,
    };
}

inline float SIGN(float x)
{
    return (x >= 0.0f) ? +1.0f : -1.0f;
}

inline float NORM(float a, float b, float c, float d)
{
    return sqrt(a * a + b * b + c * c + d * d);
}

inline Diligent::Quaternion mRot2Quat(const Diligent::float4x4& m)
{
    float r11 = m._11;
    float r12 = m._12;
    float r13 = m._13;
    float r21 = m._21;
    float r22 = m._22;
    float r23 = m._23;
    float r31 = m._31;
    float r32 = m._32;
    float r33 = m._33;
    float q0 = (r11 + r22 + r33 + 1.0f) / 4.0f;
    float q1 = (r11 - r22 - r33 + 1.0f) / 4.0f;
    float q2 = (-r11 + r22 - r33 + 1.0f) / 4.0f;
    float q3 = (-r11 - r22 + r33 + 1.0f) / 4.0f;
    if (q0 < 0.0f) {
        q0 = 0.0f;
    }
    if (q1 < 0.0f) {
        q1 = 0.0f;
    }
    if (q2 < 0.0f) {
        q2 = 0.0f;
    }
    if (q3 < 0.0f) {
        q3 = 0.0f;
    }
    q0 = sqrt(q0);
    q1 = sqrt(q1);
    q2 = sqrt(q2);
    q3 = sqrt(q3);
    if (q0 >= q1 && q0 >= q2 && q0 >= q3) {
        q0 *= +1.0f;
        q1 *= SIGN(r32 - r23);
        q2 *= SIGN(r13 - r31);
        q3 *= SIGN(r21 - r12);
    } else if (q1 >= q0 && q1 >= q2 && q1 >= q3) {
        q0 *= SIGN(r32 - r23);
        q1 *= +1.0f;
        q2 *= SIGN(r21 + r12);
        q3 *= SIGN(r13 + r31);
    } else if (q2 >= q0 && q2 >= q1 && q2 >= q3) {
        q0 *= SIGN(r13 - r31);
        q1 *= SIGN(r21 + r12);
        q2 *= +1.0f;
        q3 *= SIGN(r32 + r23);
    } else if (q3 >= q0 && q3 >= q1 && q3 >= q2) {
        q0 *= SIGN(r21 - r12);
        q1 *= SIGN(r31 + r13);
        q2 *= SIGN(r32 + r23);
        q3 *= +1.0f;
    } else {
        printf("coding error\n");
    }
    float r = NORM(q0, q1, q2, q3);
    q0 /= r;
    q1 /= r;
    q2 /= r;
    q3 /= r;

    // quaternion = [w, x, y, z]'
    Diligent::Quaternion res = { q1, q2, q3, q0 };

    return res;
}

inline Diligent::Quaternion calculateRotation(const Diligent::float4x4& m)
{
    float r11 = m._11;
    float r12 = m._12;
    float r13 = m._13;
    float r21 = m._21;
    float r22 = m._22;
    float r23 = m._23;
    float r31 = m._31;
    float r32 = m._32;
    float r33 = m._33;

    float qw, qx, qy, qz;

    float trace = r11 + r22 + r33;    // I removed + 1.0f; see discussion with Ethan
    if (trace > 0) {                  // I changed M_EPSILON to 0
        float s = 0.5f / sqrtf(trace + 1.0f);
        qw = 0.25f / s;
        qx = (r32 - r23) * s;
        qy = (r13 - r31) * s;
        qz = (r21 - r12) * s;
    } else {
        if (r11 > r22 && r11 > r33) {
            float s = 2.0f * sqrtf(1.0f + r11 - r22 - r33);
            qw = (r32 - r23) / s;
            qx = 0.25f * s;
            qy = (r12 + r21) / s;
            qz = (r13 + r31) / s;
        } else if (r22 > r33) {
            float s = 2.0f * sqrtf(1.0f + r22 - r11 - r33);
            qw = (r13 - r31) / s;
            qx = (r12 + r21) / s;
            qy = 0.25f * s;
            qz = (r23 + r32) / s;
        } else {
            float s = 2.0f * sqrtf(1.0f + r33 - r11 - r22);
            qw = (r21 - r12) / s;
            qx = (r13 + r31) / s;
            qy = (r23 + r32) / s;
            qz = 0.25f * s;
        }
    }

    Diligent::Quaternion res = { qx, qy, qz, qw };

    return res;
}

inline Diligent::Quaternion MakeQuaternionFromTwoVec3(const Diligent::float3& u,
                                                      const Diligent::float3& v)
{
    float norm_u_norm_v = sqrt(Diligent::dot(u, u) * Diligent::dot(v, v));
    float real_part = norm_u_norm_v + Diligent::dot(u, v);
    Diligent::float3 t;

    if (real_part < static_cast<float>(1.e-6f) * norm_u_norm_v) {
        // If u and v are exactly opposite, rotate 180 degrees
        // around an arbitrary orthogonal axis. Axis normalisation
        // can happen later, when we normalise the quaternion.
        real_part = static_cast<float>(0);
        t = abs(u.x) > abs(u.z) ? Diligent::float3(-u.y, u.x, static_cast<float>(0))
                                : Diligent::float3(static_cast<float>(0), -u.z, u.y);
    } else {
        // Otherwise, build quaternion the standard way.
        t = cross(u, v);
    }

    Diligent::Quaternion r = normalize(Diligent::Quaternion(t.x, t.y, t.z, real_part));

    return r;
}

inline void vec4MulMtx(Diligent::float4& out, const Diligent::float4& in,
                       const Diligent::float4x4& mat)
{
    out = in * mat;
}

inline void mtxSRT(Diligent::float4x4& _result, float _sx, float _sy, float _sz, float _ax,
                   float _ay, float _az, float _tx, float _ty, float _tz)
{
    const float sx = sin(_ax);
    const float cx = cos(_ax);
    const float sy = sin(_ay);
    const float cy = cos(_ay);
    const float sz = sin(_az);
    const float cz = cos(_az);

    const float sxsz = sx * sz;
    const float cycz = cy * cz;

    _result._11 = _sx * (cycz - sxsz * sy);
    _result._12 = _sx * -cx * sz;
    _result._13 = _sx * (cz * sy + cy * sxsz);
    _result._14 = 0.0f;

    _result._21 = _sy * (cz * sx * sy + cy * sz);
    _result._22 = _sy * cx * cz;
    _result._23 = _sy * (sy * sz - cycz * sx);
    _result._24 = 0.0f;

    _result._31 = _sz * -cx * sy;
    _result._32 = _sz * sx;
    _result._33 = _sz * cx * cy;
    _result._34 = 0.0f;

    _result._41 = _tx;
    _result._42 = _ty;
    _result._43 = _tz;
    _result._44 = 1.0f;
}

struct Handness {
    enum Enum {
        Left,
        Right,
    };
};

inline void mtxLookAt(Diligent::float4x4& _result, const Diligent::float3& _eye,
                      const Diligent::float3& _at,
                      const Diligent::float3& _up = Diligent::float3(0, 1, 0),
                      Handness::Enum _handness = Handness::Left)
{
    const Diligent::float3& view =
        Diligent::normalize(Handness::Right == _handness ? _eye - _at : _at - _eye);
    const Diligent::float3& uxv = Diligent::cross(_up, view);
    const Diligent::float3& right = Diligent::normalize(uxv);
    const Diligent::float3& up = Diligent::cross(view, right);

    memset(_result.m, 0, sizeof(float) * 16);

    _result._11 = right.x;
    _result._12 = up.x;
    _result._13 = view.x;

    _result._21 = right.y;
    _result._22 = up.y;
    _result._23 = view.y;

    _result._31 = right.z;
    _result._32 = up.z;
    _result._33 = view.z;

    _result._41 = -Diligent::dot(right, _eye);
    _result._42 = -Diligent::dot(up, _eye);
    _result._43 = -Diligent::dot(view, _eye);
    _result._44 = 1.0f;
}


inline void mtxProjXYWH(float* _result, float _x, float _y, float _width, float _height,
                        float _near, float _far, bool _homogeneousNdc, Handness::Enum _handness)
{
    const float diff = _far - _near;
    const float aa = _homogeneousNdc ? (_far + _near) / diff : _far / diff;
    const float bb = _homogeneousNdc ? (2.0f * _far * _near) / diff : _near * aa;

    memset(_result, 0, sizeof(float) * 16);

    _result[0] = _width;
    _result[5] = _height;
    _result[8] = (Handness::Right == _handness) ? _x : -_x;
    _result[9] = (Handness::Right == _handness) ? _y : -_y;
    _result[10] = (Handness::Right == _handness) ? -aa : aa;
    _result[11] = (Handness::Right == _handness) ? -1.0f : 1.0f;
    _result[14] = -bb;
}

inline void mtxProj(float* _result, float _ut, float _dt, float _lt, float _rt, float _near,
                   float _far,
             bool _homogeneousNdc, Handness::Enum _handness)
{
    const float invDiffRl = 1.0f / (_rt - _lt);
    const float invDiffUd = 1.0f / (_ut - _dt);
    const float width = 2.0f * _near * invDiffRl;
    const float height = 2.0f * _near * invDiffUd;
    const float xx = (_rt + _lt) * invDiffRl;
    const float yy = (_ut + _dt) * invDiffUd;
    mtxProjXYWH(_result, xx, yy, width, height, _near, _far, _homogeneousNdc, _handness);
}

inline void mtxProj(float* _result, const float _fov[4], float _near, float _far,
                   bool _homogeneousNdc,
             Handness::Enum _handness)
{
    mtxProj(_result, _fov[0], _fov[1], _fov[2], _fov[3], _near, _far, _homogeneousNdc, _handness);
}

inline void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far,
             bool _homogeneousNdc = false, Handness::Enum _handness = Handness::Left)
{
    const float height = 1.0f / std::tan(toRad(_fovy) * 0.5f);
    const float width = height * 1.0f / _aspect;
    mtxProjXYWH(_result, 0.0f, 0.0f, width, height, _near, _far, _homogeneousNdc, _handness);
}


}    // namespace ade
