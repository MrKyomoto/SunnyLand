#include "health_component.h"
#include "../object/game_object.h"
#include <spdlog/spdlog.h>

namespace engine::component {

HealthComponent::HealthComponent(int max_health, float invincibility_duration)
    : max_health_(max_health), current_health_(max_health),
      invincibility_duration_(invincibility_duration) {
  spdlog::trace("HealthComponent 构造成功");
}

bool HealthComponent::takeDamage(int damage_amount) {
  if (damage_amount <= 0 || !isAlive() || is_invincible_) {
    return false;
  }

  int damage =
      damage_amount > current_health_ ? current_health_ : damage_amount;

  current_health_ -= damage;

  if (isAlive() && invincibility_duration_ > 0.0f) {
    setInvincibilityTimer(invincibility_duration_);
  }
  spdlog::debug("GameObject '{}' 收到了 {} 点伤害, 当前生命值: {}/{}",
                owner_ ? owner_->getName() : "Unknown", damage_amount,
                current_health_, max_health_);

  return true;
}

void HealthComponent::heal(int heal_amount) {
  if (heal_amount <= 0) {
    spdlog::warn("heal应接收正数");
    return;
  }

  if (heal_amount > max_health_ - current_health_) {
    current_health_ = max_health_;
  } else {
    current_health_ += heal_amount;
  }

  return;
}

void HealthComponent::setMaxHealth(int max_health) {
  if (max_health <= 0) {
    spdlog::warn("max_health应大于0");
    return;
  }

  max_health_ = max_health;
  if (current_health_ > max_health_) {
    current_health_ = max_health_;
  }
}
void HealthComponent::setCurrentHealth(int current_health) {
  if (current_health < 0 || current_health > max_health_) {
    spdlog::warn("current_health参数无效");
    return;
  }

  current_health_ = current_health;
}

void HealthComponent::setInvincibilityTimer(float duration) {
  if (duration > 0.0f) {
    is_invincible_ = true;
    invincibility_timer_ = duration;
  }
}
void HealthComponent::update(float delta_time, engine::core::Context &) {
  if (is_invincible_) {
    invincibility_timer_ -= delta_time;
    if (invincibility_timer_ <= 0.0f) {
      is_invincible_ = false;
      invincibility_timer_ = 0.0f;
    }
  }
}
} // namespace engine::component