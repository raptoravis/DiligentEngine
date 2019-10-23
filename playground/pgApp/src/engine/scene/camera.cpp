#include "../engine.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

//#define ARCBALL_CAMERA_IMPLEMENTATION
//#include "arcball_camera.h"

pgCamera::pgCamera(const Diligent::float3 pos, const Diligent::float3 dir) : _pos(pos), _look(dir)
{
    reset(_pos, _look);
    setProjectionMatrix(0.1f, 1000.f);
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
    pos = p;

    look = dir;
    // look = { 0.0f, 0.0f, -1.0f };
    // look = { 0.0f, 0.0f, 1.0f };
    // look = { -1.0f, 0.0f, 0.0f };

    m_viewMatrix = Diligent::float4x4::Identity();
}

//Diligent::float4x4 perspectiveRH_NO(float fovy, float aspect, float zNear, float zFar)
//{
//    assert(abs(aspect - std::numeric_limits<float>::epsilon()) > static_cast<float>(0));
//
//    float const tanHalfFovy = tan(fovy / static_cast<float>(2));
//
//    Diligent::float4x4 Result(static_cast<float>(0));
//
//    Result[0][0] = static_cast<float>(1) / (aspect * tanHalfFovy);
//    Result[1][1] = static_cast<float>(1) / (tanHalfFovy);
//    Result[2][2] = -(zFar + zNear) / (zFar - zNear);
//    Result[2][3] = -static_cast<float>(1);
//    Result[3][2] = -(static_cast<float>(2) * zFar * zNear) / (zFar - zNear);
//    return Result;
//}


void pgCamera::setProjectionMatrix(float NearPlane, float FarPlane, bool bRightHanded)
{
    float aspectRatio =
        static_cast<float>(pgApp::s_desc.Width) / static_cast<float>(pgApp::s_desc.Height);

    // Projection matrix differs between DX and OpenGL
    m_projectionMatrix = Diligent::float4x4::Projection(Diligent::PI_F / 4.f, aspectRatio,
                                                        NearPlane, FarPlane, bRightHanded);
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
        // int flag = FLYTHROUGH_CAMERA_LEFT_HANDED_BIT;
        int flag = 0;

        float* view = &m_viewMatrix.m00;

        flythrough_camera_update(&pos.x, &look.x, &up.x, view, delta_time_sec,
                                 2.0f * (accelerate ? 5.0f : 1.0f),    // eye_speed
                                 0.1f,                                 // degrees_per_cursor_move
                                 80.0f,                                // max_pitch_rotation_degrees
                                 (int)MouseDeltaX, (int)MouseDeltaY, moveBackward, moveLeft,
                                 moveForward, moveRight, jump, crouch, flag);

        // m_viewMatrix = m_viewMatrix.Transpose();
    }

    if ((pInputController->GetKeyState(Diligent::InputKeys::Reset) &
         Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
        reset();
    }
}
