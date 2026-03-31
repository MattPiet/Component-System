#include <glew.h>
#include <iostream>
#include <SDL.h>
#include <Scenes/ChessScene.h>
#include <MMath.h>
#include <Core/Debug.h>
#include <Graphics/MaterialComponent.h>
#include <Graphics/MeshComponent.h>
#include <Graphics/ShaderComponent.h>
#include <Physics/TransformComponent.h>
#include <Graphics/SkyBoxComponent.h>
#include <Physics/PhysicsComponent.h>
#include <Physics/CollisionComponent.h>
#include <random>
#include <UI/UIManager.h>

//// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "Core/SceneManager.h"

ChessScene::ChessScene() :
drawInWireMode{false},
window{ nullptr }, 
context{ nullptr }
{
	Debug::Info("Created ChessScene: ", __FILE__, __LINE__);
}

ChessScene::~ChessScene() {
	Debug::Info("Deleted ChessScene: ", __FILE__, __LINE__);
}

bool ChessScene::OnCreate() {

	asset_manager_ = std::make_unique<AssetManager>("XML/assets.xml");
	asset_manager_->OnCreate();
	
	// me make camera here I wanted to have a skybox so I just made it a material comp
	camera = std::make_unique<CameraActor>(std::weak_ptr<Component>(), 45.0f, 16.0f / 9.0f, 0.5f, 400.0f);
	camera->AddComponent<PhysicsComponent>(std::weak_ptr<Component>(), Vec3(0.0f, 0.0f, -5.0f), Quaternion());
	camera->AddComponent<ShaderComponent>(asset_manager_->GetComponent<ShaderComponent>("SkyboxShader"));
	camera->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("Cube"));
	camera->AddComponent<SkyBoxComponent>(std::weak_ptr<Component>(),
		"textures/skybox/StarSkyboxPosx.png",
		"textures/skybox/StarSkyboxNegx.png",
		"textures/skybox/StarSkyboxPosy.png",
		"textures/skybox/StarSkyboxNegY.png",
		"textures/skybox/StarSkyboxPosz.png",
		"textures/skybox/StarSkyboxnegz.png");
	camera->OnCreate();

	// this is the foundation all living things come back to the game board. Literally everything except the camera is parented to it
	GameBoardActor = std::make_shared<Actor>(std::weak_ptr<Component>());
	//std::shared_ptr<ShaderComponent> shader =  std::make_shared<ShaderComponent>(nullptr, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	GameBoardActor->AddComponent<MaterialComponent>(asset_manager_->GetComponent<MaterialComponent>("ChessBoard"));
	GameBoardActor->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("Plane"));
	GameBoardActor->AddComponent<ShaderComponent>(asset_manager_->GetComponent<ShaderComponent>("PhongShader"));
	GameBoardActor->AddComponent<PhysicsComponent>(std::weak_ptr<Component>(), Vec3(0.0f, -1.5f, -5.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	GameBoardActor->OnCreate();

	GameBoardActor->GetComponent<PhysicsComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(-1.0f, 0.0f, 0.0f)));
	GameBoardActor->GetComponent<PhysicsComponent>()->SetScale(Vec3(5.0f, 5.0f, 5.0f));
	// this is me map I made one map that stores all physical actors. I did a map cuz its easier to track stuff

	// Make names for the map
	std::vector<std::string> pieceTypes = { "Rook", "Knight", "Bishop", "Queen", "King", "Bishop", "Knight", "Rook" };

	for (int i = 0; i < 32; i++)
	{
		// parent everything to the board
		std::unique_ptr<Actor> actor = std::make_unique<Actor>(GameBoardActor);
		// once we pass 16 the pieces are black
		bool isBlack = (i >= 16);
		std::string colour = isBlack ? "Black" : "White";
		std::string pieceType;
		// go throught the piecetypes index
		int localIndex = i % 16;
		if (localIndex < 8) {
			pieceType = pieceTypes[localIndex];
		}
		else {
			pieceType = "Pawn";
		}
		std::string actorName = pieceType + colour + std::to_string(i);
		actor->AddComponent<PhysicsComponent>(std::weak_ptr<Component>(), Vec3(0.0f, 0.0f, 0.0f), QMath::angleAxisRotation(90.0f, Vec3(1.0f, 0.0f, 0.0f)), Vec3(0.75f, 0.75f, 0.75f));

		// setup hit box
		Vec3 scale = actor->GetComponent<PhysicsComponent>()->GetScale();
		float hx = 5.0f * scale.x / 2.0f ;
		float hz = 5.0f * scale.z / 2.0f;
		float hy = 5.5f * scale.y; 
		Vec3 halfExts(hx, hy, hz);
		
		// setup meshes
		if (actorName.find("Rook") != std::string::npos)		  actor->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("RookMesh"));
		else if (actorName.find("Knight") != std::string::npos)   actor->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("KnightMesh"));
		else if (actorName.find("Bishop") != std::string::npos)
		{
			actor->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("BishopMesh"));
			halfExts.y += 1.0f;
		}
		else if (actorName.find("Queen") != std::string::npos)
		{
			actor->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("QueenMesh"));
			halfExts.y += 1.5f;
		}
		else if (actorName.find("King") != std::string::npos)
		{
			actor->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("KingMesh"));
			halfExts.y += 3.0f;
		}
		else if (actorName.find("Pawn") != std::string::npos)     actor->AddComponent<MeshComponent>(asset_manager_->GetComponent<MeshComponent>("PawnMesh"));

		if (actorName.find("White") != std::string::npos)
			actor->AddComponent<MaterialComponent>(asset_manager_->GetComponent<MaterialComponent>("WhiteMaterial"));
		else
			actor->AddComponent<MaterialComponent>(asset_manager_->GetComponent<MaterialComponent>("BlackMaterial"));

		/// Initialize collisions
		actor->AddComponent<CollisionComponent>(std::weak_ptr<Component>(), halfExts);
		actor->GetComponent<CollisionComponent>()->set_collider_type(ColliderType::OBB);
		if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::SPHERE) {
			collision_boxes.emplace(actorName + "Box", asset_manager_->GetComponent<MeshComponent>("Sphere"));
		}
		else if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::AABB || actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::OBB) {
			collision_boxes.emplace(actorName + "Box", asset_manager_->GetComponent<MeshComponent>("Box"));
		}
	//	actor->GetComponent<CollisionComponent>()->draw = true;
		actor->OnCreate();

		ActorList.emplace(actorName, std::move(actor));
	}
	collision_system_ = std::make_unique<CollisionSystem>();
	
	/*// print the names cuz why not
	for (const auto& pair : ActorList) {
		const std::string& name = pair.first;
		Actor* actor = pair.second.get();
	//	std::cout << "Actor Name: " << name << std::endl;
	}*/
	// turn the white knights cuz they face backwards
	ActorList.at("RookWhite0")->GetComponent<PhysicsComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(1.0f, 0.0f, 0.0f)) * QMath::angleAxisRotation(45.0f, Vec3(1.0f, 0.0f, 1.0f)));
	//ActorList.at("RookWhite0")->GetComponent<PhysicsComponent>()->set_position(ActorList.at("RookWhite0")->GetComponent<PhysicsComponent>()->GetPosition() + Vec3(0.0f, 5.0f, 0.0f));
	ActorList.at("KnightWhite1")->GetComponent<PhysicsComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(1.0f, 0.0f, 0.0f)) * QMath::angleAxisRotation(180.0f,Vec3(0.0f,1.0f,0.0f)));
	ActorList.at("KnightWhite1")->GetComponent<PhysicsComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(1.0f, 0.0f, 0.0f)) * QMath::angleAxisRotation(45.0f,Vec3(-1.0f,0.0f,0.0f)));
	ActorList.at("KnightWhite6")->GetComponent<PhysicsComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(1.0f, 0.0f, 0.0f)) * QMath::angleAxisRotation(180.0f, Vec3(0.0f, 1.0f, 0.0f)));
	Vec3 startPos = Vec3(-22.25f, -22.25f, 0.0f);
	float xStep = 6.35f;
	float yStep = 6.35f;

	for (int i = 0; i < 32; i++) {
		// same as above just finding the name instead of setting it
		bool isBlack = (i >= 16);
		int localIdx = i % 16;
		std::string colorSuffix = isBlack ? "Black" : "White";
		std::string pieceType = (localIdx < 8) ? pieceTypes[localIdx] : "Pawn";
		std::string name = pieceType + colorSuffix + std::to_string(i);

		if (ActorList.count(name)) {
			int row, col;
			// For the white pieces just do simple rows and columns
			if (!isBlack) {
				row = localIdx / 8;
				col = localIdx % 8;
			}
			// black pieces are flipped so reverse the row but columns are the same
			else {
				row = (localIdx < 8) ? 7 : 6;
				col = localIdx % 8;
			}

			float newX = startPos.x + (static_cast<float>(col) * xStep);
			float newY = startPos.y + (static_cast<float>(row) * yStep);
			// move the piece to the right spot
			ActorList.at(name)->GetComponent<PhysicsComponent>()->SetPosition(Vec3(newX, newY, 0.0f));
		}
	}
	// Diff Colours
	Vec4 diffuseColors[5] = {
		Vec4(0.4f, 0.4f, 0.6f, 1.0f), // Far Left
		Vec4(0.6f, 0.4f, 0.4f, 1.0f), // Far Right
		Vec4(0.5f, 0.5f, 0.4f, 1.0f), // Close Left
		Vec4(0.4f, 0.5f, 0.4f, 1.0f), // Close Right
		Vec4(0.5f, 0.5f, 0.5f, 1.0f)  // Dead Center
	};
	// Create lights and set their properties
	std::vector<std::string> light_names = { "FarLeftLight", "FarRightLight", "CloseLeftLight", "CloseRightLight", "CenterLight"};
	std::vector<std::string> DiffuseNames = { "FarLeftDiffuse", "FarRightDiffuse", "CloseLeftDiffuse", "CloseRightDiffuse", "CenterDiffuse" };
	for (int i = 0; i < 5; i++) {
		std::string lightName = light_names[i];
		std::unique_ptr<LightActor> Light = std::make_unique<LightActor>(GameBoardActor);
		Light->AddComponent<PhysicsComponent>(asset_manager_->GetComponent<PhysicsComponent>(light_names[i].c_str()));
		Light->OnCreate();
		// right now they all have the same spec but I might change it later so I just made it an array like the diffuse
		Light->SetSpecular(Vec4(0.5f, 0.5f, 0.5f, 1.0f));
		Light->SetDiffuse(diffuseColors[i]);
		Light->SetAmbient(Vec4(0.05f, 0.05f, 0.05f, 1.0f));
		// Put them into a resource map.... im just being fancy I couldve made an array of them but this is more fun
		Lights.emplace(lightName, std::move(Light));
	}
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
	ActorList.at("KnightWhite1")->GetComponent<TransformComponent>()->SetPosition(Vec3(ActorList.at("KnightWhite1")->GetComponent<TransformComponent>()->GetPosition().x - xStep,
		ActorList.at("KnightWhite1")->GetComponent<TransformComponent>()->GetPosition().y, 100.0f));
	
	return true;
}

void ChessScene::OnDestroy() {
	//// begone memory leaks
	ActorList.clear();
	Lights.clear();
	asset_manager_.reset();
	camera.reset();
	collision_boxes.clear();
	collision_system_.reset();
	GameBoardActor.reset();
	if (gamepad) {
		SDL_CloseGamepad(gamepad); 
		gamepad = nullptr;
	}
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

void ChessScene::HandleEvents(const SDL_Event &sdlEvent) {
		camera->UpdateViewMatrix(sdlEvent);
	switch( sdlEvent.type ) {
    case SDL_EVENT_KEY_DOWN:
		switch (sdlEvent.key.scancode) {
			case SDL_SCANCODE_L:
				drawInWireMode = !drawInWireMode;
				break;
			case SDL_SCANCODE_LEFT:
				// move selected actor
				ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->SetPosition(
					ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->GetPosition() + Vec3(-1.0f, 0.0f, 0.0f));
				break;
			case SDL_SCANCODE_RIGHT:
				ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->SetPosition(
					ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->GetPosition() + Vec3(1.0f, 0.0f, 0.0f));
				break;
			case SDL_SCANCODE_UP:
				ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->SetPosition(
					ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->GetPosition() + Vec3(0.0f, 1.0f, 0.0f));
				break;
			case SDL_SCANCODE_DOWN:
				ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->SetPosition(
					ActorList.at(selectedActorName)->GetComponent<PhysicsComponent>()->GetPosition() + Vec3(0.0f, -1.0f, 0.0f));
				break;
    default:
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
			// basic button input for controller.
	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		switch (sdlEvent.gbutton.button) {
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			drawInWireMode = !drawInWireMode;
			break;
		default:
			break;
		}
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	if (sdlEvent.button.button == SDL_BUTTON_LEFT)
	{
		float mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);
		int w, h;
		SDL_GetWindowSize(SDL_GetWindowFromID(sdlEvent.button.windowID), &w, &h);

		// NDC
		float x = (2.0f * mouseX) / static_cast<float>(w) - 1.0f;
		float y = 1.0f - (2.0f * mouseY) / static_cast<float>(h); 

		// take the inv proj to get to view space
		Matrix4 invProj = MMath::inverse(camera->GetProjectionMatrix());
		Vec4 rayView = invProj * Vec4(x, y, -1.0f, 1.0f); 
		rayView /= rayView.w; // Normalize the perspective W
		// the rest is self-explanatory by the names of the vars
		Matrix4 camWorldMatrix = MMath::inverse(camera->GetViewMatrix());

		Vec4 worldPoint = camWorldMatrix * rayView;
		Vec3 worldRayStart = camera->GetPosition();
		Vec3 worldRayDir = VMath::normalize(Vec3(worldPoint.x, worldPoint.y, worldPoint.z) - worldRayStart);

		std::string bestHitName = "";
		float minT = FLT_MAX;

		for (auto const& [name, actor] : ActorList) {
			float t;
			auto col = actor->GetComponent<CollisionComponent>();
			// Get the matrix that accounts for the parent board's movement
			Matrix4 Actor_Model_Matrix = actor->GetModelMatrix(); 

			if (collision_system_->RayIntersectsOBB(col, Actor_Model_Matrix, worldRayStart, worldRayDir, t)) {
				if (t > 0.0f && t < minT) {
					minT = t;
					bestHitName = name;
					actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f,0.0f,0.0f,1.0f));
				}
			}
		}

		if (!bestHitName.empty()) {
			selectedActorName = bestHitName;
			std::cout << "Selected: " << selectedActorName << " at Distance: " << minT << "\n";
		}
	}
	break; 
	default:
		break;
    }
}

void ChessScene::RenderGUI()
{

	ImVec4 r = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 g = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 b = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);

	UIManager::StartInvisibleWindow("DropDownMenu", ImVec2(0, 10));
	UIManager::PushButtonStyle(b, g, r, 5.0f);
	
	if (ImGui::Button("Quit")) {
		SceneManager::Quit();
	}

	
	if (showConsole) {
		UIManager::StartCommandWindow("Developer Console", ImVec2(0, 1000), ImVec2(350, 250));
		
		if (consoleMessageTimer > 0.0f) {
			
			float alpha = std::min(1.0f, consoleMessageTimer); 
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, alpha), "%s", consoleOutput.c_str());
		}

		UIManager::PushTextStyle(ImVec4(0, 1, 0, 1.0f),1.0f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.1f, 1.0f));
		if (ImGui::InputText("Command", commandInput, IM_ARRAYSIZE(commandInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
			std::string cmd(commandInput);
        
			// Reset timer and set message whenever Enter is pressed
			consoleMessageTimer = CONSOLE_MESSAGE_DURATION;

			if (cmd == "/cmds") {
				consoleOutput = "Commands: /show debug, /hide debug, /quit";
			}
			else if (cmd == "/show debug") {
				showDebugWindow = true;
				consoleOutput = "Debug Window Enabled";
			}
			else if (cmd == "/hide debug") {
				showDebugWindow = false;
				consoleOutput = "Debug Window Hidden";
			}
			else if (cmd == "/quit") {
				SDL_Event quitEvent;
				quitEvent.type = SDL_EVENT_QUIT;
				SDL_PushEvent(&quitEvent);
			}
			else {
				consoleOutput = "Unknown command: " + cmd;
			}
			strcpy_s(commandInput, "");
			ImGui::SetKeyboardFocusHere(-1); 
		}
		UIManager::PopTextStyle();
		ImGui::PopStyleColor(1);
		ImGui::End();
	}
	
	// Render the debug window if the toggle is true
	if (showDebugWindow) {
		UIManager::StartDebugWindow("Debug Window", ImVec2(1665, 10), ImVec2(250, 500));
		static bool showCollisionBoxState = false;
		if (ImGui::Checkbox("Show Collision Boxes", &showCollisionBoxState))
		{
			for (auto const& [name, actor]  : ActorList)
			{
				actor->GetComponent<CollisionComponent>()->draw = showCollisionBoxState;
			}
		}
		if (ImGui::CollapsingHeader("Light Controls")) {
			for (auto& [name, light] : Lights) {
				if (ImGui::TreeNode(name.c_str())) {
					Vec4 diffuse = light->GetDiffuse();
					if (ImGui::ColorEdit4("Diffuse", &diffuse.x)) {
						light->SetDiffuse(diffuse);
					}
					ImGui::TreePop();
				}
			}
		}
		if (ImGui::CollapsingHeader("Entity Inspector")) {
			// Create a selectable list of all actors
			if (ImGui::BeginChild("ActorList", ImVec2(0, 150), true)) {
				for (auto const& [name, actor] : ActorList) {
					if (ImGui::Selectable(name.c_str(), selectedActorName == name)) {
						selectedActorName = name;
					}
				}
			}
			ImGui::EndChild();

			// Show details for the selected actor
			if (ActorList.count(selectedActorName)) {
				auto& actor = ActorList.at(selectedActorName);
				auto phys = actor->GetComponent<PhysicsComponent>();
				actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f,0.0f,0.0f,1.0f));
				ImGui::SeparatorText(selectedActorName.c_str());
				Vec3 pos = phys->GetPosition();
				if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
					phys->SetPosition(pos);
				}
			}
		}
		UIManager::PushSliderStyle(b, g, r, 5.0f);
		if (ImGui::SliderFloat("Mouse sense", &m_sens, 0, 1))
		{
	
			camera->SetSensitivity(m_sens);
		}
		UIManager::PopSliderStyle();
		UIManager::EndWindow();
	}
	UIManager::PopButtonStyle();
	UIManager::EndWindow();
	
}

void ChessScene::Update(const float deltaTime) {
	// camera movement
	camera->CameraMovement(deltaTime, gamepad);
	if (consoleMessageTimer > 0.0f) {
		consoleMessageTimer -= deltaTime;
	}
	// board drifting left-right
	static float totalTime = 0.0f;
	totalTime += deltaTime;
	Vec3 leftPos = Vec3(-5.0f, -1.5f, -5.0f);
	Vec3 rightPos = Vec3(5.0f, -1.5f, -5.0f);
	float speed = 1.0f;
	float t = (sin(totalTime * speed) + 1.0f) / 2.0f;
	Vec3 newPos = VMath::lerp(leftPos, rightPos, t);
	GameBoardActor->GetComponent<PhysicsComponent>()->SetPosition(newPos);
	
	//// Physics
	ActorList.at("KnightWhite1")->GetComponent<PhysicsComponent>()->apply_force(Vec3(0.0f, 0.0f, -9.8f));
	for (auto const& [name, actor] : ActorList)
	{
		actor->GetComponent<PhysicsComponent>()->Update(deltaTime);
		for (const auto& [otherName, otherActor] : ActorList)
		{
			if (name == otherName) continue;
			if (name == selectedActorName || otherName == selectedActorName) continue;
			// made a function cuz it's less in the scene this function will automatically swap between each collision type no matter what it is
			collision_system_->handle_collisions(actor.get(), otherActor.get());
		}
	}

}

void ChessScene::Render() const {
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
	// render GB second
	glUseProgram(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetProgram()));
	glUniformMatrix4fv(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix")), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix")), 1, GL_FALSE, camera->GetViewMatrix());
	glBindTexture(GL_TEXTURE_2D, GameBoardActor->GetComponent<MaterialComponent>()->getTextureID());
	glUniformMatrix4fv(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"))
											 ,1, GL_FALSE, GameBoardActor->GetModelMatrix());
	GameBoardActor->GetComponent<MeshComponent>()->Render();
	// render all the lights in an array
	Vec4 allAmbient[5], allDiffuse[5], allSpecular[5];
	Vec3 allPosistions[5];
	int i = 0;
	for (const auto& [name, light] : Lights) {
		if (i >= 5) break;
		// this math actually converts the light position from local to world space and then to view space so everything doesnt go dark.
		Vec3 localPosistion = light->GetComponent<PhysicsComponent>()->GetPosition();
		Vec3 worldPosistion = GameBoardActor->GetModelMatrix() * localPosistion;
		allPosistions[i] = camera->GetViewMatrix() * worldPosistion;
		allAmbient[i] = light->GetAmbient();
		allDiffuse[i] = light->GetDiffuse();
		allSpecular[i] = light->GetSpecular();
		i++;
	}
	// meh if I wanna do weird lighting effects then sure use the ambient but for now it just makes everything darker and I dont want that
	//glUniform4fv(ActorList.at("GameBoard")->GetComponent<ShaderComponent>()->GetUniformID("Ambient[0]"), 5, allAmbient[0]); look its here will it be used ya probably not cuz it makes no sense
	glUniform4fv(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("Diffuse[0]")), 5, allDiffuse[0]);
	glUniform4fv(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("Specular[0]")), 5, allSpecular[0]);
	glUniform3fv(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("lightPos[0]")), 5, allPosistions[0]);
	// render the mesh, texture and mm of ever actor
	for (auto const& [name, actor] : ActorList) {
		glUniformMatrix4fv(static_cast<GLint>(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix")),
			1, GL_FALSE, actor->GetModelMatrix());
		// if your white become white if your black become black
		glBindTexture(GL_TEXTURE_2D, actor->GetComponent<MaterialComponent>()->getTextureID());
		actor->GetComponent<MeshComponent>()->Render();
	}
	// render collsion boxes
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(static_cast<GLint>(asset_manager_->GetComponent<ShaderComponent>("DefaultShader")->GetProgram()));
	glUniformMatrix4fv(static_cast<GLint>(asset_manager_->GetComponent<ShaderComponent>("DefaultShader")->GetUniformID("projectionMatrix")), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(static_cast<GLint>(asset_manager_->GetComponent<ShaderComponent>("DefaultShader")->GetUniformID("viewMatrix")), 1, GL_FALSE, camera->GetViewMatrix());
	for (auto const& [name, actor] : ActorList)
	{
		if (actor->GetComponent<CollisionComponent>()->draw)
		{
			Matrix4 colliderModelMatrix = actor->GetModelMatrix();
			auto colComp = actor->GetComponent<CollisionComponent>();
			if (!colComp->draw) continue; // Skip if we aren't drawing this one
			// make the mm for the hit boxes works with all types of collisions
			colliderModelMatrix = colComp->CalculateModelMatrix(colliderModelMatrix);
			glUniformMatrix4fv(static_cast<GLint>(asset_manager_->GetComponent<ShaderComponent>("DefaultShader")->GetUniformID("modelMatrix")), 1, GL_FALSE, colliderModelMatrix);
			glUniform4fv(static_cast<GLint>(asset_manager_->GetComponent<ShaderComponent>("DefaultShader")->GetUniformID("colour")), 1, colComp->get_collision_colour());
			asset_manager_->GetComponent<MeshComponent>("Cube")->Render();
		}
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUseProgram(0);
}

// Well that's all of my terrible code honestly I'd fail me



	
