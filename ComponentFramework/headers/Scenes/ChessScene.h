#ifndef ChessScene_H
#define ChessScene_H
#include <Core/Scene.h>
#include <Vector.h>
#include <Matrix.h>
#include <Graphics/LightActor.h>
#include <Core/Actor.h>
#include <Graphics/CameraActor.h>
#include <Physics/CollisionSystem.h>
#include <Graphics/AssetManager.h>
#include <unordered_map>

using namespace MATH;
/// Forward declarations 
union SDL_Event;


class ChessScene : public Scene {
private:
	std::unique_ptr<CameraActor> camera;
	std::shared_ptr<Actor> GameBoardActor;
	std::unique_ptr<CollisionSystem> collision_system_;
	//std::unique_ptr<AssetManager> asset_manager_;
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
	std::string selectedActorName = "";

	std::unordered_map<std::string, std::unique_ptr<Actor>> ActorList;
	std::unordered_map<std::string, std::shared_ptr<Component>> collision_boxes;
	std::unordered_map<std::string, std::unique_ptr<LightActor>> Lights;
	std::unordered_map<std::string, std::shared_ptr<Actor>> Resources;
	

	
	float m_sens = 0.3f;
	float consoleMessageTimer = 0.0f;
	std::string consoleOutput = "";
	const float CONSOLE_MESSAGE_DURATION = 5.0f; // Seconds to stay visible
	char commandInput[256] = ""; 
	bool showConsole = true; 

public:
	explicit ChessScene();
	virtual ~ChessScene();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event& sdlEvent) override;
	virtual void RenderGUI() override;

};


#endif // ChessScene_H