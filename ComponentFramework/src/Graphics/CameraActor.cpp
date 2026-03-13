#include <Graphics/CameraActor.h>
#include <Physics/TransformComponent.h>
#include <Core/Debug.h>
#include <SDL3/SDL.h>
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
		orientation = trackball.getQuat(); // Sync camera to trackball
	}
}
void CameraActor::SetView(const Quaternion& orientation_, const Vec3& position_) {
	orientation = orientation_;
	position = position_;
}

void CameraActor::Render(){}