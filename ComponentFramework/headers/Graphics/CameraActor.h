#pragma once
#include <glew.h>
#include <Core/Actor.h>
#include <UI/Trackball.h>
#include <Matrix.h>
#include <MMath.h>
#include <QMath.h>
#include <SDL3/SDL_gamepad.h>

#include "Physics/PhysicsComponent.h"

using namespace MATH;

class CameraActor : public Actor
{
	Trackball trackball;
	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;
	Quaternion orientation;
	Vec3 position;
	GLuint textureID;
	float CameraSpeed = 20.0f;

	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;
	float m_Sensitivity = 60.0f;
	
public:
	CameraActor(std::weak_ptr<Component> parent_, float fovy, float aspectRatio, float near, float far);
	~CameraActor();
	Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	Matrix4 GetViewMatrix() const { return MMath::inverse(MMath::toMatrix4(GetOrientation())) * MMath::inverse(MMath::translate(GetPosition())); }
	void UpdateViewMatrix(const SDL_Event& sdlEvent);
	void setViewMatrix(const Matrix4& viewMatrix_) { viewMatrix = viewMatrix_; }

	void SetView(const Quaternion& orientation_, const Vec3& position_);

	Vec3 freeCameraMovement(Vec3 direction) {
		Matrix4 worldToCamera = this->GetViewMatrix();
		Matrix4 cameraToWorld = MMath::inverse(worldToCamera);
		Vec3 rotated_forward_in_cam_space = cameraToWorld * direction;
		return rotated_forward_in_cam_space;
		
	}

	Quaternion GetOrientation() const { return GetComponent<PhysicsComponent>()->GetQuaternion(); }
	Vec3 GetPosition() const { return GetComponent<PhysicsComponent>()->GetPosition(); }

	float GetCameraSpeed() const { return CameraSpeed; }
	void SetSensitivity(float sens) { trackball.SetSensitivity(sens); }
	void DontTrackXYRotations() {
		trackball.Trackingx = false;
		trackball.Trackingz = false;
	}
	void CameraMovement(float deltaTime);
	void CameraMovement(float deltaTime, SDL_Gamepad* gamepad);
	
	bool OnCreate() override;
	void Render();
};


