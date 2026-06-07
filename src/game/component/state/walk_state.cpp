#include "walk_state.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/core/context.h"
#include "../../../engine/input/input_manager.h"
#include "../player_component.h"
#include "fall_state.h"
#include "idle_state.h"
#include "jump_state.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace game::component::state {
void WalkState::enter() {}
void WalkState::exit() {}

std::unique_ptr<PlayerState>
WalkState::handleInput(engine::core::Context &context) {
  auto input_manager = context.getInputManager();
  auto physics_component = player_component_->getPhysicsComponent();
  auto sprite_component = player_component_->getSpriteComponent();

  if (input_manager.isActionDown("jump")) {
    return std::make_unique<JumpState>(player_component_);
  }

  if (input_manager.isActionDown("move_left")) {
    if (physics_component->velocity_.x > 0.0f) {
      physics_component->velocity_.x =
          0.0f; // 如果当前速度向右则先减速到0(增强操作手感)
    }

    physics_component->addForce({-player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(true);
  } else if (input_manager.isActionDown("move_right")) {
    if (physics_component->velocity_.x < 0.0f) {
      physics_component->velocity_.x = 0.0f;
    }

    physics_component->addForce({player_component_->getMoveForce(), 0.0f});
    sprite_component->setFlipped(false);
  } else {
    return std::make_unique<IdleState>(player_component_);
  }

  return nullptr;
}

std::unique_ptr<PlayerState> WalkState::update(float, engine::core::Context &) {
  auto physics_component = player_component_->getPhysicsComponent();
  auto max_speed = player_component_->getMaxSpeed();
  physics_component->velocity_.x =
      glm::clamp(physics_component->velocity_.x, -max_speed, max_speed);

  if (!physics_component->hasCollidedBelow()) {
    return std::make_unique<FallState>(player_component_);
  }

  return nullptr;
}

} // namespace game::component::state