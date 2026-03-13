#include <Physics/CollisionComponent.h>
#include <glew.h>

CollisionComponent::CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics, MATHEX::Plane plane) : Component(parent_), physics_component_(physics) ,plane_(plane)
{
}

CollisionComponent::CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics, float radius) : Component(parent_), physics_component_(physics) ,radius_(radius)
{
}

CollisionComponent::CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics, AABB aabb) : Component(parent_), physics_component_(physics) , aabb_(aabb)
{
}

CollisionComponent::CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics) : Component(parent_), physics_component_(physics)
{
    collider_type_ = ColliderType::obb;
    obb_.center = physics_component_.lock()->get_position();
    obb_.halfExtents = physics_component_.lock()->get_scale() * 0.5f; 
}

CollisionComponent::~CollisionComponent()
{
}

bool CollisionComponent::OnCreate()
{
    if (isCreated) return true;
    isCreated = true;
    return true;
}

void CollisionComponent::OnDestroy()
{
}

void CollisionComponent::Update(const float deltaTime_)
{
}

void CollisionComponent::Render() const
{
        /// im lazy and just wanna submit this shit so Ill probably do a cube mesh in a wire frame later
 
}


