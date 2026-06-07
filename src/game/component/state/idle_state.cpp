#include "idle_state.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../player_component.h"
#include "fall_state.h"
#include "jump_state.h"
#include "walk_state.h"
#include <spdlog/spdlog.h>

namespace game::component::state {
void IdleState::enter() { playAnimation("idle"); }
void IdleState::exit() {}

std::unique_ptr<PlayerState>
IdleState::handleInput(engine::core::Context &context) {
  auto input_manager = context.getInputManager();
  if (input_manager.isActionDown("move_left") ||
      input_manager.isActionDown("move_right")) {
    return std::make_unique<WalkState>(player_component_);
  }
  if (input_manager.isActionDown("jump")) {
    return std::make_unique<JumpState>(player_component_);
  }
  return nullptr;
}

std::unique_ptr<PlayerState> IdleState::update(float, engine::core::Context &) {
  auto physics_component = player_component_->getPhysicsComponent();
  auto friction_factor = player_component_->getFrictionFactor();
  physics_component->velocity_.x *= friction_factor;

  if (!physics_component->hasCollidedBelow()) {
    return std::make_unique<FallState>(player_component_);
  }

  return nullptr;
}

} // namespace game::component::state