#include <Graphics/LightActor.h>
#include <Physics/TransformComponent.h>
#include <Core/Debug.h>
#include <SDL3/SDL.h>
LightActor::LightActor(std::weak_ptr<Actor> parent_, Vec4 Specular_, Vec4 Diffuse_, Vec4 Ambient) : Actor(parent_), Specular(Specular_), Diffuse(Diffuse_), Ambient(Ambient)
{

}

LightActor::~LightActor()
{
	OnDestroy();
}

void LightActor::OnDestroy(){}

void LightActor::Update(const float deltaTime) {}

bool LightActor::OnCreate(){
	return true;
}

void LightActor::Render() const{}