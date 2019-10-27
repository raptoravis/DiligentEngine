#pragma once

#include "BasicMath.h"
#include "SampleBase.h"

#include "engine/utils/mathutils.h"

struct CameraAlt {
    CameraAlt() { reset(); }

    void reset()
    {
        m_target.curr = { 0.0f, 0.0f, 0.0f };
        m_target.dest = { 0.0f, 0.0f, 0.0f };

        m_pos.curr = { 55.0f, 20.0f, 65.0f };
        m_pos.dest = { 55.0f, 20.0f, 65.0f };

        m_orbit[0] = 0.0f;
        m_orbit[1] = 0.0f;
    }

    void mtxLookAt(Diligent::float4x4& _outViewMtx)
    {
        ade::mtxLookAt(_outViewMtx, m_pos.curr, m_target.curr);
    }

    void orbit(float _dx, float _dy)
    {
        m_orbit[0] += _dx;
        m_orbit[1] += _dy;
    }

    void dolly(float _dz)
    {
        const float cnear = 1.0f;
        const float cfar = 100.0f;

        const Diligent::float3 toTarget = ade::sub(m_target.dest, m_pos.dest);
        const float toTargetLen = Diligent::length(toTarget);
        const float invToTargetLen = 1.0f / (toTargetLen + ade::kFloatMin);
        const Diligent::float3 toTargetNorm = ade::mul(toTarget, invToTargetLen);

        float delta = toTargetLen * _dz;
        float newLen = toTargetLen + delta;
        if ((cnear < newLen || _dz < 0.0f) && (newLen < cfar || _dz > 0.0f)) {
            m_pos.dest = ade::mad(toTargetNorm, delta, m_pos.dest);
        }
    }

    void consumeOrbit(float _amount)
    {
        float consume[2];
        consume[0] = m_orbit[0] * _amount;
        consume[1] = m_orbit[1] * _amount;
        m_orbit[0] -= consume[0];
        m_orbit[1] -= consume[1];

        const Diligent::float3 toPos = ade::sub(m_pos.curr, m_target.curr);
        const float toPosLen = ade::length(toPos);
        const float invToPosLen = 1.0f / (toPosLen + ade::kFloatMin);
        const Diligent::float3 toPosNorm = ade::mul(toPos, invToPosLen);

        float ll[2];
        ade::toLatLong(&ll[0], &ll[1], toPosNorm);
        ll[0] += consume[0];
        ll[1] -= consume[1];
        ll[1] = ade::clamp(ll[1], 0.02f, 0.98f);

        const Diligent::float3 tmp = ade::fromLatLong(ll[0], ll[1]);
        const Diligent::float3 diff = ade::mul(ade::sub(tmp, toPosNorm), toPosLen);

        m_pos.curr = ade::add(m_pos.curr, diff);
        m_pos.dest = ade::add(m_pos.dest, diff);
    }

    void update(float _dt)
    {
        const float amount = std::min(_dt / 0.12f, 1.0f);

        consumeOrbit(amount);

        m_target.curr = Diligent::lerp(m_target.curr, m_target.dest, amount);
        m_pos.curr = Diligent::lerp(m_pos.curr, m_pos.dest, amount);
    }

    void envViewMtx(float* _mtx)
    {
        const Diligent::float3 toTarget = ade::sub(m_target.curr, m_pos.curr);
        const float toTargetLen = Diligent::length(toTarget);
        const float invToTargetLen = 1.0f / (toTargetLen + ade::kFloatMin);
        const Diligent::float3 toTargetNorm = ade::mul(toTarget, invToTargetLen);

        const Diligent::float3 right =
            Diligent::normalize(Diligent::cross({ 0.0f, 1.0f, 0.0f }, toTargetNorm));
        const Diligent::float3 up = Diligent::normalize(Diligent::cross(toTargetNorm, right));

        _mtx[0] = right.x;
        _mtx[1] = right.y;
        _mtx[2] = right.z;
        _mtx[3] = 0.0f;
        _mtx[4] = up.x;
        _mtx[5] = up.y;
        _mtx[6] = up.z;
        _mtx[7] = 0.0f;
        _mtx[8] = toTargetNorm.x;
        _mtx[9] = toTargetNorm.y;
        _mtx[10] = toTargetNorm.z;
        _mtx[11] = 0.0f;
        _mtx[12] = 0.0f;
        _mtx[13] = 0.0f;
        _mtx[14] = 0.0f;
        _mtx[15] = 1.0f;
    }

    struct Interp3f {
        Diligent::float3 curr;
        Diligent::float3 dest;
    };

    Interp3f m_target;
    Interp3f m_pos;
    float m_orbit[2];
};

struct Mouse {
    Mouse() : m_dx(0.0f), m_dy(0.0f), m_prevMx(0.0f), m_prevMy(0.0f), m_scroll(0), m_scrollPrev(0)
    {
    }

    void update(float _mx, float _my, int32_t _mz, uint32_t _width, uint32_t _height)
    {
        const float widthf = float(int32_t(_width));
        const float heightf = float(int32_t(_height));

        // Delta movement.
        m_dx = float(_mx - m_prevMx) / widthf;
        m_dy = float(_my - m_prevMy) / heightf;

        m_prevMx = _mx;
        m_prevMy = _my;

        // Scroll.
        m_scroll = _mz - m_scrollPrev;
        m_scrollPrev = _mz;
    }

    float m_dx;    // Screen space.
    float m_dy;
    float m_prevMx;
    float m_prevMy;
    int32_t m_scroll;
    int32_t m_scrollPrev;
};