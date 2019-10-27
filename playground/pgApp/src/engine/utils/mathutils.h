#pragma once

#include "BasicMath.h"

namespace ade
{

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
                      const Diligent::float3& _at, const Diligent::float3& _up,
                      Handness::Enum _handness = Handness::Left)
{
    const Diligent::float3 view =
        Diligent::normalize(Handness::Right == _handness ? _eye - _at : _at - _eye);
    const Diligent::float3 uxv = Diligent::cross(_up, view);
    const Diligent::float3 right = Diligent::normalize(uxv);
    const Diligent::float3 up = Diligent::cross(view, right);

    memset(_result.m, 0, sizeof(float) * 16);

    _result._11 = right.x;
    _result._12 = up.x;
    _result._13 = view.x;

    _result._21 = right.y;
    _result._22 = up.y;
    _result._23 = view.y;

    _result._31 = right.z;
    _result._32 = up.z;
    _result._33= view.z;

    _result._41 = -Diligent::dot(right, _eye);
    _result._42 = -Diligent::dot(up, _eye);
    _result._43 = -Diligent::dot(view, _eye);
    _result._44 = 1.0f;
}

}    // namespace ade