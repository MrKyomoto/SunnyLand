#pragma once
#include "ai_behavior.h"

namespace game::component::ai {
class UpDownBehavior final : public AIBehavior {
private:
  float patrol_min_y_;
  float patrol_max_y_;
  float move_speed_;

public:
  UpDownBehavior(float min_y, float max_y, float speed = 60.0f);

protected:
  void enter(AIComponent &ai_component) override;
  void update(float delta_time, AIComponent &ai_component) override;
};

} // namespace game::component::ai