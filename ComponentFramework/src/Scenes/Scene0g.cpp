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
	GameBoardActor->AddComponent<MaterialComponent>(nullptr, "textures/ChessBoard.png");
	GameBoardActor->AddComponent<MeshComponent>(nullptr, "meshes/Plane.obj");
	GameBoardActor->AddComponent<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	GameBoardActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, -1.5f, -5.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	GameBoardActor->OnCreate();

	GameBoardActor->GetComponent<TransformComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(-1.0f, 0.0f, 0.0f)));
	GameBoardActor->GetComponent<TransformComponent>()->SetScale(Vec3(5.0f, 5.0f, 5.0f));

	ActorList.emplace("GameBoard", std::move(GameBoardActor));
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(-20.5f, 20.5f); // Bounds of your 5x5 board

	std::vector<Vec3> spawnedPositions;
	float minDistance = 2.0f; // The "radius" of a Mario to prevent overlap
	auto gameBoardIt = ActorList.find("GameBoard");
	Actor* gameBoardParent = (gameBoardIt != ActorList.end()) ? gameBoardIt->second.get() : nullptr;


	for (int i = 0; i < 200; i++) {
		std::unique_ptr<Actor> actor = std::make_unique<Actor>(gameBoardParent);
		actor->AddComponent<MaterialComponent>(nullptr, "textures/mario_main.png");
		actor->AddComponent<MeshComponent>(nullptr, "meshes/Mario.obj");
		actor->AddComponent<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
		actor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f,0.0f,0.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
		actor->OnCreate();
		Vec3 randomPos;
		bool validPos = false;
		int attempts = 0;

		while (!validPos && attempts < 100) {
			randomPos = Vec3(dist(gen), dist(gen),0.0f);
			validPos = true;
			attempts++;

			for (const Vec3& pos : spawnedPositions) {
				if (VMath::distance(randomPos, pos) < minDistance) {
					validPos = false;
					break;
				}
			}
		}
		spawnedPositions.push_back(randomPos);
		actor->GetComponent<TransformComponent>()->SetPosition(Vec3(randomPos.x, randomPos.y,1.5f));
		actor->GetComponent<TransformComponent>()->SetOrientation(
			QMath::angleAxisRotation(90.0f, Vec3(1.0f, 0.0f, 0.0f)) * QMath::angleAxisRotation(180.0f, Vec3(0.0f, 1.0f, 0.0f))
		);
		std::string actorName = "Mario" + std::to_string(i);
		if (actorName == "Mario" + std::to_string(50)) {
			actor->GetComponent<MaterialComponent>()->LoadImage("textures/Lmario_main.png");
			actor->GetComponent<TransformComponent>()->SetScale(Vec3(0.5f, 0.5f, 0.5f));
			actor->GetComponent<TransformComponent>()->SetPosition
			(Vec3(actor->GetComponent<TransformComponent>()->GetPosition().x, 
				actor->GetComponent<TransformComponent>()->GetPosition().y, 0.555f));
		}
		ActorList.emplace(actorName, std::move(actor));
	}


	int count;
	SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
	if (gamepads) {
		if (count > 0) {
			gamepad = SDL_OpenGamepad(gamepads[0]);
			Debug::Info("Gamepad found on startup.", __FILE__, __LINE__);
		}
		SDL_free(gamepads); // Important: SDL_GetGamepads returns an allocated array
	}
	Vec3 offset = Vec3(0.0f, 0.0f, 15.0f);
	Vec3 rotatedOffset = QMath::rotate(offset, camera->GetOrientation());
	Vec3 cameraPos = Vec3(0.0f, 0.0f, 0.0f) + rotatedOffset;
	camera->SetView(camera->GetOrientation(), cameraPos);
	camera->DontTrackXYRotations();
	return true;
}

void Scene0g::OnDestroy() {


	ActorList.clear(); // Empty the map container
	camera.reset();
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);

}

void Scene0g::HandleEvents(const SDL_Event &sdlEvent) {
	
		camera->UpdateViewMatrix(sdlEvent);
	
	switch( sdlEvent.type ) {
    case SDL_EVENT_KEY_DOWN:
		switch (sdlEvent.key.scancode) {
			case SDL_SCANCODE_W:
				//drawInWireMode = !drawInWireMode;
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
					SDL_free(gamepads); // Important: SDL_GetGamepads returns an allocated array
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
		
	
		// You can still use events for "one-tap" actions like jumping or menus
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

	//case SDL_EVENT_MOUSE_WHEEL:
	//		if(sdlEvent.wheel.y > 0) {
	//			camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(0.0f, 0.0f, -1.0f)));
	//		}
	//		else if(sdlEvent.wheel.y < 0) {
	//				camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(Vec3(0.0f, 0.0f, 1.0f)));
	//		}
	//	break;
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
	if (gamepad && SDL_GamepadConnected(gamepad)) {
		const float deadzone = 0.2f; // 20% deadzone
	

		// SDL_GetGamepadAxis returns -32768 to 32767
		float stickX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f;
		float stickY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f;

		Vec3 movement(0.0f,0.0f,0.0f);

		if (SDL_fabsf(stickX) > deadzone) movement.x = stickX;
		if (SDL_fabsf(stickY) > deadzone) movement.z = stickY;

		if (movement.x != 0.0f || movement.z != 0.0f) {
			// Apply movement scaled by speed and delta time
			camera->SetView(camera->GetOrientation(),
				camera->freeCameraMovement(movement * CameraSpeed * deltaTime));
		}

		float rightStickX = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX) / 32767.0f;
		float rightStickY = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) / 32767.0f;

		if (SDL_fabsf(rightStickX) > deadzone || SDL_fabsf(rightStickY) > deadzone) {
			// 1. Update your stored angles (Add these floats to your class)
			 m_Yaw += -rightStickX * m_Sensitivity * deltaTime;
			 m_Pitch += -rightStickY * m_Sensitivity * deltaTime;

			// 2. Keep the camera from flipping over
			if (m_Pitch > 89.0f)  m_Pitch = 89.0f;
			if (m_Pitch < -89.0f) m_Pitch = -89.0f;

			// 3. Create two clean Quaternions from the absolute angles
			// Note: We use the world axes (0,1,0) and (1,0,0)
			Quaternion qYaw = QMath::angleAxisRotation(m_Yaw, Vec3(0.0f, 1.0f, 0.0f));
			Quaternion qPitch = QMath::angleAxisRotation(m_Pitch, Vec3(1.0f, 0.0f, 0.0f));

			// 4. Combine them (Order matters: Yaw * Pitch is standard for FPS)
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
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
	for (const auto& pair : ActorList) {
		Actor* actor = pair.second.get();
		glUseProgram(actor->GetComponent<ShaderComponent>()->GetProgram());
		glBindTexture(GL_TEXTURE_2D, actor->GetComponent<MaterialComponent>()->getTextureID());

		glUniformMatrix4fv(actor->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
		glUniformMatrix4fv(actor->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());
		glUniformMatrix4fv(actor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetModelMatrix());
		glUniform3fv(actor->GetComponent<ShaderComponent>()->GetUniformID("lightPos"), 1, Vec3(-5.0f, 5.0f, -1.0f));
		actor->GetComponent<MeshComponent>()->Render();
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
}



	
