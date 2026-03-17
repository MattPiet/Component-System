#include <Physics/CollisionSystem.h>

bool CollisionSystem::CollisionDetection(const OBB& obbA, const OBB& obbB,  Ref<PhysicsComponent> pc1, Ref<PhysicsComponent> pc2 ,Vec3& outNormal, float& outPenetration)
{
    /*  ________                  ________
           A_2    |                 B_2    | hit box half extents
        |      |                 |      |
        |      |                 |      |
        |      |                 |      |
        |      |                 |      |
           +------|A_0              +------|B_0
       /       |                /       |
      /        |               /        |
     /         |              /         |
     --------                 --------
     A_3                      B_3
        */ // old dia from midstone same stuff ish tho
    if (pc1 && pc2)
    {
        Vec3 centerA = obbA.center;
        Vec3 centerB = obbB.center;

        Vec3 halfExtA = obbA.halfExtents;
        Vec3 halfExtB = obbB.halfExtents;

        Quaternion qA = pc1->GetQuaternion();
        Quaternion qB = pc2->GetQuaternion();

        Vec3 axisA[3] = {
            QMath::rotate(Vec3(1, 0, 0), qA),
            QMath::rotate(Vec3(0, 1, 0), qA),
            QMath::rotate(Vec3(0, 0, 1), qA)
        };
        Vec3 axisB[3] = {
            QMath::rotate(Vec3(1, 0, 0), qB),
            QMath::rotate(Vec3(0, 1, 0), qB),
            QMath::rotate(Vec3(0, 0, 1), qB)
        };

        for (int i = 0; i < 3; i++)
        {
            axisA[i] = VMath::normalize(axisA[i]);
            axisB[i] = VMath::normalize(axisB[i]);
        }

        float R[3][3], AbsR[3][3];
        const float EPS = 1e-5f;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
            {
                R[i][j] = VMath::dot(axisA[i], axisB[j]);
                AbsR[i][j] = std::fabs(R[i][j]) + EPS;
            }

        Vec3 tWorld = centerB - centerA;
        Vec3 t(
            VMath::dot(tWorld, axisA[0]),
            VMath::dot(tWorld, axisA[1]),
            VMath::dot(tWorld, axisA[2])
        );

        float minPen = FLT_MAX;
        Vec3 bestAxis = Vec3(0, 0, 0);

        auto CheckAxis = [&](const Vec3& axis, float dist, float ra, float rb)
        {
            float overlap = ra + rb - std::fabs(dist);
            if (overlap < 0.0f)
                return false;

            if (overlap < minPen)
            {
                minPen = overlap;

                // normal must point from A → B
                bestAxis = (dist < 0.0f ? -axis : axis);
            }
            return true;
        };

        float ra, rb, dist;

        // Axes A0, A1, A2
        for (int i = 0; i < 3; i++)
        {
            ra = halfExtA[i];
            rb = halfExtB.x * AbsR[i][0] +
                halfExtB.y * AbsR[i][1] +
                halfExtB.z * AbsR[i][2];

            dist = t[i];
            if (!CheckAxis(axisA[i], dist, ra, rb))
                return false;
        }

        // Axes B0, B1, B2
        for (int j = 0; j < 3; j++)
        {
            ra = halfExtA.x * AbsR[0][j] +
                halfExtA.y * AbsR[1][j] +
                halfExtA.z * AbsR[2][j];

            rb = halfExtB[j];

            dist = t.x * R[0][j] +
                t.y * R[1][j] +
                t.z * R[2][j];

            if (!CheckAxis(axisB[j], dist, ra, rb))
                return false;
        }

        // Cross products Ai × Bj
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
            {
                Vec3 axis = VMath::cross(axisA[i], axisB[j]);
                if (VMath::mag(axis) < 1e-5f) continue; // parallel

                axis = VMath::normalize(axis);

                // Project radii
                ra =
                    halfExtA.x * std::fabs(VMath::dot(axisA[0], axis)) +
                    halfExtA.y * std::fabs(VMath::dot(axisA[1], axis)) +
                    halfExtA.z * std::fabs(VMath::dot(axisA[2], axis));

                rb =
                    halfExtB.x * std::fabs(VMath::dot(axisB[0], axis)) +
                    halfExtB.y * std::fabs(VMath::dot(axisB[1], axis)) +
                    halfExtB.z * std::fabs(VMath::dot(axisB[2], axis));

                dist = VMath::dot(tWorld, axis);

                if (!CheckAxis(axis, dist, ra, rb))
                    return false;
            }

        // === If we got here, collision exists ===
        outNormal = bestAxis;
        outPenetration = minPen;

        return true;
    }
    return false;
}

 bool CollisionSystem::obb_response(const OBB& obbA, const OBB& obbB,  Ref<PhysicsComponent> pc1, Ref<PhysicsComponent> pc2 )
{
     Vec3 normal;
     float penetration;
  
     if (!CollisionDetection(obbA, obbB, pc1, pc2, normal, penetration)) return false;
  
     auto phys1 = pc1;
     auto phys2 = pc2;
     if (!phys1 || !phys2) return false;


    float m1 = phys1->get_mass();
    float m2 = phys2->get_mass();


    float invM1 = (m1 > 0.0f) ? 1.0f / m1 : 0.0f;
    float invM2 = (m2 > 0.0f) ? 1.0f / m2 : 0.0f;
    float invMassSum = invM1 + invM2;
    
    if (invMassSum <= 0.0f) return false;
    
    Vec3 posA = obbA.center;
    Vec3 posB = obbB.center;  
    
    Vec3 velA = phys1->get_velocity();
    Vec3 velB = phys2->get_velocity();
    Vec3 relativeVel = velB - velA;
    float separatingVel = VMath::dot(relativeVel, normal);
    
    if (invMassSum > 0.0f)
    {
        float percent = 0.2f;
        float slop = 0.02f;

        float correctionMag = std::max(penetration - slop, 0.0f) / invMassSum * percent;
        Vec3 correction = correctionMag * normal;
        if (correctionMag  > 0.05f)
        {
            Vec3 newPosA = posA - correction * invM1;
            Vec3 newPosB = posB + correction * invM2;

            phys1->set_position(newPosA);
            phys2->set_position(newPosB);
        }
    }
    
    if (separatingVel > 0.0f) return false;
        float restitution = (separatingVel < -1.0f) ? 0.1f : 0.0f;
        float j = -(1.0f + restitution) * separatingVel;
        j = std::max(j, 0.0f);
        j /= invMassSum;

        Vec3 impulseVec = normal * j;

        phys1->set_velocity(velA - impulseVec * invM1);
        phys2->set_velocity(velB + impulseVec * invM2);
    return true;
 }

bool CollisionSystem::CollisionDetection(const Sphere& s1, const Sphere& s2) const
{
    const float distance_x = s1.center.x - s2.center.x;
    const float distance_y = s1.center.y - s2.center.y;
    const float distance_z = s1.center.z - s2.center.z;
    
    const float distanceSquared = distance_x * distance_x + distance_y * distance_y + distance_z * distance_z;
    const float radiusSum = s1.r + s2.r;
    return distanceSquared <= (radiusSum * radiusSum);
}

bool CollisionSystem::CollisionDetection(const AABB& bb1, const AABB& bb2) const
{
   
    float distance_x = std::fabs(bb1.center.x - bb2.center.x);
    float distance_y = std::fabs(bb1.center.y - bb2.center.y);
    float distance_z = std::fabs(bb1.center.z - bb2.center.z);

 
    return (distance_x <= (bb1.halfExtents.x + bb2.halfExtents.x)) &&
           (distance_y <= (bb1.halfExtents.y + bb2.halfExtents.y)) &&
           (distance_z <= (bb1.halfExtents.z + bb2.halfExtents.z));
}

bool CollisionSystem::CollisionDetection(const Sphere s1, const Plane p1) const
{
    return false;
}

void CollisionSystem::SphereSphereCollisionResponse(Sphere s1, Ref<PhysicsComponent> pc1, Sphere s2, Ref<PhysicsComponent> pc2) {
    float e = 1.0f; /// coefficient of restitution
    Vec3 L = s1.center - s2.center;
    Vec3 n = VMath::normalize(L);
    Vec3 v1 = pc1->get_velocity();
    Vec3 v2 = pc2->get_velocity();
    float m1 = pc1->get_mass();
    float m2 = pc2->get_mass();

    float v1p = VMath::dot(v1, n);
    float v2p = VMath::dot(v2, n);

    if(v1p - v2p > 0.0f) { /// The colliding objects are not yet free from one and another, come back next cycle
        return;
    }
    float v1p_new = (((m1 - e * m2) * v1p) + ((1.0f + e) * m2 * v2p)) / (m1 + m2);
    float v2p_new = (((m2 - e * m1) * v2p) + ((1.0f + e) * m1 * v1p)) / (m1 + m2);

    pc1->set_velocity( v1 + (v1p_new - v1p) * n);
    pc2->set_velocity(v2 + (v2p_new - v2p) * n);
}

void CollisionSystem::AABBAABBCollisionResponse(const AABB& bb1, Ref<PhysicsComponent> pc1, const AABB& bb2, Ref<PhysicsComponent> pc2)
{
    Vec3 relativeVel = pc2->get_velocity() - pc1->get_velocity();
    Vec3 normal = VMath::normalize(bb2.center - bb1.center); 
    
    float velAlongNormal = VMath::dot(relativeVel, normal);
    if (velAlongNormal > 0) return;

    float e = 1.0f; 
    float impulseJ = -(1.0f + e) * velAlongNormal;
    impulseJ /= (1.0f /  pc1->get_mass() + 1.0f / pc2->get_mass());

    Vec3 impulse = impulseJ * normal;
    pc1->set_velocity(pc1->get_velocity() - (1.0f /  pc1->get_mass()) * impulse);
    pc2->set_velocity(pc2->get_velocity() + (1.0f / pc2->get_mass()) * impulse);
}

void CollisionSystem::handle_collisions(Actor* actor, Actor* otherActor)
{
    
    actor->GetComponent<CollisionComponent>()->calculate_center(actor->GetComponent<PhysicsComponent>()->GetPosition(),actor->GetComponent<PhysicsComponent>()->GetQuaternion());
	otherActor->GetComponent<CollisionComponent>()->calculate_center(otherActor->GetComponent<PhysicsComponent>()->GetPosition(),otherActor->GetComponent<PhysicsComponent>()->GetQuaternion());

	if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::SPHERE &&
		otherActor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::SPHERE)
	{
		Sphere s1 = actor->GetComponent<CollisionComponent>()->get_Sphere();
		Sphere s2 = otherActor->GetComponent<CollisionComponent>()->get_Sphere();
	
		if (CollisionDetection(s1,s2))
		{
			SphereSphereCollisionResponse(s1, actor->GetComponent<PhysicsComponent>(),
				s2, otherActor->GetComponent<PhysicsComponent>());
			actor->GetComponent<CollisionComponent>()->calculate_center(actor->GetComponent<PhysicsComponent>()->GetPosition());
			otherActor->GetComponent<CollisionComponent>()->calculate_center(otherActor->GetComponent<PhysicsComponent>()->GetPosition());
			actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		otherActor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		else
		{
			actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
			otherActor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
	}
	else if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::AABB &&
		otherActor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::AABB)
	{
		AABB b1 = actor->GetComponent<CollisionComponent>()->get_AABB();
		AABB b2 = otherActor->GetComponent<CollisionComponent>()->get_AABB();

		if (CollisionDetection(b1, b2))
		{
			AABBAABBCollisionResponse(b1, actor->GetComponent<PhysicsComponent>(),
				b2, otherActor->GetComponent<PhysicsComponent>());
			actor->GetComponent<CollisionComponent>()->calculate_center(actor->GetComponent<PhysicsComponent>()->GetPosition());
			otherActor->GetComponent<CollisionComponent>()->calculate_center(otherActor->GetComponent<PhysicsComponent>()->GetPosition());
			actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
			otherActor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		else
		{
			actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
			otherActor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
	}
	else if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::OBB &&
		otherActor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::OBB)
	{
		OBB obb1 = actor->GetComponent<CollisionComponent>()->get_OBB();
		OBB obb2 = otherActor->GetComponent<CollisionComponent>()->get_OBB();
		if (obb_response(obb1, obb2, actor->GetComponent<PhysicsComponent>(), otherActor->GetComponent<PhysicsComponent>()))
		{
		    actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		    otherActor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		    actor->GetComponent<CollisionComponent>()->calculate_center(actor->GetComponent<PhysicsComponent>()->GetPosition(),actor->GetComponent<PhysicsComponent>()->GetQuaternion());
		    otherActor->GetComponent<CollisionComponent>()->calculate_center(otherActor->GetComponent<PhysicsComponent>()->GetPosition(),otherActor->GetComponent<PhysicsComponent>()->GetQuaternion());
		}
	    else
	    {
	    	actor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	    	otherActor->GetComponent<CollisionComponent>()->set_collision_colour(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	    }
	}
	
}


void CollisionSystem::Update(const float deltaTime)
{
}
