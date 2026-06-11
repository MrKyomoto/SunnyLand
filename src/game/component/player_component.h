#pragma once
#include "../../engine/component/component.h"
#include "state/player_state.h"
#include <memory>

namespace engine::input {
class InputManager;
}

namespace engine::component {
class TransformComponent;
class PhysicsComponent;
class SpriteComponent;
class AnimationComponent;
class HealthComponent;
} // namespace engine::component

namespace game::component::state {
class PlayerState;
}

namespace game::component {

class PlayerComponent final : public engine::component::Component {
  friend class engine::object::GameObject;

private:
  engine::component::TransformComponent *transform_component_ = nullptr;
  engine::component::PhysicsComponent *physics_component_ = nullptr;
  engine::component::SpriteComponent *sprite_component_ = nullptr;
  engine::component::AnimationComponent *animation_component_ = nullptr;
  engine::component::HealthComponent *health_component_ = nullptr;

  std::unique_ptr<state::PlayerState> current_state_;
  bool is_dead_ = false;

  float move_force_ = 200.0f;
  float max_speed_ = 120.0f;
  float friction_factor_ = 0.85f;
  float jump_force_ = 350.0f;

  float stunned_duration_ = 0.4f; // (s)
  float dual_jump_cd_ = 0.2f;     // (s)
  bool can_dual_jump_ = false;

public:
  PlayerComponent() = default;
  ~PlayerComponent() override = default;

  PlayerComponent(const PlayerComponent &) = delete;
  PlayerComponent &operator=(const PlayerComponent &) = delete;
  PlayerComponent(PlayerComponent &&) = delete;
  PlayerComponent &operator=(PlayerComponent &&) = delete;

  // setters and getters
  engine::component::TransformComponent *getTransformComponent() const {
    return transform_component_;
  }
  engine::component::SpriteComponent *getSpriteComponent() const {
    return sprite_component_;
  }
  engine::component::PhysicsComponent *getPhysicsComponent() const {
    return physics_component_;
  }
  engine::component::AnimationComponent *getAnimationComponent() const {
    return animation_component_;
  }
  engine::component::HealthComponent *getHealthComponent() const {
    return health_component_;
  }

  void setIsDead(bool is_dead) {
    is_dead_ = is_dead;
  } ///< @brief 设置玩家是否死亡
  bool isDead() const { return is_dead_; } ///< @brief 获取玩家是否死亡
  void setMoveForce(float move_force) {
    move_force_ = move_force;
  } ///< @brief 设置水平移动力
  float getMoveForce() const { return move_force_; } ///< @brief 获取水平移动力
  void setMaxSpeed(float max_speed) {
    max_speed_ = max_speed;
  } ///< @brief 设置最大移动速度
  float getMaxSpeed() const { return max_speed_; } ///< @brief 获取最大移动速度
  void setFrictionFactor(float friction_factor) {
    friction_factor_ = friction_factor;
  } ///< @brief 设置摩擦系数
  float getFrictionFactor() const {
    return friction_factor_;
  } ///< @brief 获取摩擦系数
  void setJumpForce(float jump_force) {
    jump_force_ = jump_force;
  } ///< @brief 设置跳跃力
  float getJumpForce() const { return jump_force_; }
  float getStunnedDuration() const { return stunned_duration_; }
  void setStunnedDuration(float stunned_duration) {
    stunned_duration_ = stunned_duration;
  }
  float getDualJumpCD() const { return dual_jump_cd_; }
  void setDualJumpCD(float dual_jump_cd) { dual_jump_cd_ = dual_jump_cd; }
  void setCanDualJump(bool can_dual_jump) { can_dual_jump_ = can_dual_jump; }
  bool isCanDualJump() const { return can_dual_jump_; }

  void setState(
      std::unique_ptr<state::PlayerState> new_state); ///< @brief 切换玩家状态

  bool takeDamage(int damage);

private:
  void init() override;
  void handleInput(engine::core::Context &context) override;
  void update(float delta_time, engine::core::Context &context) override;
};

} // namespace game::component
