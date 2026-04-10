#include <Graphics/CameraActor.h>
#include <Physics/TransformComponent.h>
#include <Core/Debug.h>
#include <SDL3/SDL.h>
#include <UI/UIManager.h>

#include "Physics/PhysicsComponent.h"

CameraActor::CameraActor(std::weak_ptr<Component> parent_, float fovy, float aspectRatio, float near, float far) : Actor(parent_), 
                                                                                                                   orientation(), projectionMatrix(), viewMatrix(), position(), trackball(), textureID(0)
{
	projectionMatrix = MMath::perspective(fovy, aspectRatio, near, far);
	viewMatrix.loadIdentity();
}

CameraActor::~CameraActor()
{
	OnDestroy();
}

void CameraActor::CameraMovement(float deltaTime)
{
	if (!ImGui::GetIO().WantCaptureKeyboard) 
	{
	Vec3 velocity(0.0f, 0.0f, 0.0f);
		const bool* keyboardState = SDL_GetKeyboardState(NULL);
		if (keyboardState[SDL_SCANCODE_W])      velocity.z -= GetCameraSpeed();
		if (keyboardState[SDL_SCANCODE_S])      velocity.z += GetCameraSpeed();
		if (keyboardState[SDL_SCANCODE_A])      velocity.x -= GetCameraSpeed();
		if (keyboardState[SDL_SCANCODE_D])      velocity.x += GetCameraSpeed();
		if (keyboardState[SDL_SCANCODE_SPACE])  velocity.y += GetCameraSpeed();
		if (keyboardState[SDL_SCANCODE_LSHIFT]) velocity.y -= GetCameraSpeed();
		if (VMath::mag(velocity) > 0.0f) {
			velocity = VMath::normalize(velocity);
			Vec3 displacement = velocity * GetCameraSpeed() * deltaTime;
			SetView(GetOrientation(), freeCameraMovement(displacement));
		}
	}
}

void CameraActor::CameraMovement(float deltaTime, SDL_Gamepad* gamepad)
{
	if (gamepad && SDL_GamepadConnected(gamepad)) {
		const float deadzone = 0.2f; 

		float stickX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f;
		float stickY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f;

		Vec3 movement(0.0f,0.0f,0.0f);

		if (SDL_fabsf(stickX) > deadzone) movement.x = stickX;
		if (SDL_fabsf(stickY) > deadzone) movement.z = stickY;

		if (movement.x != 0.0f || movement.z != 0.0f) {

			SetView(GetOrientation(),
				freeCameraMovement(movement * GetCameraSpeed() * deltaTime));
		}

		float rightStickX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX) / 32767.0f;
		float rightStickY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) / 32767.0f;

		if (SDL_fabsf(rightStickX) > deadzone || SDL_fabsf(rightStickY) > deadzone) {

			 m_Yaw += -rightStickX * m_Sensitivity * deltaTime;
			 m_Pitch += -rightStickY * m_Sensitivity * deltaTime;

			if (m_Pitch > 89.0f)  m_Pitch = 89.0f;
			if (m_Pitch < -89.0f) m_Pitch = -89.0f;

			Quaternion qYaw = QMath::angleAxisRotation(m_Yaw, Vec3(0.0f, 1.0f, 0.0f));
			Quaternion qPitch = QMath::angleAxisRotation(m_Pitch, Vec3(1.0f, 0.0f, 0.0f));

			GetComponent<PhysicsComponent>()->SetOrientation(qYaw * qPitch);
			SetView(GetComponent<PhysicsComponent>()->GetQuaternion(), freeCameraMovement(movement * GetCameraSpeed() * deltaTime));
		}
	}
	// Camera Movement with keyboard inputs
	// tbh I just wanted to see if I could apply what I did with the controller to track ball and movement 
	// very happy with how it turned out.
	// I think Ill put all this code into the camera class later but I left it here so you could see it

	int16_t leftTrigger  = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
	int16_t rightTrigger = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

	// 2. Use a consistent threshold (deadzone)
	Vec3 velocity(0.0f, 0.0f, 0.0f);
	const int16_t triggerDeadzone = 8000; 

	if (leftTrigger > triggerDeadzone) {
		velocity.y -= GetCameraSpeed();
	}
	if (rightTrigger > triggerDeadzone) {
		velocity.y += GetCameraSpeed();
	}
	
	if (!ImGui::GetIO().WantCaptureKeyboard) 
	{
	const bool* keyboardState = SDL_GetKeyboardState(NULL);
	if (keyboardState[SDL_SCANCODE_W])      velocity.z -= GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_S])      velocity.z += GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_A])      velocity.x -= GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_D])      velocity.x += GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_SPACE])  velocity.y += GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_LSHIFT]) velocity.y -= GetCameraSpeed();
	if (VMath::mag(velocity) > 0.0f) {
		velocity = VMath::normalize(velocity);
		Vec3 displacement = velocity * GetCameraSpeed() * deltaTime;
		SetView(GetOrientation(), freeCameraMovement(displacement));
	}
	}
}

bool CameraActor::OnCreate()
{
	if (isCreated) return true;
	Debug::Info("Loading assets for Actor: ", __FILE__, __LINE__);
	for (auto component : components) {
		if (component->OnCreate() == false) {
			Debug::Error("Component failed OnCreate: " + std::string(typeid(*component).name()), __FILE__, __LINE__);
			isCreated = false;
			return isCreated;
		}
	}
	isCreated = true;
	return isCreated;
}

void CameraActor::UpdateViewMatrix(const SDL_Event &sdlEvent)
{
	if (sdlEvent.type == SDL_EVENT_MOUSE_WHEEL)
	{
		if (SDL_GetGlobalMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
			CameraSpeed += sdlEvent.wheel.y * 2.0f; 
		}
		if (CameraSpeed < 0.0f) CameraSpeed = 0.0f;
		if (CameraSpeed > 100.0f) CameraSpeed = 100.0f;
	}
	// Only let the trackball update if it's a mouse event
	if (sdlEvent.type == SDL_EVENT_MOUSE_MOTION ||
		sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
		sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
		trackball.HandleEvents(sdlEvent);
		GetComponent<PhysicsComponent>()->SetOrientation(trackball.getQuat()); // Sync camera to trackball
	}
}
void CameraActor::SetView(const Quaternion& orientation_, const Vec3& position_) {
	GetComponent<PhysicsComponent>()->SetOrientation(orientation_);
	GetComponent<PhysicsComponent>()->set_position(position_);
	
	this->position = position_;
	this->orientation = orientation_;
}

void CameraActor::Render(){}