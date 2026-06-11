#pragma once
#include "ai_behavior.h"
#include <glm/vec2.hpp>

namespace game::component::ai {
class JumpBehavior final : public AIBehavior {
private:
  float patrol_min_x_;
  float patrol_max_x_;
  glm::vec2 jump_vel_;
  float jump_interval_;

  float jump_timer_ = 0.0f;
  bool jumping_right_ = false;

public:
  JumpBehavior(float min_x, float max_x, glm::vec2 jump_vel = {120.0f, -300.0f},
               float interval = 2.0f);

protected:
  void enter(AIComponent &ai_component) override;
  void update(float delta_time, AIComponent &ai_component) override;
};

} // namespace game::component::ai