
#include "jump_state.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../player_component.h"
#include "dual_jump_state.h"
#include "fall_state.h"
#include "idle_state.h"
#include "walk_state.h"
#include <glm/common.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace game::component::state {
void JumpState::enter() {
  playAnimation("jump");
  auto physics_component = player_component_->getPhysicsComponent();
  physics_component->velocity_.y = -player_component_->getJumpForce();
}
void JumpState::exit() {}

std::unique_ptr<PlayerState>
JumpState::handleInput(engine::core::Context &context) {
  auto input_manager = context.getInputManager();
  auto sprite_component = player_component_->getSpriteComponent();
  auto physics_component = player_component_->getPhysicsComponent();

  if (player_component_->isCanDualJump() && input_manager.isActionDown("jump")) {
    player_component_->setCanDualJump(false);
    return std::make_unique<DualJumpState>(player_component_);
  }

  else if (input_manager.isActionDown("move_left")) {
    if (physics_component->velocity_.x > 0.0f) {
      physics_component->velocity_.x = 0.0f;
    }
    physics_component->addForce({-player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(true);
  } else if (input_manager.isActionDown("move_right")) {
    if (physics_component->velocity_.x < 0.0f) {
      physics_component->velocity_.x = 0.0f;
    }
    physics_component->addForce({player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(false);
  }

  return nullptr;
}

std::unique_ptr<PlayerState> JumpState::update(float delta_time,
                                               engine::core::Context &) {
  auto physics_component = player_component_->getPhysicsComponent();
  auto max_speed = player_component_->getMaxSpeed();
  physics_component->velocity_.x =
      glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

  dual_jump_timer_ += delta_time;
  if (dual_jump_timer_ >= player_component_->getDualJumpCD()) {
    dual_jump_timer_ = 0.0;
    player_component_->setCanDualJump(true);
  }

  if (physics_component->getVelocity().y > 0.0f) {
    return std::make_unique<FallState>(player_component_);
  }

  return nullptr;
}

} // namespace game::component::state
