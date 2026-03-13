#pragma once
#include <glew.h>
#include <Core/Actor.h>
#include <UI/Trackball.h>
#include <Matrix.h>
#include <MMath.h>
#include <QMath.h>

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
public:
	CameraActor(std::weak_ptr<Component> parent_, float fovy, float aspectRatio, float near, float far);
	~CameraActor();
	Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	Matrix4 GetViewMatrix() const { return MMath::inverse(MMath::toMatrix4(orientation)) * MMath::inverse(MMath::translate(position)); }
	void UpdateViewMatrix(const SDL_Event& sdlEvent);
	void setViewMatrix(const Matrix4& viewMatrix_) { viewMatrix = viewMatrix_; }

	void SetView(const Quaternion& orientation_, const Vec3& position_);

	Vec3 freeCameraMovement(Vec3 direction) {
		Matrix4 worldToCamera = this->GetViewMatrix();
		Matrix4 cameraToWorld = MMath::inverse(worldToCamera);
		Vec3 rotated_forward_in_cam_space = cameraToWorld * direction;
		return rotated_forward_in_cam_space;
	}

	Quaternion GetOrientation() const { return orientation; }

	float GetCameraSpeed() const { return CameraSpeed; }

	void DontTrackXYRotations() {
		trackball.Trackingx = false;
		trackball.Trackingz = false;
	}
	bool OnCreate() override;
	void Render();
};


