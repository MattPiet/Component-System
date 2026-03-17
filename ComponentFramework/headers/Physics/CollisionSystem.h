#pragma once
#include <Core/Actor.h>
#include <Physics/CollisionComponent.h>
#include <Core/Debug.h>

#include <Sphere.h>
#include <unordered_map>
#include <string>
using namespace MATH;
using namespace MATHEX;

class CollisionSystem {
private:
    std::vector<Ref<Actor>> collidingActors;
public:
    /// This function will check the the actor being added is new and has the all proper components 
    void AddActor(Ref<Actor> actor_) {
        if(actor_->GetComponent<CollisionComponent>().get() == nullptr){
            Debug::Error("The Actor must have a CollisionComponent - ignored ", __FILE__, __LINE__);
            return;
        }

        if(actor_->GetComponent<PhysicsComponent>().get() == nullptr){
            Debug::Error("The Actor must have a PhysicsComponent - ignored ", __FILE__, __LINE__);
            return;
        }
        collidingActors.push_back(actor_);
    }
    bool CollisionDetection(const OBB& obbA, const OBB& obbB,  Ref<PhysicsComponent> pc1, Ref<PhysicsComponent> pc2 ,Vec3& outNormal, float& outPenetration);
    bool CollisionDetection(const Sphere &s1, const Sphere &s2) const; 
    bool CollisionDetection(const AABB &bb1, const AABB &bb2) const; 
    bool CollisionDetection(const Sphere s1, const Plane p1) const;
    bool RayIntersectsOBB(Ref<CollisionComponent> col, const Matrix4& modelMatrix, const Vec3& rayOrigin, const Vec3& rayDir, float& outDist);
    bool obb_response(const OBB& obbA, const OBB& obbB,  Ref<PhysicsComponent> pc1, Ref<PhysicsComponent> pc2 );
    void SphereSphereCollisionResponse(Sphere s1, Ref<PhysicsComponent> pc1, Sphere s2, Ref<PhysicsComponent> pc2);
    void AABBAABBCollisionResponse(const AABB& bb1, Ref<PhysicsComponent> pc1, const AABB& bb2, Ref<PhysicsComponent> pc2);
    void handle_collisions(Actor* actor, Actor* otherActor);
    void Update(const float deltaTime);
	
};
