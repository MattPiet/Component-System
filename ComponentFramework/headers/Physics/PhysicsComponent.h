#pragma once
#include <Core/Actor.h>

class PhysicsComponent : public TransformComponent
{
    Vec3 velocity_;
    Vec3 acceleration_;
    Vec3 angular_velocity_;
    Vec3 force_;
    Vec3 torque_;
    float mass_;
public:
    PhysicsComponent(std::weak_ptr<Component> parent_,Vec3 pos_, Quaternion orientation_, Vec3 scale_ = Vec3(1.0f, 1.0f, 1.0f));
    PhysicsComponent(std::weak_ptr<Component> parent_);
    ~PhysicsComponent() ;
    void update_position(float deltaTime);
    void update_velocity(float deltaTime);
    void update_angular_velocity(float deltaTime);
    void apply_force(const Vec3& force);
    void apply_torque(const float& torque);
    void set_velocity(const Vec3& velocity){velocity_ = velocity;}
    void set_angular_velocity(const Vec3& angular_velocity){angular_velocity_ = angular_velocity;}
    void set_position(const Vec3& position){  SetPosition(position); }
    void set_mass(const float& mass){mass_ = mass;} 
    [[nodiscard]] Vec3 get_force() const {return force_;}
    [[nodiscard]] Vec3 get_torque() const {return torque_;}
    [[nodiscard]] Vec3 get_angular_velocity() const {return angular_velocity_;}
    [[nodiscard]] float get_mass() const {return mass_;}
    [[nodiscard]] Vec3 get_velocity() const {return velocity_;}
    [[nodiscard]] Vec3 get_acceleration() const {return acceleration_;}
    bool OnCreate() override;
    void OnDestroy() override;
    void Update(const float deltaTime_) override;
    void Render() const override;
};


