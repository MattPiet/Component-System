#ifndef SCENE0_H
#define SCENE0_H
#include <Core/Scene.h>
#include "Vector.h"
#include <Matrix.h>
#include <QMath.h>
#include <Core/Actor.h>
#include <Graphics/CameraActor.h>
#include <unordered_map>

using namespace MATH;
/// Forward declarations 
union SDL_Event;


class Scene0g : public Scene {
private:
	std::unique_ptr<CameraActor> camera;
	bool drawInWireMode;
	bool showImGuiDemoWindow = true; // optional for testing
	char textBuffer[256] = "";       // input text buffer
	int buttonClicks = 0;            // button counter
	class Window* window;
	SDL_GLContext context;
	SDL_Gamepad* gamepad = nullptr;

	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;
	Matrix4 modelMatrix;

	std::unordered_map<std::string, std::unique_ptr<Actor>> ActorList;
	

	// 1. Update your stored angles (Add these floats to your class)
	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;
	float m_Sensitivity = 100.0f;
	float CameraSpeed = 10.0f;

public:
	explicit Scene0g();
	virtual ~Scene0g();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event& sdlEvent) override;
	virtual void RenderGUI() override;

};


#endif // SCENE0_H