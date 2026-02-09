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



///ImGui includes
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

bool Scene0g::OnCreate() {

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

	std::unique_ptr<Actor> GameBoardActor = std::make_unique<Actor>(nullptr);
	GameBoardActor->AddComponent<MaterialComponent>(nullptr, "textures/Red&Black_Board.png");
	GameBoardActor->AddComponent<MeshComponent>(nullptr, "meshes/Plane.obj");
	GameBoardActor->AddComponent<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	GameBoardActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, -1.5f, -5.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	GameBoardActor->OnCreate();

	GameBoardActor->GetComponent<TransformComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(-1.0f, 0.0f, 0.0f)));
	GameBoardActor->GetComponent<TransformComponent>()->SetScale(Vec3(5.0f, 5.0f, 5.0f));

	ActorList.emplace("GameBoard", std::move(GameBoardActor));
	
	auto gameBoardIt = ActorList.find("GameBoard");
	Actor* gameBoardParent = (gameBoardIt != ActorList.end()) ? gameBoardIt->second.get() : nullptr;

	std::unique_ptr<Actor> ParentActor = std::make_unique<Actor>(gameBoardParent);
	ParentActor->AddComponent<MaterialComponent>(nullptr, "textures/blackCheckerPiece.png");
	ParentActor->AddComponent<MeshComponent>(nullptr, "meshes/CheckerPiece.obj");
	ParentActor->OnCreate();
	ActorList.emplace("ParentPiece", std::move(ParentActor));
	std::unique_ptr<Actor> ParentActorRED = std::make_unique<Actor>(gameBoardParent);
	ParentActorRED->AddComponent<MaterialComponent>(nullptr, "textures/redCheckerPiece.png");
	ParentActorRED->OnCreate();
	ActorList.emplace("ParentPieceRED", std::move(ParentActorRED));

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
	camera->DontTrackXYRotations();

	Vec3 startPos = Vec3(-22.25f, -22.25f, 0.0f);
	float xStep = 6.35f;
	float yStep = 6.35f;

	for (int i = 0; i < 12; i++) {
		int row = i / 4;
		int col = (i % 4) * 2 + (row % 2);

		std::string name = "CheckerPieceB" + std::to_string(i);
		if (ActorList.count(name)) {
			float newX = startPos.x + (col * xStep);
			float newY = startPos.y + (row * yStep);
			ActorList.at(name)->GetComponent<TransformComponent>()->SetPosition(Vec3(newX, newY, 0.0f));
		}
	}

	for (int j = 0; j <= 12; j++) {
	
		int index = j;
		int row = (index / 4) + 5; 
		int col = (index % 4) * 2 + (row % 2);

		std::string name = "CheckerPieceR" + std::to_string(j);

		if (ActorList.count(name)) {
			float newX = startPos.x + (col * xStep);
			float newY = startPos.y + (row * yStep);
			ActorList.at(name)->GetComponent<TransformComponent>()->SetPosition(Vec3(newX, newY, 0.0f));
		}
	}
	return true;
}

void Scene0g::OnDestroy() {


	ActorList.clear(); 
	camera.reset();
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);

}

void Scene0g::HandleEvents(const SDL_Event &sdlEvent) {
	
		camera->UpdateViewMatrix(sdlEvent);
	
	switch( sdlEvent.type ) {
    case SDL_EVENT_KEY_DOWN:
		switch (sdlEvent.key.scancode) {
			case SDL_SCANCODE_W:
				camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(0.0f, 0.0f, -1.0f)));
				break;
			case SDL_SCANCODE_S:
				camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(0.0f, 0.0f, 1.0f)));
				break;
			case SDL_SCANCODE_A:
				camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(-1.0f, 0.0f, 0.0f)));
				break;
			case SDL_SCANCODE_D:
				camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(1.0f, 0.0f, 0.0f)));
				break;
			case SDL_SCANCODE_LSHIFT:
				camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(0.0f, -1.0f, 0.0f)));
				break;
			case SDL_SCANCODE_SPACE:
				camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(0.0f, 1.0f, 0.0f)));
				break;
			case SDL_SCANCODE_L:
				drawInWireMode = !drawInWireMode;
				break;
			case SDL_SCANCODE_LEFT:
				ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->GetPosition() + Vec3(-1.0f, 0.0f, 0.0f));
				break;
			case SDL_SCANCODE_RIGHT:
				ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->GetPosition() + Vec3(1.0f, 0.0f, 0.0f));
				break;
			case SDL_SCANCODE_UP:
				ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->GetPosition() + Vec3(0.0f, 1.0f, 0.0f));
				break;
			case SDL_SCANCODE_DOWN:
				ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->SetPosition(
					ActorList["GameBoard"].get()->GetComponent<TransformComponent>()->GetPosition() + Vec3(0.0f, -1.0f, 0.0f));
				break;
		}
		break;
		
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

		case SDL_EVENT_GAMEPAD_REMOVED:
			if (gamepad && sdlEvent.gdevice.which == SDL_GetGamepadID(gamepad)) {
				SDL_CloseGamepad(gamepad);
				gamepad = nullptr;
			}
			break;
		
			break;
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
}

void Scene0g::Render() const {
	/// Set the background color then clear the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	glUseProgram(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetProgram());
	glUniformMatrix4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());
    glUniform3fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("lightPos"), 1, Vec3(-5.0f, 5.0f, -1.0f));

	for (auto const& [name, actor] : ActorList) {
		if (name == "ParentPiece" || name == "ParentPieceRED") continue;
		if (name == "GameBoard") {
			glBindTexture(GL_TEXTURE_2D, actor->GetComponent<MaterialComponent>()->getTextureID());
		}
		else if (name.find("CheckerPieceB") != std::string::npos) {
			glBindTexture(GL_TEXTURE_2D, ActorList.at("ParentPiece")->GetComponent<MaterialComponent>()->getTextureID());
		}
		else if (name.find("CheckerPieceR") != std::string::npos) {
			glBindTexture(GL_TEXTURE_2D, ActorList.at("ParentPieceRED")->GetComponent<MaterialComponent>()->getTextureID());
		}
		glUniformMatrix4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());
		if (name == "GameBoard") {
			actor->GetComponent<MeshComponent>()->Render();
		}
		else {
			ActorList.at("ParentPiece")->GetComponent<MeshComponent>()->Render();
		}
	}
		glUseProgram(0);
}



	
