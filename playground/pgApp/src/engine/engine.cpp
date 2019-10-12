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
	//look = { 0.0f, 0.0f, 1.0f };
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

		//m_cameraTransform = m_cameraTransform.Transpose();
	}

	if ((pInputController->GetKeyState(Diligent::InputKeys::Reset) & Diligent::INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0) {
		reset();
	}
}

pgBasePass::pgBasePass(Diligent::IRenderDevice* device, Diligent::IDeviceContext* pCtx, Diligent::IEngineFactory* factory)
	: base()
	, m_pDevice(device)
	, m_pImmediateContext(pCtx)
	, m_pEngineFactory(factory)
{
}

pgBasePass::~pgBasePass()
{
}


void pgBasePass::Update(RenderEventArgs& e)
{
}

void pgBasePass::Render(RenderEventArgs& e)
{
}

void pgBasePass::UpdateUI(RenderEventArgs& e)
{
}


pgTechnique::pgTechnique()
{}

pgTechnique::~pgTechnique()
{}

unsigned int pgTechnique::AddPass(pgPass* pass)
{
	// No check for duplicate passes (it may be intended to render the same pass multiple times?)
	m_Passes.push_back(pass);
	return static_cast<unsigned int>(m_Passes.size()) - 1;
}

pgPass* pgTechnique::GetPass(unsigned int ID) const
{
	if (ID < m_Passes.size())
	{
		return m_Passes[ID];
	}

	return 0;
}

void pgTechnique::Update(RenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->IsEnabled())
		{
			pass->Update(e);
		}
	}
}

// Render the scene using the passes that have been configured.
void pgTechnique::Render(RenderEventArgs& e)
{
	for (auto pass : m_Passes)
	{
		if (pass->IsEnabled())
		{
			pass->Render(e);
		}
	}
}


