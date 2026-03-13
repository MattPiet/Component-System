#pragma once
#include <Core/Component.h>
#include <Plane.h>
#include <Physics/TransformComponent.h>

#include "PhysicsComponent.h"
using namespace MATH;

enum class ColliderType{
    sphere,
    aabb,
    plane,
    obb
};

struct AABB
{
    Vec3 center;
    Vec3 halfExtents;
};

struct OBB
{
    Vec3 center;
    Vec3 halfExtents;
    Quaternion orientation;
};

class CollisionComponent : public Component
{
    friend class CollisionSystem;
protected:
    ColliderType collider_type_;
    std::weak_ptr<PhysicsComponent> physics_component_;
    float radius_;
    MATHEX::Plane plane_;
    AABB aabb_;
    OBB obb_;
    Vec3 hit_box_;
    bool is_collidable = physics_component_.lock()->is_collideable();
public:
    CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics, MATHEX::Plane plane);
    CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics, float radius);
    CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics, AABB aabb);
    CollisionComponent(std::weak_ptr<Component> parent_, Ref<PhysicsComponent> physics);
    ~CollisionComponent();

    void set_collideable(bool is_collideable_) { is_collidable = is_collideable_; physics_component_.lock()->set_collidable(is_collideable_); }
    bool OnCreate() override;
    void OnDestroy() override;
    void Update(const float deltaTime_) override;
    void Render() const override;
};
