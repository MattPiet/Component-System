#include <glew.h>
#include <iostream>
#include <SDL.h>
#include <Scenes/Scene0g.h>
#include <MMath.h>
#include <Core/Debug.h>
#include <Core/GuiWindow.h>
#include <Graphics/MaterialComponent.h>
#include <Graphics/MeshComponent.h>
#include <Graphics/ShaderComponent.h>
#include <Physics/TransformComponent.h>
#include <Graphics/SkyBoxComponent.h>
#include <random>
#include <UI/UIManager.h>
#include <Utils/MemoryMonitor.h>

Scene0g::Scene0g() :
drawInWireMode{false},
window{ nullptr }, 
context{ nullptr }
{
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene0g::~Scene0g() {
	Debug::Info("Deleted Scene0: ", __FILE__, __LINE__);
}
/*
		Hey Hey Hey read me 

	Press F1 & F2 to Scene Switch
	WASD to move camera
	Shift to go down and space to go up
	Left click and drag to look around

	Hey Scott this all runs on a different version of GameDev that has ImGui 
	I have it built into the project file so it should just run without any issues at all 
	let me know if something is bugged and Ill fix it instantly.
*/
bool Scene0g::OnCreate() {
	// me make camera here I wanted to have a skybox so I just made it a material comp
	camera = std::make_unique<CameraActor>(nullptr, 45.0f, 16.0f / 9.0f, 0.5f, 100.0f);
	camera->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, -5.0f), Quaternion());
	camera->AddComponent<ShaderComponent>(nullptr, "shaders/skyBoxVert.glsl", "shaders/skyBoxFrag.glsl");
	camera->AddComponent<MeshComponent>(nullptr, "meshes/Cube.obj");
	camera->AddComponent<SkyBoxComponent>(nullptr,
		"textures/skybox/StarSkyboxPosx.png",
		"textures/skybox/StarSkyboxNegx.png",
		"textures/skybox/StarSkyboxPosy.png",
		"textures/skybox/StarSkyboxNegY.png",
		"textures/skybox/StarSkyboxPosz.png",
		"textures/skybox/StarSkyboxnegz.png");
	camera->OnCreate();
	
	// this is the foundation all living things come back to the game board. Literally everything except the camera is parented to it
	std::unique_ptr<Actor> GameBoardActor = std::make_unique<Actor>(nullptr);
	GameBoardActor->AddComponent<MaterialComponent>(nullptr, "textures/Red&Black_Board.png");
	GameBoardActor->AddComponent<MeshComponent>(nullptr, "meshes/Plane.obj");
	GameBoardActor->AddComponent<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	GameBoardActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, -1.5f, -5.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	GameBoardActor->OnCreate();

	GameBoardActor->GetComponent<TransformComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(-1.0f, 0.0f, 0.0f)));
	GameBoardActor->GetComponent<TransformComponent>()->SetScale(Vec3(5.0f, 5.0f, 5.0f));
	// this is me map I made one map that stores all physical actors. I did a map cuz its easier to track stuff
	ActorList.emplace("GameBoard", std::move(GameBoardActor));
	
	// go get board and test id its actually valid
	auto gameBoardIt = ActorList.find("GameBoard");
	Actor* gameBoardParent = (gameBoardIt != ActorList.end()) ? gameBoardIt->second.get() : nullptr;
	// This is the parent piece for all checker pieces.
	std::unique_ptr<Actor> ParentActor = std::make_unique<Actor>(gameBoardParent);
	ParentActor->AddComponent<MaterialComponent>(nullptr, "textures/blackCheckerPiece.png");
	ParentActor->AddComponent<MeshComponent>(nullptr, "meshes/CheckerPiece.obj");
	ParentActor->OnCreate();
	ActorList.emplace("ParentPiece", std::move(ParentActor));
	// This is basically a material for all the Red ones
	std::unique_ptr<Actor> ParentActorRED = std::make_unique<Actor>(gameBoardParent);
	ParentActorRED->AddComponent<MaterialComponent>(nullptr, "textures/redCheckerPiece.png");
	ParentActorRED->OnCreate();
	ActorList.emplace("ParentPieceRED", std::move(ParentActorRED));
	// This actually makes every piece assigning them to be red or black.
	for (int i = 0; i < 24; i++) {
		std::unique_ptr<Actor> actor = std::make_unique<Actor>(gameBoardParent);
		actor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
		actor->OnCreate();
		std::string actorName;
		if (i % 2 == 0) {
			actorName = "CheckerPieceB" + std::to_string(i / 2);
		}
		else {
			actorName = "CheckerPieceR" + std::to_string(i / 2);
		}
		actor->GetComponent<TransformComponent>()->SetScale(Vec3(0.5f, 0.5f, 0.5f));
		ActorList.emplace(actorName, std::move(actor));
	}

	// This code actually sets up their positions on the board.
	Vec3 startPos = Vec3(-22.25f, -22.25f, 0.0f);
	float xStep = 6.35f;
	float yStep = 6.35f;
	for (int i = 0; i < 12; i++) {
		// Basically setup board rows and columns. For the near end
		int row = i / 4;
		int col = (i % 4) * 2 + (row % 2);
		// name it and check if its valid
		std::string name = "CheckerPieceB" + std::to_string(i);
		if (ActorList.count(name)) {
			// if its valid set its position based on the row and column
			float newX = startPos.x + (col * xStep);
			float newY = startPos.y + (row * yStep);
			ActorList.at(name)->GetComponent<TransformComponent>()->SetPosition(Vec3(newX, newY, 0.0f));
		}
	}
	// This is the same thing but for the far end
	for (int j = 0; j <= 12; j++) {
		int index = j;
		int row = (index / 4) + 5; 
		int col = (index % 4) * 2 + (row % 2);
		// name it and check if its valid
		std::string name = "CheckerPieceR" + std::to_string(j);
		// if its valid set its position based on the row and column
		if (ActorList.count(name)) {
			float newX = startPos.x + (col * xStep);
			float newY = startPos.y + (row * yStep);
			ActorList.at(name)->GetComponent<TransformComponent>()->SetPosition(Vec3(newX, newY, 0.0f));
		}
	}
	// This is the start of the light setup. Below are the positions
	Vec3 positions[5] = {
		Vec3(-20.0f,  0.0f,   10.0f), // Far Left 
		Vec3(20.0f,  0.0f,   10.0f), // Far Right 
		Vec3(-10.0f, -20.0f,  10.0f), // Close Left 
		Vec3(10.0f, -20.0f,  10.0f), // Close Right 
		Vec3(0.0f,  0.0f,   10.0f)  // Dead Center
	};

	// Diff Colours
	Vec4 diffuseColors[5] = {
		Vec4(0.4f, 0.4f, 0.6f, 1.0f), // Far Left
		Vec4(0.6f, 0.4f, 0.4f, 1.0f), // Far Right
		Vec4(0.5f, 0.5f, 0.4f, 1.0f), // Close Left
		Vec4(0.4f, 0.5f, 0.4f, 1.0f), // Close Right
		Vec4(0.5f, 0.5f, 0.5f, 1.0f)  // Dead Center
	};
	// Create lights and set their properties
	for (int i = 0; i < 5; i++) {
		std::string lightName = "Light" + std::to_string(i);
		std::unique_ptr<LightActor> Light = std::make_unique<LightActor>(gameBoardParent);

		Light->AddComponent<TransformComponent>(nullptr, positions[i], Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
		Light->OnCreate();
		// right now they all have the same spec but I might change it later so I just made it an array like the diffuse
		Light->SetSpecular(Vec4(0.5f, 0.5f, 0.5f, 1.0f));
		Light->SetDiffuse(diffuseColors[i]);
		Light->SetAmbient(Vec4(0.05f, 0.05f, 0.05f, 1.0f));
		// Put them into a resource map.... im just being fancy I couldve made an array of them but this is more fun
		Resources.emplace(lightName, std::move(Light));
	};
	// joystick setup
	int count;
	SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
	if (gamepads) {
		if (count > 0) {
			gamepad = SDL_OpenGamepad(gamepads[0]);
			Debug::Info("Gamepad found on startup.", __FILE__, __LINE__);
		}
		SDL_free(gamepads);
	}
	Vec3 offset = Vec3(0.0f, 0.0f, 15.0f);
	Vec3 rotatedOffset = QMath::rotate(offset, camera->GetOrientation());
	Vec3 cameraPos = Vec3(0.0f, 0.0f, 0.0f) + rotatedOffset;
	camera->SetView(camera->GetOrientation(), cameraPos);
	//camera->DontTrackXYRotations();
	//ActorList.at("CheckerPieceR0")->GetComponent<TransformComponent>()->SetPosition(Vec3(0.0f, 0.0f, 5.0f));
	return true;
}

void Scene0g::OnDestroy() {
	// begone memory leaks
	ActorList.clear(); 
	camera.reset();
	Resources.clear();
	if (gamepad) {
		SDL_CloseGamepad(gamepad); 
		gamepad = nullptr;
	}
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);

}

void Scene0g::HandleEvents(const SDL_Event &sdlEvent) {
	//make camera respond to input
		camera->UpdateViewMatrix(sdlEvent);
	
	switch( sdlEvent.type ) {
    case SDL_EVENT_KEY_DOWN:
		switch (sdlEvent.key.scancode) {
			case SDL_SCANCODE_L:
				drawInWireMode = !drawInWireMode;
				break;
			case SDL_SCANCODE_LEFT:
				// move board
				ActorList["GameBoard"]->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"]->GetComponent<TransformComponent>()->GetPosition() + Vec3(-1.0f, 0.0f, 0.0f));
				break;
			case SDL_SCANCODE_RIGHT:
				ActorList["GameBoard"]->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"]->GetComponent<TransformComponent>()->GetPosition() + Vec3(1.0f, 0.0f, 0.0f));
				break;
			case SDL_SCANCODE_UP:
				ActorList["GameBoard"]->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"]->GetComponent<TransformComponent>()->GetPosition() + Vec3(0.0f, 1.0f, 0.0f));
				break;
			case SDL_SCANCODE_DOWN:
				ActorList["GameBoard"]->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"]->GetComponent<TransformComponent>()->GetPosition() + Vec3(0.0f, -1.0f, 0.0f));
				break;
		}
		break;
		// this is so I can plug in a controller while the program is runnning
		case SDL_EVENT_GAMEPAD_ADDED:
			if (!gamepad) {
				gamepad = SDL_OpenGamepad(sdlEvent.gdevice.which);
				Debug::Info("Gamepad connected.", __FILE__, __LINE__);
				int count;
				SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
				if (gamepads) {
					if (count > 0) {
						gamepad = SDL_OpenGamepad(gamepads[0]);
						Debug::Info("Gamepad found on Event.", __FILE__, __LINE__);
					}
					SDL_free(gamepads); 
				}
			}
			break;
			// this detects when one is removed during run time
		case SDL_EVENT_GAMEPAD_REMOVED:
			if (gamepad && sdlEvent.gdevice.which == SDL_GetGamepadID(gamepad)) {
				SDL_CloseGamepad(gamepad);
				gamepad = nullptr;
			}
			break;
		
			break;
			// basic button input for controller.
	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		switch (sdlEvent.gbutton.button) {
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			drawInWireMode = !drawInWireMode;
			break;
		}
		break;
	case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		if (sdlEvent.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
			if (sdlEvent.gaxis.value > 16000) { // If pressed more than halfway
				camera->SetView(camera->GetOrientation(),
					camera->freeCameraMovement(Vec3(0.0f, 1.0f, 0.0f)));
			}
		}
		else if (sdlEvent.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) {
			if (sdlEvent.gaxis.value > 16000) { // If pressed more than halfway
				camera->SetView(camera->GetOrientation(),
					camera->freeCameraMovement(Vec3(0.0f, -1.0f, 0.0f)));
			}
		}
		break;
	case SDL_EVENT_MOUSE_MOTION:

		break;

	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	
		break; 

	case SDL_EVENT_MOUSE_BUTTON_UP:
	break;
	default:
		break;
    }
}

void Scene0g::RenderGUI()
{
	//im gui stuff. I havent really done anything with it 
	// but I wanna put the output of the memory monitor in it whenever I get around to it
	ImVec4 r = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 g = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 b = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);

	UIManager::StartInvisibleWindow("GunSelector", ImVec2(0, 10));
	UIManager::PushButtonStyle(b, g, r, 5.0f);

	if (ImGui::Button("Test")) {
		r = ImVec4(0.5f, 0.0f, 0.0f, 1.0f);
	}

	if (ImGui::Button("Test Two")) {
		r = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	UIManager::PopButtonStyle();
	UIManager::EndWindow();
}

void Scene0g::Update(const float deltaTime) {
	static float totalTime = 0.0f;
	totalTime += deltaTime;
	Vec3 leftPos = Vec3(-5.0f, -1.5f, -5.0f);
	Vec3 rightPos = Vec3(5.0f, -1.5f, -5.0f);
	float speed = 1.0f;
	float t = (sin(totalTime * speed) + 1.0f) / 2.0f;
	Vec3 newPos = VMath::lerp(leftPos, rightPos, t);
	ActorList.at("GameBoard")->GetComponent<TransformComponent>()->SetPosition(newPos);
	// This is all the logic for the controller input moving the controller 
	// I left it here for you to see Ill probably move it to somewhere else later
	if (gamepad && SDL_GamepadConnected(gamepad)) {
		const float deadzone = 0.2f; 

		float stickX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f;
		float stickY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f;

		Vec3 movement(0.0f,0.0f,0.0f);

		if (SDL_fabsf(stickX) > deadzone) movement.x = stickX;
		if (SDL_fabsf(stickY) > deadzone) movement.z = stickY;

		if (movement.x != 0.0f || movement.z != 0.0f) {

			camera->SetView(camera->GetOrientation(),
				camera->freeCameraMovement(movement * CameraSpeed * deltaTime));
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

			camera->GetComponent<TransformComponent>()->SetOrientation(qYaw * qPitch);
			camera->SetView(camera->GetComponent<TransformComponent>()->GetQuaternion(), camera->freeCameraMovement(movement * CameraSpeed * deltaTime));
		}
	}
	// Camera Movement with keyboard inputs
	// tbh I just wanted to see if I could apply what I did with the controller to track ball and movement 
	// very happy with how it turned out.
	// I think Ill put all this code into the camera class later but I left it here so you could see it
	const bool* keyboardState = SDL_GetKeyboardState(NULL);
	Vec3 velocity(0.0f, 0.0f, 0.0f);
	float CameraSpeed = 20.0f;
	if (keyboardState[SDL_SCANCODE_W])      velocity.z -= CameraSpeed;
	if (keyboardState[SDL_SCANCODE_S])      velocity.z += CameraSpeed;
	if (keyboardState[SDL_SCANCODE_A])      velocity.x -= CameraSpeed;
	if (keyboardState[SDL_SCANCODE_D])      velocity.x += CameraSpeed;
	if (keyboardState[SDL_SCANCODE_SPACE])  velocity.y += CameraSpeed;
	if (keyboardState[SDL_SCANCODE_LSHIFT]) velocity.y -= CameraSpeed;
	if (VMath::mag(velocity) > 0.0f) {
		velocity = VMath::normalize(velocity);
		Vec3 displacement = velocity * CameraSpeed * deltaTime;
		camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(displacement));
	}
}

void Scene0g::Render() const {
	/// Set the background color then clear the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// render skybox first
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glUseProgram(camera->GetComponent<ShaderComponent>()->GetProgram());
	glUniformMatrix4fv(camera->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(camera->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, MMath::inverse(MMath::toMatrix4(camera->GetOrientation())));
	glBindTexture(GL_TEXTURE_CUBE_MAP, camera->GetComponent<SkyBoxComponent>()->getTextureID());
	camera->GetComponent<MeshComponent>()->Render();
	glDepthMask(GL_TRUE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	if (drawInWireMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	// One shader for everything and it is attached to the board
	glUseProgram(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetProgram());
	glUniformMatrix4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());

	// render all the lights in an array
	Vec4 allAmbient[5], allDiffuse[5], allSpecular[5];
	Vec3 allPos[5];
	int i = 0;
	for (const auto& [name, light] : Resources) {
		if (i >= 5) break;
		// this math actually converts the light position from local to world space and then to view space so everything doesnt go dark.
		Vec3 localPos = light->GetComponent<TransformComponent>()->GetPosition();
		Vec3 worldPos = ActorList.at("GameBoard")->GetModelMatrix() * localPos;
		allPos[i] = camera->GetViewMatrix() * worldPos;
		allAmbient[i] = light->GetAmbient();
		allDiffuse[i] = light->GetDiffuse();
		allSpecular[i] = light->GetSpecular();
		i++;
	}
	// meh if I wanna do weird lighting effects then sure use the ambient but for now it just makes everything darker and I dont want that
	//glUniform4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("Ambient[0]"), 5, allAmbient[0]); look its here will it be used ya probably not cuz it makes no sense
	glUniform4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("Diffuse[0]"), 5, allDiffuse[0]);
	glUniform4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("Specular[0]"), 5, allSpecular[0]);
	glUniform3fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("lightPos[0]"), 5, allPos[0]);
	// this part is actually pretty cool. It renders everything in the ActorList
	for (auto const& [name, actor] : ActorList) {
		// If your a parent piece I dont wanna see you so skip you.
		if (name == "ParentPiece" || name == "ParentPieceRED") continue;
		// If your the board then bind the actors texture.... but wait not the boards texture.
		// The board is the first thing added to the list so this will never fail I put an if statement cuz it will die with literally everything else
		if (name == "GameBoard") {
			glBindTexture(GL_TEXTURE_2D, actor->GetComponent<MaterialComponent>()->getTextureID());
		}
		// Are you a black checker yes ok bind the original parent
		else if (name.find("CheckerPieceB") != std::string::npos) {
			glBindTexture(GL_TEXTURE_2D, ActorList.at("ParentPiece")->GetComponent<MaterialComponent>()->getTextureID());
		}
		// Are you a red checker yes ok bind the red parent
		else if (name.find("CheckerPieceR") != std::string::npos) {
			glBindTexture(GL_TEXTURE_2D, ActorList.at("ParentPieceRED")->GetComponent<MaterialComponent>()->getTextureID());
		}
		// everything has a model matrix right???? well I guess the lights dont buuutt they could imagine setting up directional lights now that could be funky
		// maybe when time is free Ill right a shader for directional lights
		glUniformMatrix4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());
		// if you the GameBoard then render yourself. This will always be the first thing in the list so it cant fail
		if (name == "GameBoard") {
			actor->GetComponent<MeshComponent>()->Render();
		}
		// if you is not board you is checker piece so render checker piece
		else {
			ActorList.at("ParentPiece")->GetComponent<MeshComponent>()->Render();
		}
	}
		glUseProgram(0);
}

// Well thats all of my terrible code honestly Id fail me



	
