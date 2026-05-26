#include "physics_engine.h"
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace engine::physics {

void PhysicsEngine::registerComponent(
    engine::component::PhysicsComponent *component) {
  components_.push_back(component);
  spdlog::trace("PhysicsComponent 注册完成");
}

void PhysicsEngine::unregisterComponent(
    engine::component::PhysicsComponent *component) {
  auto it = std::remove(components_.begin(), components_.end(), component);

  spdlog::trace("PhysicsComponent 注销完成");
}

void PhysicsEngine::update(float delta_time) {
  for (auto *pc : components_) {
    if (!pc || !pc->isEnabled()) {
      continue;
    }

    // F = g * m
    if (pc->isUserGravity()) {
      pc->addForce(gravity_ * pc->getMass());
    }

    // 还可以添加其它力的影响,比如风力摩擦力等,目前不考虑
    // 更新速度 v += a * dt , a = F / m
    pc->velocity_ += (pc->getForce() / pc->getMass()) * delta_time;
    // 清除当前帧的力
    pc->clearForce();

    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);

    auto *tc = pc->getTransfrom();
    if (tc) {
      tc->translate(pc->getVelocity() * delta_time);
    }

  }
}
} // namespace engine::physics