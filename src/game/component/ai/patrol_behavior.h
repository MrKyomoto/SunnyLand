#pragma once
#include "ai_behavior.h"

namespace game::component::ai {

class PatrolBehavior final : public AIBehavior {
private:
  float patrol_min_x_;
  float patrol_max_x_;
  float move_speed_;
  bool moving_right_ = true;

public:
  PatrolBehavior(float min_x, float max_x, float speed = 80.0f);

protected:
  void enter(AIComponent &ai_component) override;
  void update(float delta_time, AIComponent &ai_component) override;
};

} // namespace game::component::ai