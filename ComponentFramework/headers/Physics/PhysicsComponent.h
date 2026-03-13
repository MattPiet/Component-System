#pragma once
#include <Core/Actor.h>

class PhysicsComponent : public Component
{
    std::weak_ptr<TransformComponent> transform_;
    Vec3 velocity_;
    Vec3 acceleration_;
    Vec3 angular_velocity_;
    Vec3 force_;
    Vec3 torque_;
    float mass_;
    bool is_collideable_ = false;
public:
    PhysicsComponent(std::weak_ptr<Component> parent_, std::shared_ptr<TransformComponent> transform, float mass);
    ~PhysicsComponent() ;
    void update_position(float deltaTime);
    void update_velocity(float deltaTime);
    void update_angular_velocity(float deltaTime);
    void apply_force(const Vec3& force);
    void apply_torque(const float& torque);
    void set_velocity(const Vec3& velocity){velocity_ = velocity;}
    void set_angular_velocity(const Vec3& angular_velocity){angular_velocity_ = angular_velocity;}
    void set_position(const Vec3& position){  transform_.lock()->SetPosition(position); }
    void set_collidable(bool is_collideable) { is_collideable_ = is_collideable; }
    [[nodiscard]] Vec3 get_force() const {return force_;}
    [[nodiscard]] Vec3 get_torque() const {return torque_;}
    [[nodiscard]] Vec3 get_angular_velocity() const {return angular_velocity_;}
    [[nodiscard]] float get_mass() const {return mass_;}
    [[nodiscard]] Vec3 get_velocity() const {return velocity_;}
    [[nodiscard]] Vec3 get_acceleration() const {return acceleration_;}
    [[nodiscard]] Vec3 get_position() const { return transform_.lock()->GetPosition(); }
    [[nodiscard]] Vec3 get_scale() const { return transform_.lock()->GetScale(); }
    [[nodiscard]] Quaternion get_orientation() const { return transform_.lock()->GetQuaternion(); }
    [[nodiscard]] Matrix4 get_world_transform() const { return transform_.lock()->GetTransformMatrix(); }
    bool is_collideable() const {return is_collideable_;}
    bool OnCreate() override;
    void OnDestroy() override;
    void Update(const float deltaTime_) override;
    void Render() const override;
};


