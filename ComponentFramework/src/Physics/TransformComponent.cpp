#include <Physics/TransformComponent.h>
#include <Utils/MemoryMonitor.h>
TransformComponent::TransformComponent(Component* parent_) : Component(parent_){}

Matrix4 TransformComponent::GetTransformMatrix() const
{
	return MMath::translate(pos)
		* MMath::scale(scale)
		* MMath::toMatrix4(orientation);
	 
}

TransformComponent::TransformComponent(Component* parent_, Vec3 pos_, Quaternion orientation_, Vec3 scale_) : Component(parent_), 
pos(pos_), 
orientation(orientation_), 
scale(scale_){}

TransformComponent::~TransformComponent()
{
}

bool TransformComponent::OnCreate()
{
	return true;
}

void TransformComponent::OnDestroy(){}

void TransformComponent::Update(const float deltaTime_){}

void TransformComponent::Render() const{}
