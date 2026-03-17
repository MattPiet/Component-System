#include <Physics/PhysicsComponent.h>


PhysicsComponent::PhysicsComponent(std::weak_ptr<Component> parent_, Vec3 pos_, Quaternion orientation_, Vec3 scale_) :
TransformComponent(parent_, pos_, orientation_, scale_){}

PhysicsComponent::PhysicsComponent(std::weak_ptr<Component> parent_): TransformComponent(parent_)
{
}


PhysicsComponent::~PhysicsComponent()
{
}

void PhysicsComponent::update_position(float deltaTime)
{
   SetPosition(GetPosition() + velocity_ * deltaTime + 0.5f * acceleration_ * deltaTime * deltaTime);
}

void PhysicsComponent::update_velocity(float deltaTime)
{
    velocity_ += acceleration_ * deltaTime;
}

void PhysicsComponent::update_angular_velocity(float deltaTime)
{
}

void PhysicsComponent::apply_force(const Vec3& force)
{
    acceleration_ = force / mass_;
}

void PhysicsComponent::apply_torque(const float& torque)
{
}

bool PhysicsComponent::OnCreate()
{
    if (isCreated) return true;
    isCreated = true;
    return true;
}

void PhysicsComponent::OnDestroy()
{
}

void PhysicsComponent::Update(const float deltaTime_)
{
    update_position(deltaTime_);
    update_velocity(deltaTime_);
    
}

void PhysicsComponent::Render() const
{
}
