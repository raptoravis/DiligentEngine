#include "engine/engine.h"

#include "engine/utils/mathutils.h"

#include "camera.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

//#define ARCBALL_CAMERA_IMPLEMENTATION
//#include "arcball_camera.h"


namespace ade
{

Camera::Camera()
{
    //
}

Camera::~Camera()
{
    //
}

void Camera::reset()
{
    m_viewMatrix = Diligent::float4x4::Identity();
}

void Camera::SetPos(const Diligent::float3& p)
{
    pos = p;
}

const Diligent::float3& Camera::GetPos() const
{
    return pos;
}

void Camera::SetLookAt(const Diligent::float3& target)
{
    lookAt = target;
}

Diligent::float3 Camera::GetLookDir() const
{
    Diligent::float3 lookDir{
        m_viewMatrix._31,
        m_viewMatrix._32,
        m_viewMatrix._33,
    };

    return lookDir;
}


CameraFly::CameraFly() : base()
{
#if RIGHT_HANDED
    bool bIsGL = true;
#else
    bool bIsGL = false;
#endif
    setProjectionMatrix(0.1f, 1000.f, bIsGL);
}

CameraFly::~CameraFly()
{
    //
}

void CameraFly::reset()
{
    base::reset();
}

static void SetNearFarClipPlanes(Diligent::float4x4& mat, float zNear, float zFar, bool bIsGL) {}

static Diligent::float4x4 _setProjectionMatrix(float fov, float aspectRatio, float zNear,
                                               float zFar, bool bIsGL)
{
    Diligent::float4x4 mOut;
    auto yScale = static_cast<float>(1) / std::tan(fov / static_cast<float>(2));
    auto xScale = yScale / aspectRatio;
    mOut._11 = xScale;
    mOut._22 = yScale;

    if (bIsGL) {
        // Note that OpenGL uses right-handed coordinate system, where
        // CameraFly is looking in negative z direction:
        //   OO
        //  |__|<--------------------
        //         -z             +z
        // Consequently, OpenGL projection matrix given by these two
        // references inverts z axis.

        // We do not need to do this, because we use DX coordinate
        // system for the CameraFly space. Thus we need to invert the
        // sign of the values in the third column in the matrix
        // from the references:

        mOut._33 = -(-(zFar + zNear) / (zFar - zNear));
        mOut._43 = -2 * zNear * zFar / (zFar - zNear);
        mOut._34 = -(-1);
    } else {
        mOut._33 = zFar / (zFar - zNear);
        mOut._43 = -zNear * zFar / (zFar - zNear);
        mOut._34 = 1;
    }


    return mOut;
}

void CameraFly::setProjectionMatrix(float NearPlane, float FarPlane, bool bIsGL)
{
    float aspectRatio =
        static_cast<float>(App::s_desc.Width) / static_cast<float>(App::s_desc.Height);

    if (bIsGL) {
        m_projectionMatrix =
            _setProjectionMatrix(Diligent::PI_F / 4.f, aspectRatio, NearPlane, FarPlane, bIsGL);
    } else {
        // Projection matrix differs between DX and OpenGL
        m_projectionMatrix = Diligent::float4x4::Projection(Diligent::PI_F / 4.f, aspectRatio,
                                                            NearPlane, FarPlane, bIsGL);
    }
}

void CameraFly::update(Diligent::InputController* pInputController, float ElapsedTime)
{
    const auto& mouseState = pInputController->GetMouseState();
    float MouseDeltaX = 0;
    float MouseDeltaY = 0;
    if (m_LastMouseState.PosX >= 0 && m_LastMouseState.PosY >= 0 &&
        m_LastMouseState.ButtonFlags != Diligent::MouseState::BUTTON_FLAG_NONE) {
        MouseDeltaX = mouseState.PosX - m_LastMouseState.PosX;
        MouseDeltaY = mouseState.PosY - m_LastMouseState.PosY;
    }
    m_LastMouseState = mouseState;

    {
        int moveLeft = 0;
        if ((pInputController->GetKeyState(Diligent::InputKeys::MoveLeft) &
             Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
            moveLeft = 1;
        }
        int moveRight = 0;
        if ((pInputController->GetKeyState(Diligent::InputKeys::MoveRight) &
             Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
            moveRight = 1;
        }
        int moveForward = 0;
        if ((pInputController->GetKeyState(Diligent::InputKeys::MoveForward) &
             Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
            moveForward = 1;
        }
        int moveBackward = 0;
        if ((pInputController->GetKeyState(Diligent::InputKeys::MoveBackward) &
             Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
            moveBackward = 1;
        }
        int jump = 0;
        if ((pInputController->GetKeyState(Diligent::InputKeys::MoveUp) &
             Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
            jump = 1;
        }
        int crouch = 0;
        if ((pInputController->GetKeyState(Diligent::InputKeys::MoveDown) &
             Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
            crouch = 1;
        }
        int accelerate = 0;
        if ((pInputController->GetKeyState(Diligent::InputKeys::ShiftDown) &
             Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
            accelerate = 1;
        }

        float delta_time_sec = ElapsedTime;
#if RIGHT_HANDED
        int mf = moveForward;
        int mb = moveBackward;
        int flag = FLYTHROUGH_CAMERA_LEFT_HANDED_BIT;

        // side: 1, 0, 0
        // up : 0, 1, 0
        // dir: 0, 0, -1
#else
        int mf = moveBackward;
        int mb = moveForward;

        // dir is inverted
        // side: 1, 0, 0
        // up : 0, 1, 0
        // dir: 0, 0, -(-1)
        int flag = 0;
        // int flag = FLYTHROUGH_CameraFly_LEFT_HANDED_BIT;
#endif
        const Diligent::float3 up = { 0, 1, 0 };
        Diligent::float3 lookDir = Diligent::normalize(lookAt - pos);

        float* view = &m_viewMatrix.m00;

        // flythrough_camera_update calculate in opengl right-handed space
        // side = cross(look, up) (0, 0, -1) X (0, 1, 0) => (1, 0, 0)
        // up = cross(side, look)
        // if !FLYTHROUGH_CAMERA_LEFT_HANDED_BIT, lookDir = -lookDir
        flythrough_camera_update(&pos.x, &lookDir.x, &up.x, view, delta_time_sec,
                                 2.0f * (accelerate ? 5.0f : 1.0f),    // eye_speed
                                 0.1f,                                 // degrees_per_cursor_move
                                 80.0f,                                // max_pitch_rotation_degrees
                                 (int)MouseDeltaX, (int)MouseDeltaY, mf, moveLeft, mb, moveRight,
                                 jump, crouch, flag);

        // m_viewMatrix = m_viewMatrix.Transpose();
    }

    if ((pInputController->GetKeyState(Diligent::InputKeys::Reset) &
         Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
        reset();
    }
}


CameraAlt::CameraAlt()
{
    reset();

    float aspect = float(App::s_desc.Width) / float(App::s_desc.Height);

    mtxProj((float*)m_projectionMatrix.m, 60.0f, aspect, 0.1f, 500.0f, false, Handness::Left);
}

CameraAlt::~CameraAlt()
{
    //
}

void CameraAlt::reset()
{
    base::reset();

    m_target.curr = { 0.0f, 0.0f, 0.0f };
    m_target.dest = { 0.0f, 0.0f, 0.0f };

    m_pos.curr = { 55.0f, 20.0f, 65.0f };
    m_pos.dest = { 55.0f, 20.0f, 65.0f };

    m_orbit[0] = 0.0f;
    m_orbit[1] = 0.0f;
}

void CameraAlt::mtxLookAt(Diligent::float4x4& _outViewMtx)
{
    ade::mtxLookAt(_outViewMtx, m_pos.curr, m_target.curr);
}

void CameraAlt::orbit(float _dx, float _dy)
{
    m_orbit[0] += _dx;
    m_orbit[1] += _dy;
}

void CameraAlt::dolly(float _dz)
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

void CameraAlt::consumeOrbit(float _amount)
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

const Diligent::float3& CameraAlt::GetTarget() const
{
    return this->m_target.curr;
}


void CameraAlt::SetTarget(const Diligent::float3& target)
{
    this->m_target.dest = target;
}


void CameraAlt::update(Diligent::InputController* pInputController, float _dt)
{
    Diligent::MouseState mouseState = pInputController->GetMouseState();

    m_mouse.update(float(mouseState.PosX), float(mouseState.PosY), 0, App::s_desc.Width,
                   App::s_desc.Height);

    {
        if (mouseState.ButtonFlags & Diligent::MouseState::BUTTON_FLAG_LEFT) {
            this->orbit(m_mouse.m_dx, m_mouse.m_dy);
        } else if (mouseState.ButtonFlags & Diligent::MouseState::BUTTON_FLAG_RIGHT) {
            this->dolly(m_mouse.m_dx + m_mouse.m_dy);
        } else if (m_mouse.m_scroll != 0) {
            this->dolly(float(m_mouse.m_scroll) * 0.05f);
        }
    }

    {
        const float amount = std::min(_dt / 0.12f, 1.0f);

        consumeOrbit(amount);

        m_target.curr = Diligent::lerp(m_target.curr, m_target.dest, amount);
        m_pos.curr = Diligent::lerp(m_pos.curr, m_pos.dest, amount);

        mtxLookAt(m_viewMatrix);

        pos = m_pos.curr;
    }
}

void CameraAlt::envViewMtx(float* _mtx)
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

}    // namespace ade