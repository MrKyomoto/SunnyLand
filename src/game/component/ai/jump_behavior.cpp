#include "jump_behavior.h"
#include "../../../engine/component/animation_component.h"
#include "../../../engine/component/audio_component.h"
#include "../../../engine/component/physics_component.h"
#include "../../../engine/component/sprite_component.h"
#include "../../../engine/component/transform_component.h"
#include "../../../engine/object/game_object.h"
#include "../ai_component.h"
#include <spdlog/spdlog.h>

namespace game::component::ai {

JumpBehavior::JumpBehavior(float min_x, float max_x, glm::vec2 jump_vel,
                           float interval)
    : patrol_min_x_(min_x), patrol_max_x_(max_x), jump_vel_(jump_vel),
      jump_interval_(interval) {
  if (patrol_min_x_ > patrol_max_x_) {
    spdlog::error("PatrolBehavior: min_x {} should be less than max_x {}",
                  min_x, max_x);
    patrol_min_x_ = patrol_max_x_;
  }
  if (jump_interval_ <= 0.0f) { // 确保跳跃间隔是正数
    spdlog::error("JumpBehavior: jump_interval ({}) 应为正数。已设置为 2.0f。",
                  jump_interval_);
    jump_interval_ = 2.0f;
  }
  if (jump_vel_.y > 0) { // 确保垂直跳跃速度是负数（向上）
    spdlog::error(
        "JumpBehavior: 垂直跳跃速度 ({}) 应为负数（向上）。已取相反数。",
        jump_vel_.y);
    jump_vel_.y = -jump_vel_.y;
  }
}
void JumpBehavior::enter(AIComponent&){}

void JumpBehavior::update(float delta_time, AIComponent &ai_component) {
  auto *pc = ai_component.getPhysicsComponent();
  auto *tc = ai_component.getTransformComponent();
  auto *sc = ai_component.getSpriteComponent();
  auto *ac = ai_component.getAnimationComponent();
  if (!pc || !tc || !sc || !ac) {
    spdlog::error("AIComponent 缺少必要组件");
    return;
  }

  auto is_on_ground = pc->hasCollidedBelow();
  if (is_on_ground) {
    // 刚刚落地时播放叫声音效
    if (jump_timer_ == 0.0f) {
      if (auto *audio = ai_component.getAudioComponent(); audio) {
        audio->playSound("cry", -1, true);
      }
    }

    jump_timer_ += delta_time;
    pc->velocity_.x = 0.0f;

    if (jump_timer_ >= jump_interval_) {
      jump_timer_ = 0.0f;
      auto current_x = tc->getPosition().x;
      if (jumping_right_ &&
          (pc->hasCollidedRight() || current_x >= patrol_max_x_)) {
        jumping_right_ = false;
      } else if (!jumping_right_ &&
                 (pc->hasCollidedLeft() || current_x <= patrol_min_x_)) {
        jumping_right_ = true;
      }

      auto jump_vel_x = jumping_right_ ? jump_vel_.x : -jump_vel_.x;
      pc->velocity_ = {jump_vel_x, jump_vel_.y};
      sc->setFlipped(jumping_right_);
      ac->playAnimation("jump");
    } else {
      ac->playAnimation("idle");
    }
  } else {
    if (pc->getVelocity().y < 0) {
      ac->playAnimation("jump");
    } else {
      ac->playAnimation("fall");
    }
  }
}
} // namespace game::component::ai