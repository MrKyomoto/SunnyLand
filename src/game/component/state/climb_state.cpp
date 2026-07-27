#include "climb_state.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../player_component.h"
#include "dual_jump_state.h"
#include "fall_state.h"
#include "idle_state.h"
#include "jump_state.h"
#include "walk_state.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace game::component::state {
void ClimbState::enter() {
  playAnimation("climb");
  if (auto *physics = player_component_->getPhysicsComponent(); physics) {
    physics->setUseGravity(false);
  }
}

void ClimbState::exit() {
  if (auto *physics = player_component_->getPhysicsComponent(); physics) {
    physics->setUseGravity(true);
  }
}

std::unique_ptr<PlayerState>
ClimbState::handleInput(engine::core::Context &context) {
  auto input = context.getInputManager();
  auto physics = player_component_->getPhysicsComponent();
  auto anim = player_component_->getAnimationComponent();
  auto speed = player_component_->getClimbSpeed();

  physics->velocity_.y = input.isActionDown("move_up")     ? -speed
                         : input.isActionDown("move_down") ? speed
                                                           : 0.0f;

  physics->velocity_.x = input.isActionDown("move_left")    ? -speed
                         : input.isActionDown("move_right") ? speed
                                                            : 0.0f;

  (physics->velocity_.y != 0.0f || physics->velocity_.x != 0.0f)
      ? anim->resumeAnimation()
      : anim->stopAnimation();

  if (input.isActionPressed("jump")) {
    return std::make_unique<JumpState>(player_component_);
  }

  return nullptr;
}

std::unique_ptr<PlayerState> ClimbState::update(float,
                                                engine::core::Context &) {
  auto physics = player_component_->getPhysicsComponent();
  if (physics->hasCollidedBelow()) {
    return std::make_unique<IdleState>(player_component_);
  }

  if (!physics->hasCollidedLadder()) {
    return std::make_unique<FallState>(player_component_);
  }
  return nullptr;
}

} // namespace game::component::state