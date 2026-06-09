#include "ai_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/object/game_object.h"
#include <spdlog/spdlog.h>

namespace game::component {

void AIComponent::init() {
  if (!owner_) {
    spdlog::error("AIComponent 没有所属游戏对象");
    return;
  }

  transform_component_ =
      getOwner()->getComponent<engine::component::TransformComponent>();
  physics_component_ =
      getOwner()->getComponent<engine::component::PhysicsComponent>();
  sprite_component_ =
      getOwner()->getComponent<engine::component::SpriteComponent>();
  animation_component_ =
      getOwner()->getComponent<engine::component::AnimationComponent>();

  if (!transform_component_ || !physics_component_ || !sprite_component_ ||
      !animation_component_) {
    spdlog::error("AIComponent's Owner 对象缺少必要组件");
    return;
  }
}

void AIComponent::setBehavior(std::unique_ptr<ai::AIBehavior> behavior) {
  current_behavior_ = std::move(behavior);
  if (current_behavior_) {
    current_behavior_->enter(*this);
  }
}

void AIComponent::update(float delta_time, engine::core::Context &) {
  if (current_behavior_) {
    current_behavior_->update(delta_time, *this);
  }
}

} // namespace game::component