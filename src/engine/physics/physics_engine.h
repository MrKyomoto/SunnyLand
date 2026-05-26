#pragma once
#include <glm/vec2.hpp>
#include <utility> // for std::pair
#include <vector>

namespace engine::component {
class PhysicsComponent;
}

namespace engine::object {
class GameObject;
}

namespace engine::physics {
class PhysicsEngine {
private:
  std::vector<engine::component::PhysicsComponent *> components_;
  // 默认值 (px/s^2,相当于100px对应现实里1m)
  glm::vec2 gravity_ = {0.0f, 980.0f};
  float max_speed_ = 500.0f;

  // @brief 储存本帧发声的 GameObject 碰撞对 (每次 update 开始时清空)
  std::vector<
      std::pair<engine::object::GameObject *, engine::object::GameObject *>>
      collision_pairs_;

public:
  PhysicsEngine() = default;

  PhysicsEngine(const PhysicsEngine &) = delete;
  PhysicsEngine &operator=(const PhysicsEngine &) = delete;
  PhysicsEngine(const PhysicsEngine &&) = delete;
  PhysicsEngine &operator=(const PhysicsEngine &&) = delete;

  void registerComponent(engine::component::PhysicsComponent *component);
  void unregisterComponent(engine::component::PhysicsComponent *component);

  void update(float delta_time);
  void checkObjectCollision();

  void setGravity(const glm::vec2 &gravity) { gravity_ = gravity; }
  const glm::vec2 &getGravity() const { return gravity_; }
  void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }
  float getMaxSpeed() const { return max_speed_; }
  const std::vector<
      std::pair<engine::object::GameObject *, engine::object::GameObject *>> &
  getCollisionPairs() const {
    return collision_pairs_;
  }
};

} // namespace engine::physics