#include "physics_engine.h"
#include "../component/collider_component.h"
#include "../component/physics_component.h"
#include "../component/transform_component.h"
#include "../object/game_object.h"
#include <glm/common.hpp>
#include <spdlog/spdlog.h>
#include "collision.h"

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
  collision_pairs_.clear();

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

    checkObjectCollision();
  }
}

void PhysicsEngine::checkObjectCollision() {
  for (size_t i = 0; i < components_.size(); i++) {
    auto *pc_a = components_[i];
    if (!pc_a || !pc_a->isEnabled())
      continue;

    auto *obj_a = pc_a->getOwner();
    if (!obj_a)
      continue;

    auto *cc_a = obj_a->getComponent<engine::component::ColliderComponent>();
    if (!cc_a || !cc_a->isActive())
      continue;

    for (size_t j = i + 1; j < components_.size(); j++) {
      auto* pc_b = components_[j];
      if(!pc_b || !pc_b->isEnabled()) continue;

      auto* obj_b = pc_b->getOwner();
      if(!obj_b) continue;
      
      auto* cc_b = obj_b->getComponent<engine::component::ColliderComponent>();
      if(!cc_b || !cc_b->isActive()) continue;

      // 通过保护性测试后,正式执行逻辑

      if (collision::checkCollision(*cc_a, *cc_b)) {
        // TODO: 并不是所有碰撞都需要插入 collision_pairs_,未来会添加过滤条件
        collision_pairs_.emplace_back(obj_a,obj_b);
      }
    }
  }
}

} // namespace engine::physics