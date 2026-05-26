#include "component.h"
#include <glm/vec2.hpp>

namespace engine::object {
class GameObject;
}

namespace engine::physics {
class PhysicsEngine;
}

namespace engine::component {
class TransformComponent;

class PhysicsComponent final : public Component {
  friend class engine::object::GameObject;

public:
  glm::vec2 velocity_ = {0.0f, 0.0f};

private:
  engine::physics::PhysicsEngine *physics_engine_ = nullptr;
  TransformComponent *transform_ = nullptr;

  // 当前帧受到的力
  glm::vec2 force_ = {0.0f, 0.0f};
  float mass_ = 1.0f;
  bool use_gravity_ = true;
  bool enabled_ = true;

public:
  PhysicsComponent(engine::physics::PhysicsEngine *physics_engine,
                   bool use_gravity = true, float mass = 1.0f);
  ~PhysicsComponent() override = default;

  PhysicsComponent(const PhysicsComponent &) = delete;
  PhysicsComponent &operator=(const PhysicsComponent &) = delete;
  PhysicsComponent(const PhysicsComponent &&) = delete;
  PhysicsComponent &operator=(const PhysicsComponent &&) = delete;

  void addForce(const glm::vec2 &force) {
    if (enabled_)
      force_ += force;
  }
  void clearForce() { force_ = {0.0f, 0.0f}; }
  const glm::vec2 &getForce() const { return force_; }
  float getMass() const { return mass_; }
  bool isEnabled() const { return enabled_; }
  bool isUserGravity() const { return use_gravity_; }

  void setEnabled(bool enabled) { enabled_ = enabled; }
  void setMass(float mass) { mass_ = (mass >= 0.0f) ? mass : 1.0f; }
  void setUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }
  void setVelocity(const glm::vec2 &velocity) { velocity_ = velocity; }
  const glm::vec2 &getVelocity() const { return velocity_; }
  TransformComponent *getTransfrom() const { return transform_; }

private:
  void init() override;
  void update(float, engine::core::Context &) override {}
  void clean() override;
};

} // namespace engine::component