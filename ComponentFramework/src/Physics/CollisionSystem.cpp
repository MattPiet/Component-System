#include <Physics/CollisionSystem.h>

bool CollisionSystem::CollisionDetection(const OBB& obbA, const OBB& obbB,  Ref<PhysicsComponent> pc1, Ref<PhysicsComponent> pc2 ,Vec3& outNormal, float& outPenetration)
{
    /*  ________                  ________
        A_1    |                 B_1    | hit box half extents
        |      |                 |      |
        |      |                 |      |
        |      |                 |      |
        |      |                 |      |
        +------|A_0              +------|B_0
       /       |                /       |
      /        |               /        |
     /         |              /         |
     --------                 --------
     A_2                      B_2
        */ // old dia from midstone same stuff ish tho

	/* in all tho it does paint a good picture of whats happening
	 * Basically you get three axis from the orientation of the object then rip throw a bunch of cross products of them
	 * And before any crosses are made we check each basic axis this is essentially meaning, is the distance between A0 and B0 smaller than their lengths if so thats a overlap we then check every possible overlap
	 * so now we do the cross of ever potential case like this in no particular order.
	 * A_0 X B_0, A_0 XB_1, A_0 X B_2, A_1 X B_1, A_1 X B_0, A_1 X B_2, A_2 X B_2, A_2 X B_0, A_2 X B_1
	 * now if ya count that its 9 cross products for now a total of 15 axis
	 * ok so now that we have the cross axis we need to project points on those axes
	 * if the axis are
	 * then we see if the two points overlap
	 * if all 9 potential collisions overlap then we have a collision
	 * test which one overlaps the least or what the best axis is then detect an overlap from that
	 * send out how much penetration there was and the collision normal
	 * we do that by adjusting the two values passed in
	 *
	 * Thats a lot but it boils down to this there are 15 axis in total first we check the 6 coming from each objects center one at a time only after all of those 6 detect overlap do we move on
	 * We then do 9 crosses each one at a time and checking each overlap before making the next cross product (it saves on performance) only if all of those 9 AND the 6 from before detect overlap do we have a collision
	 */
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

    	// as the name entails this actually checks for the overlaps between everything
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
        	// And before any crosses are made we check each basic axis this is essentially meaning is the distance between A0 and B0 smaller than there lengths if so thats a collision
        	// This comment is for here
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
        	// And before any crosses are made we check each basic axis this is essentially meaning is the distance between A0 and B0 smaller than there lengths if so thats a collision
        	// This comment is for here
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
    	// so this is the 9 crosses of each objects axis against the potentially overlapping objects axis
    	// for loop runs like this main loop A0 against all b so 3 then a1 against all b so total 6 then a2 against all b so total 9 bringing the final axis total to 15
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
	// this is one the normal and two the amount of pen between two boxes
     Vec3 normal;
     float penetration;
		// detection return false if not true
     if (!CollisionDetection(obbA, obbB, pc1, pc2, normal, penetration)) return false;
	// I like breaking it down it's easier so I setup a bunch of variables that fall out of scope but it makes it easier to read
     auto phys1 = pc1;
     auto phys2 = pc2;
     if (!phys1 || !phys2) return false;


    float m1 = phys1->get_mass();
    float m2 = phys2->get_mass();

	// inverse masses
    float invM1 = (m1 > 0.0f) ? 1.0f / m1 : 0.0f;
    float invM2 = (m2 > 0.0f) ? 1.0f / m2 : 0.0f;
    float invMassSum = invM1 + invM2;
    
    if (invMassSum <= 0.0f) return false;
    // center positions
    Vec3 posA = obbA.center;
    Vec3 posB = obbB.center;  
    // vels
    Vec3 velA = phys1->get_velocity();
    Vec3 velB = phys2->get_velocity();
    Vec3 relativeVel = velB - velA;
	// vels dotted on the Cnormal
    float separatingVel = VMath::dot(relativeVel, normal);
    
    if (invMassSum > 0.0f)
    {
        float percent = 0.2f;
        float slop = 0.02f;

        float correctionMag = std::max(penetration - slop, 0.0f) / invMassSum * percent;
        Vec3 correction = correctionMag * normal;
    	// ok this part is iffy and there is another fine tuner in the detection but this is essentially a buffer for what the pen threshold is for pos correction.
    	// You could tweak this in detection instead of setting up a barrier but if you do that then you wont be adjusting velocity every single time its needed
    	// but if you do a pos correction every time you basically tp the two objects on top of each other. soooooo if the correction mag/ the amount of correctiong needed is super super fucking small then dont correct
        if (correctionMag  > 0.08f)
        {
            Vec3 newPosA = posA - correction * invM1;
            Vec3 newPosB = posB + correction * invM2;

            phys1->set_position(newPosA);
            phys2->set_position(newPosB);
        }
    }
    // just take that seperating vel/dot along the normal from earlier and give it some impulse then multiply it by mass and add/subtract vel
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
    // I call this here everytime just incase somewhere else the object was rotated and whoever rotated it (me) didnt recalculate the center..... also once something collides the center needs to be recalculated also when something moves
	// the center needs to be recalculated so I do it here and after ever single response to make triple sure we dont get a double collision within the same tick
    actor->GetComponent<CollisionComponent>()->calculate_center(actor->GetComponent<PhysicsComponent>()->GetPosition(),actor->GetComponent<PhysicsComponent>()->GetQuaternion());
	otherActor->GetComponent<CollisionComponent>()->calculate_center(otherActor->GetComponent<PhysicsComponent>()->GetPosition(),otherActor->GetComponent<PhysicsComponent>()->GetQuaternion());

	// Sphere Sphere Collision
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
	// AABB - AABB collision or none rotating Quad Quad
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
	// OBB - OBB or Oriented Bounding Box so a rotating box
	else if (actor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::OBB &&
		otherActor->GetComponent<CollisionComponent>()->get_colliderType() == ColliderType::OBB)
	{
		OBB obb1 = actor->GetComponent<CollisionComponent>()->get_OBB();
		OBB obb2 = otherActor->GetComponent<CollisionComponent>()->get_OBB();
		// I call response because it calls detection I coded it like that for Mid-Stone and although it's a simple fix there is no reason to fix it really
		// plus I actually like it better this way ya only needa call one
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
bool CollisionSystem::RayIntersectsOBB(Ref<CollisionComponent> col, const Matrix4& modelMatrix, const Vec3& rayOrigin, const Vec3& rayDir, float& outDist) {
    
    if (!col) return false;

    // Get the hitbox mm same as in render
    Matrix4 hitboxTransform = col->CalculateModelMatrix(modelMatrix);

    // Then find out the position of it by extracting these collunms 
    Vec3 worldBoxCenter(hitboxTransform[12], hitboxTransform[13], hitboxTransform[14]);
    Vec3 halfExtents = col->get_OBB().halfExtents;

    // These represent the distance from the ray origin to the entry and exit points
    float distToEntry = -FLT_MAX;
    float distToExit = FLT_MAX;

    // Vector from the ray start to the center of the box
    Vec3 rayToBoxDelta = worldBoxCenter - rayOrigin;

    for (int i = 0; i < 3; i++) {
       // get the normalized world-space axis (X, Y, or Z)
       Vec3 rotationAxis = VMath::normalize(Vec3(hitboxTransform[i*4], hitboxTransform[i*4+1], hitboxTransform[i*4+2]));
        
       // Project the distance to the center and the ray direction onto the axis
       float centerDistOnAxis = VMath::dot(rotationAxis, rayToBoxDelta);
       float rayDirectionOnAxis = VMath::dot(rayDir, rotationAxis);

       if (std::fabs(rayDirectionOnAxis) > 0.001f) {
       	// d1 and d2 are the distances to the two parallel QUADS for this axis
       	float d1 = (centerDistOnAxis + halfExtents[i]) / rayDirectionOnAxis;
       	float d2 = (centerDistOnAxis - halfExtents[i]) / rayDirectionOnAxis;
          
          if (d1 > d2) std::swap(d1, d2);

       	// all this math is dumbed down to each axis forms different zones if you will
       	// these zones are mode of two planes that form a tunnel between them.
       	
          // this is figuring how far into that tunnel (dist between two quads) our ray has traveled 
          if (d1 > distToEntry) distToEntry = d1;
          if (d2 < distToExit) distToExit = d2;


       	/*  _________                 ________
			 A_1    |                 B_1    | hit box half extents
		     |      |                 |      |
		     |      |                 |      |
		     |      |                 |      |
		     |      |                 |      |
			 +------|A_0              +------|B_0
		    /       |                /       |
		   /        |               /        |
	      /         |              /         |
	        --------                 --------
	         A_2                      B_2
		  */

       	// same drawing again lol it's hard to paint the picture on a 2d image but essentially each axis xyz make two faces left right for x the rest are obvious
       	// those faces are located at the center of the box + the half exts (kinda but also literally that) the rest of this math basically finds the length of the ray at each plane intersection
       	// if that distance between the entry to the mouse is greater than the exit to the mouse then it never even hit the box at all
       	// if the distance of exit is 0 then the exit is behind the rays origin which is a funky problem if your inside a col box
       	// if the ray is parrllel with a plane then its impossible for it to have hit
       	
          // If entry is further than exit, the ray missed the box entirely
          if (distToEntry > distToExit) return false;
          // If the exit is behind the ray origin, the box is behind us
          if (distToExit < 0) return false;
       } else {
          // Ray is parallel; if the origin is outside the slab, it's a miss
          if (-centerDistOnAxis - halfExtents[i] > 0 || -centerDistOnAxis + halfExtents[i] < 0) return false;
       }
    }

    // Output the final distance to the closest hit point
    outDist = distToEntry;
    return true;
}

void CollisionSystem::Update(const float deltaTime)
{
}
