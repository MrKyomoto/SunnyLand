#include "patrol_behavior.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/object/game_object.h"
#include "../ai_component.h"
#include <spdlog/spdlog.h>

namespace game::component::ai {

PatrolBehavior::PatrolBehavior(float min_x, float max_x, float speed)
    : patrol_min_x_(min_x), patrol_max_x_(max_x), move_speed_(speed) {
  if (patrol_min_x_ >= patrol_max_x_) {
    spdlog::error("PatrolBehavior: min_x {} should be less than max_x {}",
                  min_x, max_x);
    patrol_min_x_ = patrol_max_x_;
  }
}
void PatrolBehavior::enter(AIComponent &ai_component) {
  if (auto *ac = ai_component.getAnimationComponent()) {
    ac->playAnimation("walk");
  }
}
void PatrolBehavior::update(float, AIComponent &ai_component) {
  auto *pc = ai_component.getPhysicsComponent();
  auto *tc = ai_component.getTransformComponent();
  auto *sc = ai_component.getSpriteComponent();
  if (!pc || !tc || !sc) {
    spdlog::error("AIComponent 缺少必要组件");
    return;
  }

  auto current_x = tc->getPosition().x;
  if (pc->hasCollidedRight() || current_x >= patrol_max_x_) {
    pc->velocity_.x = -move_speed_;
    moving_right_ = false;
  } else if (pc->hasCollidedLeft() || current_x <= patrol_min_x_) {
    pc->velocity_.x = move_speed_;
    moving_right_ = true;
  }
  sc->setFlipped(moving_right_);
}
} // namespace game::component::ai