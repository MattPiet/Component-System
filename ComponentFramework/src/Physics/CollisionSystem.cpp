#include <Physics/CollisionSystem.h>

bool CollisionSystem::sphere_sphere_detection(const CollisionComponent& collider1, const CollisionComponent& collider2){
    
    return false;
}

bool CollisionSystem::sphere_plane_detection(const CollisionComponent& collider1, const CollisionComponent& collider2)
{
    return false;
}

bool CollisionSystem::sphere_aabb_detection(const CollisionComponent& collider1, const CollisionComponent& collider2)
{
    return false;
}

bool CollisionSystem::obb_detection(const Ref<CollisionComponent>& collider1, const Ref<CollisionComponent>& collider2, Vec3& outNormal, float& outPenetration)
{
    auto physA = collider1->physics_component_.lock();
    auto physB = collider2->physics_component_.lock();
    if (physA && physB)
    {
        Vec3 centerA = physA->get_position();
        Vec3 centerB = physB->get_position();

        Vec3 halfExtA = collider1->obb_.halfExtents;
        Vec3 halfExtB = collider2->obb_.halfExtents;

        Quaternion qA = physA->get_orientation();
        Quaternion qB = physB->get_orientation();

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

 void CollisionSystem::obb_response(const Ref<CollisionComponent>& collider1, const Ref<CollisionComponent>& collider2)
{
     Vec3 normal;
     float penetration;
  
     if (!obb_detection(collider1, collider2, normal, penetration)) return;
  
     auto phys1 = collider1->physics_component_.lock();
     auto phys2 = collider2->physics_component_.lock();
     if (!phys1 || !phys2) return;

     Vec3 posA = phys1->get_position();
     Vec3 posB = phys2->get_position();  
     float overlap = penetration;
  
     // --- Unified Position Correction ---
     // Rule: true = Moveable, false = Static Anchor

     if (!collider1->is_collidable && collider2->is_collidable) // 1 is Board (Static), 2 is Piece (Dynamic)
     {
         // Normal points 1 -> 2, move 2 along the normal
         posB += normal * overlap;
         phys2->set_position(posB);
     }
     else if (collider1->is_collidable && !collider2->is_collidable) // 1 is Piece (Dynamic), 2 is Board (Static)
     {
         // Move 1 against the normal to push it out of the board
         posA -= normal * overlap;
         phys1->set_position(posA);
     }
     else if (collider1->is_collidable && collider2->is_collidable) // Both are Dynamic Pieces
     {
         posA -= normal * (overlap * 0.5f);
         posB += normal * (overlap * 0.5f);
         phys1->set_position(posA);
         phys2->set_position(posB);
     }

     // --- Velocity Response ---
     Vec3 velA = phys1->get_velocity();
     Vec3 velB = phys2->get_velocity();
     Vec3 relativeVel = velB - velA;
     float separatingVel = VMath::dot(relativeVel, normal);
  
     if (separatingVel > 0.0f) return; // Already moving apart
  
     // Restitution: 1.8f is likely too high (gains energy). 0.3f is better for chess.
     float restitution = 0.3f; 
     float impulse = -(1.0f + restitution) * separatingVel;
     Vec3 impulseVec = normal * impulse;
  
     if (!collider1->is_collidable && collider2->is_collidable)
     {
         phys2->set_velocity(velB + impulseVec);
     }
     else if (collider1->is_collidable && !collider2->is_collidable)
     {
         phys1->set_velocity(velA - impulseVec);
     }
     else if (collider1->is_collidable && collider2->is_collidable)
     {
         phys1->set_velocity(velA - impulseVec * 0.5f);
         phys2->set_velocity(velB + impulseVec * 0.5f);
     }
 }

