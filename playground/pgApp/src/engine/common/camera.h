#pragma once

#include "BasicMath.h"
#include "SampleBase.h"

#include "engine/utils/mathutils.h"

namespace ade
{

class CameraFly : public Camera
{
    typedef Camera base;

    Diligent::MouseState m_LastMouseState;

    void setProjectionMatrix(float NearPlane, float FarPlane, bool bIsGL);

  public:
    CameraFly();
    virtual ~CameraFly();

	virtual void reset();
    virtual void update(Diligent::InputController* pInputController, float ElapsedTime);

    const Diligent::float4x4& getViewMatrix() const { return m_viewMatrix; }

    const Diligent::float4x4& getProjectionMatrix() const { return m_projectionMatrix; }
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

class CameraAlt : public Camera
{
    typedef Camera base;

    void mtxLookAt(Diligent::float4x4& _outViewMtx);

    void orbit(float _dx, float _dy);

    void dolly(float _dz);

    void consumeOrbit(float _amount);

    void update(float _dt);

    void envViewMtx(float* _mtx);

    struct Interp3f {
        Diligent::float3 curr;
        Diligent::float3 dest;
    };

    Interp3f m_target;
    Interp3f m_pos;
    float m_orbit[2];

    Mouse m_mouse;

  public:
    CameraAlt();
    virtual ~CameraAlt();

	virtual void reset();
    virtual void update(Diligent::InputController* pInputController, float _dt);

	const Diligent::float3& GetTarget() const;
    void SetTarget(const Diligent::float3& target);

    const Diligent::float4x4& getViewMatrix() const { return m_viewMatrix; }

    const Diligent::float4x4& getProjectionMatrix() const { return m_projectionMatrix; }
};


}    // namespace ade