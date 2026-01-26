#include <Graphics/CameraActor.h>
#include <Physics/TransformComponent.h>
#include <Core/Debug.h>
CameraActor::CameraActor(Actor* parent_, float fovy, float aspectRatio, float near, float far) : Actor(parent_), 
orientation(), projectionMatrix(), viewMatrix(), position(), trackball(), textureID(0)
{
	projectionMatrix = MMath::perspective(fovy, aspectRatio, near, far);
	viewMatrix.loadIdentity();
}

CameraActor::~CameraActor()
{
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
	trackball.HandleEvents(sdlEvent);
	 orientation = trackball.getQuat();
}
void CameraActor::SetView(const Quaternion& orientation_, const Vec3& position_) {
	orientation = orientation_;
	position = position_;
}

void CameraActor::Render(){}