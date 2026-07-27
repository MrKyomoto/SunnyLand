#include "hurt_state.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../player_component.h"
#include "fall_state.h"
#include "idle_state.h"

namespace game::component::state {

void HurtState::enter() {
  playAnimation("hurt");
  auto physics_component = player_component_->getPhysicsComponent();
  auto sprite_component = player_component_->getSpriteComponent();
  auto knockback_velocity = glm::vec2(-100.0f, -150.0f);
  if (sprite_component->isFlipped()) {
    knockback_velocity.x = -knockback_velocity.x;
  }

  physics_component->velocity_ = knockback_velocity;

  if (auto *audio = player_component_->getAudioComponent(); audio) {
    audio->playSound("hurt");
  }
}
void HurtState::exit(){}

std::unique_ptr<PlayerState> HurtState::handleInput(engine::core::Context &) {
  // 硬直期不能进行任何操控
  return nullptr;
}

std::unique_ptr<PlayerState> HurtState::update(float delta_time,
                                               engine::core::Context &) {
  stunned_timer_ += delta_time;
  auto physics_component = player_component_->getPhysicsComponent();
  if (physics_component->hasCollidedBelow()) {
    return std::make_unique<IdleState>(player_component_);
  }

  if (stunned_timer_ > player_component_->getStunnedDuration()) {
    return std::make_unique<FallState>(player_component_);
  }

  return nullptr;

}

} // namespace game::component::state