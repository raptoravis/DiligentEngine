#include "engine.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

//#define ARCBALL_CAMERA_IMPLEMENTATION
//#include "arcball_camera.h"


Camera::Camera() {
	reset();
}

Camera::~Camera() {
	//
}

void Camera::reset() {
	pos = { 0.0f, 0.0f, 0.0f };

	look = { 0.0f, 0.0f, -1.0f };
	//look = { -1.0f, 0.0f, 0.0f };

	m_cameraTransform = Diligent::float4x4::Identity();
}

void Camera::update(Diligent::InputController* pInputController, float ElapsedTime) {
	const auto& mouseState = pInputController->GetMouseState();
	float MouseDeltaX = 0;
	float MouseDeltaY = 0;
	if (m_LastMouseState.PosX >= 0 && m_LastMouseState.PosY >= 0 &&
		m_LastMouseState.ButtonFlags != Diligent::MouseState::BUTTON_FLAG_NONE)
	{
		MouseDeltaX = mouseState.PosX - m_LastMouseState.PosX;
		MouseDeltaY = mouseState.PosY - m_LastMouseState.PosY;
	}
	m_LastMouseState = mouseState;

	{
		int moveLeft = 0;
		if ((pInputController->GetKeyState(Diligent::InputKeys::MoveLeft) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
			moveLeft = 1;
		}
		int moveRight = 0;
		if ((pInputController->GetKeyState(Diligent::InputKeys::MoveRight) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
			moveRight = 1;
		}
		int moveForward = 0;
		if ((pInputController->GetKeyState(Diligent::InputKeys::MoveForward) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
			moveForward = 1;
		}
		int moveBackward = 0;
		if ((pInputController->GetKeyState(Diligent::InputKeys::MoveBackward) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
			moveBackward = 1;
		}
		int jump = 0;
		if ((pInputController->GetKeyState(Diligent::InputKeys::MoveUp) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
			jump = 1;
		}
		int crouch = 0;
		if ((pInputController->GetKeyState(Diligent::InputKeys::MoveDown) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
			crouch = 1;
		}
		int accelerate = 0;
		if ((pInputController->GetKeyState(Diligent::InputKeys::ShiftDown) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
			accelerate = 1;
		}

		float delta_time_sec = ElapsedTime;
		//int flag = FLYTHROUGH_CAMERA_LEFT_HANDED_BIT;
		int flag = 0;

		float* view = &m_cameraTransform.m00;

		flythrough_camera_update(
			&pos.x, &look.x, &up.x, view,
			delta_time_sec,
			2.0f * (accelerate ? 5.0f : 1.0f),	//eye_speed
			0.1f,								//degrees_per_cursor_move
			80.0f,								//max_pitch_rotation_degrees
			(int)MouseDeltaX, (int)MouseDeltaY,
			moveBackward, moveLeft, moveForward, moveRight,
			jump, crouch,
			flag);
	}

	if ((pInputController->GetKeyState(Diligent::InputKeys::Reset) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
		reset();
	}
}
