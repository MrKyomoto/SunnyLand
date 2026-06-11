#include "updown_behavior.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/object/game_object.h"
#include "../ai_component.h"
#include <spdlog/spdlog.h>

namespace game::component::ai {

UpDownBehavior::UpDownBehavior(float min_y, float max_y, float speed)
    : patrol_min_y_(min_y), patrol_max_y_(max_y), move_speed_(speed) {
  if (patrol_min_y_ > patrol_max_y_) {
    spdlog::error("UpDownBehavior: min_y {} should be less than max_y {}",
                  min_y, max_y);
    patrol_min_y_ = patrol_max_y_;
  }
}

void UpDownBehavior::enter(AIComponent &ai_component) {
  if (auto *ac = ai_component.getAnimationComponent()) {
    ac->playAnimation("fly");
  }

  if (auto *pc = ai_component.getPhysicsComponent()) {
    pc->setUseGravity(false);
  }
}

void UpDownBehavior::update(float delta_time, AIComponent &ai_component) {
  auto *pc = ai_component.getPhysicsComponent();
  auto *tc = ai_component.getTransformComponent();
  auto *sc = ai_component.getSpriteComponent();
  if (!pc || !tc || !sc) {
    spdlog::error("AIComponent 缺少必要组件");
    return;
  }

  auto current_y = tc->getPosition().y;
  if (pc->hasCollidedBelow() || current_y >= patrol_max_y_) {
    pc->velocity_.y = -move_speed_;
  } else if (pc->hasCollidedAbove() || current_y <= patrol_min_y_) {
    pc->velocity_.y = move_speed_;
  }
}

} // namespace game::component::ai