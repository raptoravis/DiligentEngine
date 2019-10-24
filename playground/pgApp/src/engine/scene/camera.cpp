#include "../engine.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

//#define ARCBALL_CAMERA_IMPLEMENTATION
//#include "arcball_camera.h"

pgCamera::pgCamera(const Diligent::float3 pos, const Diligent::float3 dir) : _pos(pos), _look(dir)
{
    reset(_pos, _look);

#if RIGHT_HANDED
    bool bIsGL = true;
#else
    bool bIsGL = false;
#endif
    setProjectionMatrix(0.1f, 1000.f, bIsGL);
}

pgCamera::~pgCamera()
{
    //
}


void pgCamera::reset()
{
    reset(_pos, _look);
}

void pgCamera::reset(const Diligent::float3& p, const Diligent::float3& dir)
{
    // pos = { 0.0f, 0.0f, 0.0f };
    setPos(p);

    void setPos(const Diligent::float3& p);
    setLook(dir);
    // look = { 0.0f, 0.0f, -1.0f };
    // look = { 0.0f, 0.0f, 1.0f };
    // look = { -1.0f, 0.0f, 0.0f };

    m_viewMatrix = Diligent::float4x4::Identity();
}

void pgCamera::setLook(const Diligent::float3& dir)
{
    look = dir;
}

Diligent::float3 pgCamera::getLook() const
{
    return Diligent::float3(m_viewMatrix._31, m_viewMatrix._32, m_viewMatrix._33);
}

void pgCamera::setPos(const Diligent::float3& p)
{
    pos = p;
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
        // camera is looking in negative z direction:
        //   OO
        //  |__|<--------------------
        //         -z             +z
        // Consequently, OpenGL projection matrix given by these two
        // references inverts z axis.

        // We do not need to do this, because we use DX coordinate
        // system for the camera space. Thus we need to invert the
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

void pgCamera::setProjectionMatrix(float NearPlane, float FarPlane, bool bIsGL)
{
    float aspectRatio =
        static_cast<float>(pgApp::s_desc.Width) / static_cast<float>(pgApp::s_desc.Height);

    if (bIsGL) {
        m_projectionMatrix =
            _setProjectionMatrix(Diligent::PI_F / 4.f, aspectRatio, NearPlane, FarPlane, bIsGL);
    } else {
        // Projection matrix differs between DX and OpenGL
        m_projectionMatrix = Diligent::float4x4::Projection(Diligent::PI_F / 4.f, aspectRatio,
                                                            NearPlane, FarPlane, bIsGL);
    }
}

void pgCamera::update(Diligent::InputController* pInputController, float ElapsedTime)
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
        //int flag = FLYTHROUGH_CAMERA_LEFT_HANDED_BIT;
#endif

        float* view = &m_viewMatrix.m00;

		// flythrough_camera_update calculate in opengl right-handed space
		// side = cross(look, up) (0, 0, -1) X (0, 1, 0) => (1, 0, 0)
		// up = cross(side, look)
		// if !FLYTHROUGH_CAMERA_LEFT_HANDED_BIT, look = -look
        flythrough_camera_update(&pos.x, &look.x, &up.x, view, delta_time_sec,
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
