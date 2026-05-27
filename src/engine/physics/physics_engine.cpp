#include "physics_engine.h"
#include "../component/collider_component.h"
#include "../component/physics_component.h"
#include "../component/tilelayer_component.h"
#include "../component/transform_component.h"
#include "../object/game_object.h"
#include "collision.h"
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

void PhysicsEngine::registerCollisionLayer(
    engine::component::TileLayerComponent *layer) {
  collision_tile_layers_.push_back(layer);
  layer->setPhysicsEngine(this);
  spdlog::trace("TileLayerComponent 注册完成");
}

void PhysicsEngine::unregisterCollisionLayer(
    engine::component::TileLayerComponent *layer) {
  auto it = std::remove(collision_tile_layers_.begin(),
                        collision_tile_layers_.end(), layer);
  spdlog::trace("TileLayerComponent 注销完成");
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

    resolveTileCollisions(pc, delta_time);
    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
  }

  checkObjectCollisions();
}

void PhysicsEngine::checkObjectCollisions() {
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
      auto *pc_b = components_[j];
      if (!pc_b || !pc_b->isEnabled())
        continue;

      auto *obj_b = pc_b->getOwner();
      if (!obj_b)
        continue;

      auto *cc_b = obj_b->getComponent<engine::component::ColliderComponent>();
      if (!cc_b || !cc_b->isActive())
        continue;

      // 通过保护性测试后,正式执行逻辑

      if (collision::checkCollision(*cc_a, *cc_b)) {
        // TODO: 并不是所有碰撞都需要插入 collision_pairs_,未来会添加过滤条件
        collision_pairs_.emplace_back(obj_a, obj_b);
      }
    }
  }
}

void PhysicsEngine::resolveTileCollisions(
    engine::component::PhysicsComponent *pc, float delta_time) {

  auto *obj = pc->getOwner();
  if (!obj)
    return;
  auto *tc = obj->getComponent<engine::component::TransformComponent>();
  auto *cc = obj->getComponent<engine::component::ColliderComponent>();
  if (!tc || !cc || !cc->isActive() || cc->isTrigger())
    return;

  auto world_aabb = cc->getWorldAABB();
  auto obj_pos = world_aabb.position;
  auto obj_size = world_aabb.size;
  if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f)
    return;

  // 检查右边缘和下边缘时,需要减1px,否则会检查到下一行/列的瓦片
  auto tolerance = 1.0f;
  auto ds = pc->getVelocity() * delta_time;
  auto new_obj_pos = obj_pos + ds;

  for (auto *layer : collision_tile_layers_) {
    if (!layer)
      continue;

    auto tile_size = layer->getTileSize();

    // 轴分离碰撞检测,先检查X方向是否有碰撞(y方向使用初始值obj_pos.y)
    if (ds.x > 0.0f) {
      // 检测右侧碰撞,分别测试右上和右下角
      auto right_top_x = new_obj_pos.x + obj_size.x;
      auto tile_x = static_cast<int>(floor(right_top_x / tile_size.x));
      auto tile_y = static_cast<int>(floor(obj_pos.y / tile_size.y));
      auto tile_type_top = layer->getTileTypeAt({tile_x, tile_y});
      auto tile_y_bottom = static_cast<int>(
          floor((obj_pos.y + obj_size.y - tolerance) / tile_size.y));
      auto tile_type_bottom = layer->getTileTypeAt({tile_x, tile_y_bottom});

      if (tile_type_top == component::TileType::SOLID ||
          tile_type_bottom == component::TileType::SOLID) {
        new_obj_pos.x = tile_x * layer->getTileSize().x - obj_size.x;
        // hit wall, v.x set to 0
        pc->setVelocity({0.0f, pc->getVelocity().y});
      }
    } else if (ds.x < 0.0f) {
      auto left_top_x = new_obj_pos.x;
      auto tile_x = static_cast<int>(floor(left_top_x / tile_size.x));
      auto tile_y = static_cast<int>(floor(obj_pos.y / tile_size.y));
      auto tile_type_top = layer->getTileTypeAt({tile_x, tile_y});
      auto tile_y_bottom = static_cast<int>(
          floor((obj_pos.y + obj_size.y - tolerance) / tile_size.y));
      auto tile_type_bottom = layer->getTileTypeAt({tile_x, tile_y_bottom});

      if (tile_type_top == component::TileType::SOLID ||
          tile_type_bottom == component::TileType::SOLID) {
        new_obj_pos.x = (tile_x + 1) * layer->getTileSize().x;
        // hit wall, v.x set to 0
        pc->setVelocity({0.0f, pc->getVelocity().y});
      }
    }

    if (ds.y > 0.0f) {
      auto bottom_left_y = new_obj_pos.y + obj_size.y;
      auto tile_y = static_cast<int>(floor(bottom_left_y / tile_size.y));
      auto tile_x = static_cast<int>(floor(obj_pos.x / tile_size.x));
      auto tile_type_left = layer->getTileTypeAt({tile_x, tile_y});
      auto tile_x_right = static_cast<int>(
          floor((obj_pos.x + obj_size.x - tolerance) / tile_size.x));
      auto tile_type_right = layer->getTileTypeAt({tile_x_right, tile_y});

      if (tile_type_left == component::TileType::SOLID ||
          tile_type_right == component::TileType::SOLID) {
        new_obj_pos.y = tile_y * layer->getTileSize().y - obj_size.y;
        pc->setVelocity({pc->getVelocity().x, 0.0f});
      }
    } else if (ds.y < 0.0f) {
      auto top_left_y = new_obj_pos.y;
      auto tile_y = static_cast<int>(floor(top_left_y / tile_size.y));
      auto tile_x = static_cast<int>(floor(obj_pos.x / tile_size.x));
      auto tile_type_left = layer->getTileTypeAt({tile_x, tile_y});
      auto tile_x_right = static_cast<int>(
          floor((obj_pos.x + obj_size.x - tolerance) / tile_size.x));
      auto tile_type_right = layer->getTileTypeAt({tile_x_right, tile_y});

      if (tile_type_left == component::TileType::SOLID ||
          tile_type_right == component::TileType::SOLID) {
        new_obj_pos.y = (tile_y + 1) * layer->getTileSize().y;
        pc->setVelocity({pc->getVelocity().x, 0.0f});
      }
    }
  }
  tc->setPosition(new_obj_pos);
}

} // namespace engine::physics