#pragma once
#include <Core/Component.h>
#include <Plane.h>
#include <Sphere.h>
#include <Physics/PhysicsComponent.h>
#include <Core/Debug.h>
using namespace MATHEX;

enum class ColliderType {
    SPHERE,
    AABB,
    PLANE,
    OBB,
	
};

/// For an Axis Aligned Bounding Box, their are many ways you could define the box.
/// My favorite way, the easiest to understand way is pick the center location of 
/// the box, then specify the radius from that center in 
/// the x, y,and z dimensions. Umer calls the radius the halfExtent
struct AABB {
    Vec3 center;
    Vec3 halfExtents;
};

struct OBB {
    Vec3 center;
    Vec3 halfExtents;
};

class CollisionComponent: public Component {
    friend class CollisionSystem;
    CollisionComponent(const CollisionComponent&) = delete;
    CollisionComponent(CollisionComponent&&) = delete;
    CollisionComponent& operator = (const CollisionComponent&) = delete;
    CollisionComponent& operator = (CollisionComponent&&) = delete;
protected:
    ColliderType colliderType;  
    float radius; /// Sphere collision
    Vec3 halfExtents; /// AABB
    Plane plane; /// Plane
    Sphere sphere;
    Vec4 collision_colour_ = Vec4(0.0f,1.0f,0.0f,1.0f);
    AABB aabb;
    OBB obb;
public:
    bool draw = false;
    CollisionComponent(std::weak_ptr<Component> parent_, float radius_);
    CollisionComponent(std::weak_ptr<Component> parent_, Vec3 halfExtents_);
    CollisionComponent(std::weak_ptr<Component> parent_, Plane plane_);
    void calculate_center(const Vec3& position,  const Quaternion& Ori = Quaternion());
    void set_Sphere(const Vec3& center, const float& r) { sphere.center = center; sphere.r = r; }
    void set_collision_colour(Vec4 collision_colour) { collision_colour_ = collision_colour; }
    void set_collider_type(ColliderType type) { colliderType = type; }

    Sphere get_Sphere() const
    {
        if (colliderType == ColliderType::SPHERE) return sphere;
        Debug::Error("CollisionComponent::get_Sphere called with invalid type", __FILE__, __LINE__) ;
        return Sphere();
    }
    AABB get_AABB() const
    {
        if (colliderType == ColliderType::AABB) return aabb;
        Debug::Error("CollisionComponent::get_AABB called with invalid type", __FILE__, __LINE__) ;
        return AABB();
    }

    OBB get_OBB() const  
    {
        if (colliderType == ColliderType::OBB) return obb;
        Debug::Error("CollisionComponent::get_OOBB called with invalid type", __FILE__, __LINE__) ;
        return OBB();
    }
    MATH::Matrix4 CalculateModelMatrix(MATH::Matrix4 BaseMM) const;
    ColliderType get_colliderType() const { return colliderType; }
    Vec4 get_collision_colour() const { return collision_colour_; }
    bool OnCreate(){return true;}
    void OnDestroy(){}
    void Update(const float deltaTime_){}
    void Render()const;
};

