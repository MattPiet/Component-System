#include <Physics/CollisionComponent.h>
#include <glew.h>
CollisionComponent::CollisionComponent(std::weak_ptr<Component> parent_, Plane plane_) : Component(parent_), plane(plane_)
{
    colliderType = ColliderType::PLANE;
}

void CollisionComponent::calculate_center(const Vec3& position, const Quaternion& Ori)
{
    if (colliderType == ColliderType::SPHERE)
    {
        sphere.center = Vec3(position.x, position.y + radius, position.z);
    }
    else if (colliderType == ColliderType::AABB)
    {
        aabb.center = Vec3(position.x, position.y + halfExtents.y, position.z);
        aabb.halfExtents = halfExtents;
    }
    else if (colliderType == ColliderType::OBB)
    {
        Vec3 offset = Vec3(0.0f, halfExtents.y, 0.0f);
        Vec3 rotatedOffset = QMath::rotate(offset, Ori);

        obb.center = position + rotatedOffset;
        obb.halfExtents = halfExtents;
    }
    
}

MATH::Matrix4 CollisionComponent::CalculateModelMatrix(MATH::Matrix4 BaseMM) const
{
    	
    MATH::Matrix4 colliderModelMatrix = BaseMM;

    switch (get_colliderType())
    {
    case ColliderType::SPHERE:
        {
            colliderModelMatrix = colliderModelMatrix * MMath::translate(Vec3(0.0f, sphere.r, 0.0f))
                                                       * MMath::scale(Vec3(sphere.r, sphere.r, sphere.r));
        }
        break;
    case ColliderType::AABB:
        {
            colliderModelMatrix = colliderModelMatrix * MMath::translate(Vec3(0.0f, halfExtents.y, 0.0f))
                                                       * MMath::scale(halfExtents);
        }
        break;

    case ColliderType::OBB:
        {
            colliderModelMatrix = colliderModelMatrix * MMath::translate(Vec3(0.0f, halfExtents.y, 0.0f))
                                                       * MMath::scale(halfExtents);
        }
        break;
    default:
			
        break;
    }
    return colliderModelMatrix;
}

void CollisionComponent::Render() const
{
    
}

CollisionComponent::CollisionComponent(std::weak_ptr<Component> parent_, float radius_) : Component(parent_), radius(radius_)
{
    colliderType = ColliderType::SPHERE;
    sphere.r = radius;
}

CollisionComponent::CollisionComponent(std::weak_ptr<Component> parent_, Vec3 halfExtents_) : Component(parent_), halfExtents(halfExtents_)
{
    colliderType = ColliderType::AABB;
    aabb.halfExtents = halfExtents;
    obb.halfExtents = halfExtents;
}

