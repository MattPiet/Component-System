#include <Physics/PhysicsComponent.h>

PhysicsComponent::PhysicsComponent(std::weak_ptr<Component> parent_, std::shared_ptr<TransformComponent> transform, float mass) : Component(parent_), transform_(transform), mass_(mass)
{
}

PhysicsComponent::~PhysicsComponent()
{
}

void PhysicsComponent::update_position(float deltaTime)
{
   transform_.lock()->SetPosition(transform_.lock()->GetPosition() + velocity_ * deltaTime + 0.5f * acceleration_ * deltaTime * deltaTime);
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
