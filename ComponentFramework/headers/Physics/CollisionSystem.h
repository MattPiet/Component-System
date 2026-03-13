#pragma once
#include <Core/Actor.h>
#include <Physics/CollisionComponent.h>

class CollisionSystem
{
    public:
    static bool sphere_sphere_detection(const CollisionComponent& collider1, const CollisionComponent& collider2);
    static bool sphere_plane_detection(const CollisionComponent& collider1, const CollisionComponent& collider2);
    static bool sphere_aabb_detection(const CollisionComponent& collider1, const CollisionComponent& collider2);
    static bool obb_detection(const Ref<CollisionComponent>& collider1, const Ref<CollisionComponent>& collider2, Vec3& outNormal, float& outPenetration);
    static void obb_response(const Ref<CollisionComponent>& collider1, const Ref<CollisionComponent>& collider2);
    
};