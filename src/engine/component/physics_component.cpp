#include "physics_component.h"
#include "../object/game_object.h"
#include "../physics/physics_engine.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::component {
PhysicsComponent::PhysicsComponent(
    engine::physics::PhysicsEngine *physics_engine, bool use_gravity,
    float mass)
    : physics_engine_(physics_engine), mass_(mass >= 0.0f ? mass : 1.0f),
      use_gravity_(use_gravity) {
  if (!physics_engine_) {
    spdlog::error("PhysicsComponent 构造函数中 PhysicsEngine 指针不能为空");
  }
  spdlog::trace("PhysicsComponent 构建完成, 质量: {},使用重力: {}", mass_,
                use_gravity_);
}

void PhysicsComponent::init() {
  if (!owner_) {
    spdlog::error("PhysicsComponent 初始化前需要一个GameObject作为owner");
    return;
  }
  if (!physics_engine_) {
    spdlog::error("PhysicsComponent 初始化时 PhysicsEngine 未正确初始化");
    return;
  }
  transform_ = owner_->getComponent<TransformComponent>();
  if (!transform_) {
    spdlog::warn("PhysicsComponent 初始化时, 同一GameObject上未找到 TransformComponent 组件");
    return;
  }

  physics_engine_->registerComponent(this);
  spdlog::trace("PhysicsComponent is initialized");
}
void PhysicsComponent::clean() {
  physics_engine_->unregisterComponent(this);
  spdlog::trace("PhysicsComponent清理完成");
}

} // namespace engine::component