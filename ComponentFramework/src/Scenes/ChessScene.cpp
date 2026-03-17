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
	// me make camera here I wanted to have a skybox so I just made it a material comp
	camera = std::make_unique<CameraActor>(std::weak_ptr<Component>(), 45.0f, 16.0f / 9.0f, 0.5f, 400.0f);
	camera->AddComponent<PhysicsComponent>(std::weak_ptr<Component>(), Vec3(0.0f, 0.0f, -5.0f), Quaternion());
	camera->AddComponent<ShaderComponent>(std::weak_ptr<Component>(), "shaders/skyBoxVert.glsl", "shaders/skyBoxFrag.glsl");
	camera->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Cube.obj");
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
	GameBoardActor->AddComponent<MaterialComponent>(std::weak_ptr<Component>(), "textures/ChessBoard.png");
	GameBoardActor->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Plane.obj");
	GameBoardActor->AddComponent<ShaderComponent>(std::weak_ptr<Component>(), "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	GameBoardActor->AddComponent<PhysicsComponent>(std::weak_ptr<Component>(), Vec3(0.0f, -1.5f, -5.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	GameBoardActor->OnCreate();

	GameBoardActor->GetComponent<PhysicsComponent>()->SetOrientation(QMath::angleAxisRotation(90.0f, Vec3(-1.0f, 0.0f, 0.0f)));
	GameBoardActor->GetComponent<PhysicsComponent>()->SetScale(Vec3(5.0f, 5.0f, 5.0f));
	// this is me map I made one map that stores all physical actors. I did a map cuz its easier to track stuff


	// Im not commenting alla this its setting up meshes and mats
	std::shared_ptr<Actor> ParentActor = std::make_shared<Actor>(std::weak_ptr<Component>());
	ParentActor->AddComponent<MaterialComponent>(std::weak_ptr<Component>(), "textures/White Chess Base Colour.png");
	ParentActor->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Rook.obj");
	ParentActor->OnCreate();
	Resources.emplace("ParentPiece", std::move(ParentActor));

	std::shared_ptr<Actor> BlackMaterial = std::make_shared<Actor>(std::weak_ptr<Component>());
	BlackMaterial->AddComponent<MaterialComponent>(std::weak_ptr<Component>(), "textures/Black Chess Base Colour.png");
	BlackMaterial->OnCreate();
	Resources.emplace("BlackMaterial", std::move(BlackMaterial));

	std::shared_ptr<Actor> Knight = std::make_shared<Actor>(std::weak_ptr<Component>());
	Knight->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Knight.obj");
	Knight->OnCreate();
	Resources.emplace("KnightMesh", std::move(Knight));

	std::shared_ptr<Actor> Bishop = std::make_shared<Actor>(std::weak_ptr<Component>());
	Bishop->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Bishop.obj");
	Bishop->OnCreate();
	Resources.emplace("BishopMesh", std::move(Bishop));

	std::shared_ptr<Actor> Queen = std::make_shared<Actor>(std::weak_ptr<Component>());
	Queen->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Queen.obj");
	Queen->OnCreate();
	Resources.emplace("QueenMesh", std::move(Queen));

	std::shared_ptr<Actor> King = std::make_shared<Actor>(std::weak_ptr<Component>());
	King->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/King.obj");
	King->OnCreate();
	Resources.emplace("KingMesh", std::move(King));

	std::shared_ptr<Actor> Pawn = std::make_shared<Actor>(std::weak_ptr<Component>());
	Pawn->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Pawn.obj");
	Pawn->OnCreate();
	Resources.emplace("PawnMesh", std::move(Pawn));

	std::shared_ptr<Actor> sphere_ = std::make_shared<Actor>(std::weak_ptr<Component>());
	sphere_->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Sphere.obj");
	sphere_->OnCreate();
	Resources.emplace("Sphere", std::move(sphere_));

	std::shared_ptr<Actor> Box = std::make_shared<Actor>(std::weak_ptr<Component>());
	Box->AddComponent<MeshComponent>(std::weak_ptr<Component>(), "meshes/Cube.obj");
	Box->OnCreate();
	Resources.emplace("Box", std::move(Box));
	
	std::shared_ptr<Actor> CollisionShader = std::make_shared<Actor>(std::weak_ptr<Component>());
	CollisionShader->AddComponent<ShaderComponent>(std::weak_ptr<Component>(), "shaders/defaultVert.glsl", "shaders/defaultFrag.glsl");
	CollisionShader->OnCreate();
	Resources.emplace("CollisionShader", std::move(CollisionShader));

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
		if (actorName.find("Rook") != std::string::npos)		  actor->AddComponent<MeshComponent>(Resources.at("ParentPiece")->GetComponent<MeshComponent>());
		else if (actorName.find("Knight") != std::string::npos)   actor->AddComponent<MeshComponent>(Resources.at("KnightMesh")->GetComponent<MeshComponent>());
		else if (actorName.find("Bishop") != std::string::npos)
		{
			actor->AddComponent<MeshComponent>(Resources.at("BishopMesh")->GetComponent<MeshComponent>());
			halfExts.y += 1.0f;
		}
		else if (actorName.find("Queen") != std::string::npos)
		{
			actor->AddComponent<MeshComponent>(Resources.at("QueenMesh")->GetComponent<MeshComponent>());
			halfExts.y += 1.5f;
		}
		else if (actorName.find("King") != std::string::npos)
		{
			actor->AddComponent<MeshComponent>(Resources.at("KingMesh")->GetComponent<MeshComponent>());
			halfExts.y += 3.0f;
		}
		else if (actorName.find("Pawn") != std::string::npos)     actor->AddComponent<MeshComponent>(Resources.at("PawnMesh")->GetComponent<MeshComponent>());

		if (actorName.find("White") != std::string::npos)
			actor->AddComponent<MaterialComponent>(Resources.at("ParentPiece")->GetComponent<MaterialComponent>());
		else
			actor->AddComponent<MaterialComponent>(Resources.at("BlackMaterial")->GetComponent<MaterialComponent>());

		/// Initialize collisions
		actor->AddComponent<CollisionComponent>(std::weak_ptr<Component>(), halfExts);
		actor->GetComponent<CollisionComponent>()->set_collider_type(ColliderType::OBB);
		if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::SPHERE) {
			collision_boxes.emplace(actorName + "Box", Resources.at("Sphere"));
		}
		else if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::AABB || actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::OBB) {
			collision_boxes.emplace(actorName + "Box", Resources.at("Box"));
		}
	//	actor->GetComponent<CollisionComponent>()->draw = true;
		actor->OnCreate();

		ActorList.emplace(actorName, std::move(actor));
	}
	//ActorList.at("KnightWhite1")->GetComponent<CollisionComponent>()->draw = true;
	//ActorList.at("RookWhite0")->GetComponent<CollisionComponent>()->draw = true;
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

			float newX = startPos.x + (col * xStep);
			float newY = startPos.y + (row * yStep);
			// move the piece to the right spot
			ActorList.at(name)->GetComponent<PhysicsComponent>()->SetPosition(Vec3(newX, newY, 0.0f));
		}
	}
	// This is the start of the light setup. Below are the positions
	Vec3 positions[5] = {
		Vec3(-20.0f,  0.0f,   20.0f), // Far Left 
		Vec3(20.0f,  0.0f,   20.0f), // Far Right 
		Vec3(-10.0f, -20.0f,  20.0f), // Close Left 
		Vec3(10.0f, -20.0f,  0.0f), // Close Right 
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
		std::unique_ptr<LightActor> Light = std::make_unique<LightActor>(GameBoardActor);

		Light->AddComponent<PhysicsComponent>(std::weak_ptr<Component>(), positions[i], Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
		Light->OnCreate();
		// right now they all have the same spec but I might change it later so I just made it an array like the diffuse
		Light->SetSpecular(Vec4(0.5f, 0.5f, 0.5f, 1.0f));
		Light->SetDiffuse(diffuseColors[i]);
		Light->SetAmbient(Vec4(0.05f, 0.05f, 0.05f, 1.0f));
		// Put them into a resource map.... im just being fancy I couldve made an array of them but this is more fun
		Lights.emplace(lightName, std::move(Light));
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
	ActorList.at("KnightWhite1")->GetComponent<TransformComponent>()->SetPosition(Vec3(ActorList.at("KnightWhite1")->GetComponent<TransformComponent>()->GetPosition().x - xStep,
		ActorList.at("KnightWhite1")->GetComponent<TransformComponent>()->GetPosition().y, 25.0f));
	
	return true;
}

void ChessScene::OnDestroy() {
	//// begone memory leaks
	ActorList.clear();
	Lights.clear();
	Resources.clear();
	camera.reset();
	collision_boxes.clear();
	collision_system_.reset();
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
				// move board
				// it dont work cuz it gets overriden in update but hey its here
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
		// the rest is self explanatory by the names of the vars
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
			Matrix4 modelMatrix = actor->GetModelMatrix(); 

			if (collision_system_->RayIntersectsOBB(col, modelMatrix, worldRayStart, worldRayDir, t)) {
				if (t > 0.0f && t < minT) {
					minT = t;
					bestHitName = name;
					actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f,0.0f,0.0f,1.0f));
				}
			}
		}

		if (!bestHitName.empty()) {
			selectedActorName = bestHitName;
			std::cout << "Selected: " << selectedActorName << " at Distance: " << minT << std::endl;
		}
	}
		break; 

	case SDL_EVENT_MOUSE_BUTTON_UP:
	break;
	default:
		break;
    }
}

void ChessScene::RenderGUI()
{
	//im gui stuff. I havent really done anything with it 
	// but I wanna put the output of the memory monitor in it whenever I get around to it
	ImVec4 r = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 g = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 b = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);

	UIManager::StartInvisibleWindow("DropDownMenu", ImVec2(0, 10));
	UIManager::PushButtonStyle(b, g, r, 5.0f);

	if (ImGui::Button("ShowDebug Window")) {
		showDebugWindow = !showDebugWindow;
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
					Vec4 diff = light->GetDiffuse();
					if (ImGui::ColorEdit4("Diffuse", &diff.x)) {
						light->SetDiffuse(diff);
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
		UIManager::EndWindow();
	}
	UIManager::PopButtonStyle();
	UIManager::EndWindow();
	
}

void ChessScene::Update(const float deltaTime) {
	static float totalTime = 0.0f;
	totalTime += deltaTime;
	Vec3 leftPos = Vec3(-5.0f, -1.5f, -5.0f);
	Vec3 rightPos = Vec3(5.0f, -1.5f, -5.0f);
	float speed = 1.0f;
	float t = (sin(totalTime * speed) + 1.0f) / 2.0f;
	Vec3 newPos = VMath::lerp(leftPos, rightPos, t);
	GameBoardActor->GetComponent<PhysicsComponent>()->SetPosition(newPos);
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
				camera->freeCameraMovement(movement * camera->GetCameraSpeed() * deltaTime));
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

			camera->GetComponent<PhysicsComponent>()->SetOrientation(qYaw * qPitch);
			camera->SetView(camera->GetComponent<PhysicsComponent>()->GetQuaternion(), camera->freeCameraMovement(movement * camera->GetCameraSpeed() * deltaTime));
		}
	}
	// Camera Movement with keyboard inputs
	// tbh I just wanted to see if I could apply what I did with the controller to track ball and movement 
	// very happy with how it turned out.
	// I think Ill put all this code into the camera class later but I left it here so you could see it
	const bool* keyboardState = SDL_GetKeyboardState(NULL);
	Vec3 velocity(0.0f, 0.0f, 0.0f);
	if (keyboardState[SDL_SCANCODE_W])      velocity.z -= camera->GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_S])      velocity.z += camera->GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_A])      velocity.x -= camera->GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_D])      velocity.x += camera->GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_SPACE])  velocity.y += camera->GetCameraSpeed();
	if (keyboardState[SDL_SCANCODE_LSHIFT]) velocity.y -= camera->GetCameraSpeed();
	if (VMath::mag(velocity) > 0.0f) {
		velocity = VMath::normalize(velocity);
		Vec3 displacement = velocity * camera->GetCameraSpeed() * deltaTime;
		camera->SetView(camera->GetOrientation(), camera->freeCameraMovement(displacement));
	}

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
	//glUseProgram(ActorList.at("GameBoard")->GetSharedComponent<ShaderComponent>()->GetProgram());
	glUseProgram(GameBoardActor->GetComponent<ShaderComponent>()->GetProgram());
	glUniformMatrix4fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());
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
	glUniform4fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("Diffuse[0]"), 5, allDiffuse[0]);
	glUniform4fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("Specular[0]"), 5, allSpecular[0]);
	glUniform3fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("lightPos[0]"), 5, allPosistions[0]);
	glBindTexture(GL_TEXTURE_2D, GameBoardActor->GetComponent<MaterialComponent>()->getTextureID());
	glUniformMatrix4fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix")
		,1, GL_FALSE, GameBoardActor->GetModelMatrix());
	GameBoardActor->GetComponent<MeshComponent>()->Render();
	glUniformMatrix4fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"),
		1, GL_FALSE, GameBoardActor->GetModelMatrix());
	for (auto const& [name, actor] : ActorList) {
		glUniformMatrix4fv(GameBoardActor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"),
			1, GL_FALSE, actor->GetModelMatrix());
		// if your white become white if your black become black
		glBindTexture(GL_TEXTURE_2D, actor->GetComponent<MaterialComponent>()->getTextureID());
		actor->GetComponent<MeshComponent>()->Render();
	}
	// render collsion boxes
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(Resources.at("CollisionShader")->GetComponent<ShaderComponent>()->GetProgram());
	glUniformMatrix4fv(Resources.at("CollisionShader")->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(Resources.at("CollisionShader")->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());
	for (auto const& [name, actor] : ActorList)
	{
		if (actor->GetComponent<CollisionComponent>()->draw)
		for (const auto& [boxName, boxActor] : collision_boxes)
		{
			Matrix4 colliderModelMatrix = actor->GetModelMatrix();
			auto colComp = actor->GetComponent<CollisionComponent>();
			if (!colComp->draw) continue; // Skip if we aren't drawing this one
			// make the mm for the hit boxes works with all types of collisions
			colliderModelMatrix = colComp->CalculateModelMatrix(colliderModelMatrix);
			glUniformMatrix4fv(Resources.at("CollisionShader")->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"), 1, GL_FALSE, colliderModelMatrix);
			glUniform4fv(Resources.at("CollisionShader")->GetComponent<ShaderComponent>()->GetUniformID("colour"), 1, colComp->get_collision_colour());
			boxActor->GetComponent<MeshComponent>()->Render();
			
		}
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUseProgram(0);
}

// Well that's all of my terrible code honestly I'd fail me



	
