#pragma once
#include "component.h"

namespace engine::component {
class HealthComponent final : public engine::component::Component {
  friend class engine::object::GameObject;

private:
  int max_health_ = 1;
  int current_health_ = 1;
  bool is_invincible_ = false;          ///< @brief 是否处于无敌状态
  float invincibility_duration_ = 2.0f; ///< @brief 受伤后无敌的总时长(s)
  float invincibility_timer_ = 0.0f;    ///< @brief 无敌时间计数器(s)

public:
  explicit HealthComponent(int max_health = 1,
                           float invincibility_duration = 2.0f);
  ~HealthComponent() override = default;

  /**
   * @brief 对 GameObject 施加伤害
   * @param damage_amount 应为正数
   * @return 成功造成伤害则返回true
   */
  bool takeDamage(int damage_amount);
  void heal(int heal_amount);

  bool isAlive() const { return current_health_ > 0; }
  int getMaxHealth() const { return max_health_; }
  int getCurrentHealth() const { return current_health_; }
  bool isInvincible() const { return is_invincible_; }
  float getInvincibilityDuration() const { return invincibility_duration_; }
  float getInvincibilityTimer() const { return invincibility_timer_; }

  void setMaxHealth(int max_health);
  void setCurrentHealth(int current_health);
  void setInvincible(bool invincible){is_invincible_ = invincible;}
  void setInvincibilityDuration(float invincibility_duration){invincibility_duration_=invincibility_duration;}
  void setInvincibilityTimer(float timer);

protected:
  void update(float,engine::core::Context&) override;
};

} // namespace engine::component