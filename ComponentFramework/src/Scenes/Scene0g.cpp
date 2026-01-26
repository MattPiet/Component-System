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

	camera = M_new CameraActor(nullptr, 45.0f, 16.0f / 9.0f, 0.5f, 100.0f);
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

	actor = M_new Actor(nullptr);
	actor->AddComponent<MaterialComponent>(actor, "textures/mario_main.png");
	actor->AddComponent<MeshComponent>(actor, "meshes/Mario.obj");
	actor->AddComponent<ShaderComponent>(actor, "shaders/texturePhongVert.glsl", "shaders/texturePhongFrag.glsl");
	actor->AddComponent<TransformComponent>(actor, Vec3(0.0f, -1.0f, -5.0f), Quaternion(), Vec3(1.0f, 1.0f, 1.0f));
	actor->OnCreate();
	actor->GetComponent<TransformComponent>()->SetOrientation(QMath::angleAxisRotation(180.0f, Vec3(0.0f, 1.0f, 0.0f)));

	actor->ListComponents();

	Vec3 offset = Vec3(0.0f, 0.0f, 15.0f);
	Vec3 rotatedOffset = QMath::rotate(offset, camera->GetOrientation());
	Vec3 cameraPos = Vec3(0.0f, 0.0f, 0.0f) + rotatedOffset;
	camera->SetView(camera->GetOrientation(), cameraPos);
	camera->DontTrackXYRotations();
	return true;
}

void Scene0g::OnDestroy() {
	if(actor){
		actor->OnDestroy();
		delete actor;
		actor = nullptr;
	}
	if(camera){
		camera->OnDestroy();
		delete camera;
		camera = nullptr;
	}
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
	glUniformMatrix4fv(camera->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, MMath::toMatrix4(camera->GetOrientation()));
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
	glUseProgram(actor->GetComponent<ShaderComponent>()->GetProgram());
	glBindTexture(GL_TEXTURE_2D, actor->GetComponent<MaterialComponent>()->getTextureID());

	glUniformMatrix4fv(actor->GetComponent<ShaderComponent>()->GetUniformID("projectionMatrix"), 1, GL_FALSE, camera->GetProjectionMatrix());
	glUniformMatrix4fv(actor->GetComponent<ShaderComponent>()->GetUniformID("viewMatrix"), 1, GL_FALSE, camera->GetViewMatrix());
	glUniformMatrix4fv(actor->GetComponent<ShaderComponent>()->GetUniformID("modelMatrix"), 1, GL_FALSE, actor->GetComponent<TransformComponent>()->GetTransformMatrix());
	glUniform3fv(actor->GetComponent<ShaderComponent>()->GetUniformID("lightPos"), 1, Vec3(-5.0f, 5.0f, -1.0f));
	actor->GetComponent<MeshComponent>()->Render();
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}



	
