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

  // 碰撞状态标志
  bool collided_below_ = false;
  bool collided_above_ = false;
  bool collided_left_ = false;
  bool collided_right_ = false;

  bool collided_ladder_ =
      false; // 梯子不同于危险瓦片，它不是一个触发一次性事件的物品，而是一个能
             // 持续影响
             // 玩家状态的区域。因此，我们不把它当作"触发器事件"，而是直接在物理引擎中检测，并设置一个状态标志

  bool is_on_top_ladder_ = false;
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
  bool isUseGravity() const { return use_gravity_; }

  void setEnabled(bool enabled) { enabled_ = enabled; }
  void setMass(float mass) { mass_ = (mass >= 0.0f) ? mass : 1.0f; }
  void setUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }
  void setVelocity(const glm::vec2 &velocity) { velocity_ = velocity; }
  const glm::vec2 &getVelocity() const { return velocity_; }
  TransformComponent *getTransfrom() const { return transform_; }

  void resetCollisionFlags() {
    collided_above_ = false;
    collided_below_ = false;
    collided_right_ = false;
    collided_left_ = false;

    collided_ladder_ = false;

    is_on_top_ladder_ = false;
  }

  void setCollidedBelow(bool collided) { collided_below_ = collided; }
  void setCollidedAbove(bool collided) { collided_above_ = collided; }
  void setCollidedLeft(bool collided) { collided_left_ = collided; }
  void setCollidedRight(bool collided) { collided_right_ = collided; }
  void setCollidedLadder(bool collided) { collided_ladder_ = collided; }

  bool hasCollidedBelow() const { return collided_below_; }
  bool hasCollidedAbove() const { return collided_above_; }
  bool hasCollidedLeft() const { return collided_left_; }
  bool hasCollidedRight() const { return collided_right_; }
  bool hasCollidedLadder() const { return collided_ladder_; }

  void setOnTopLadder(bool on_top) { is_on_top_ladder_ = on_top; }
  bool isOnTopLadder() const {return is_on_top_ladder_;}
private:
  void init() override;
  void update(float, engine::core::Context &) override {}
  void clean() override;
};

} // namespace engine::component