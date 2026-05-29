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
    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);

    resolveTileCollisions(pc, delta_time);

    applyWorldBounds(pc);
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
        // 如果是可移动物体与SOLID物体碰撞,则直接处理位置变化,不用记录碰撞对
        if (obj_a->getTag() != "solid" && obj_b->getTag() == "solid") {
          resolveSolidObjectCollisions(obj_a, obj_b);
        } else if (obj_a->getTag() == "solid" && obj_b->getTag() != "solid") {
          resolveSolidObjectCollisions(obj_b, obj_a);
        } else {
          collision_pairs_.emplace_back(obj_a, obj_b);
        }
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
      } else {
        // 检测右下角斜坡瓦片

        auto width_right = new_obj_pos.x + obj_size.x - tile_x * tile_size.x;
        auto height_right =
            getTileHeightAtWidth(width_right, tile_type_bottom, tile_size);
        if (height_right > 0.0f) {
          if (new_obj_pos.y > (tile_y_bottom + 1) * layer->getTileSize().y -
                                  obj_size.y - height_right) {
            new_obj_pos.y = (tile_y_bottom + 1) * layer->getTileSize().y -
                            obj_size.y - height_right;
          }
        }
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
      } else {
        auto width_left = new_obj_pos.x - tile_x * tile_size.x;
        auto height_left =
            getTileHeightAtWidth(width_left, tile_type_bottom, tile_size);
        if (height_left > 0.0f) {
          if (new_obj_pos.y > (tile_y_bottom + 1) * layer->getTileSize().y -
                                  obj_size.y - height_left) {
            new_obj_pos.y = (tile_y_bottom + 1) * layer->getTileSize().y -
                            obj_size.y - height_left;
          }
        }
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
          tile_type_right == component::TileType::SOLID ||
          tile_type_left == component::TileType::UNISOLID ||
          tile_type_right == component::TileType::UNISOLID) {
        new_obj_pos.y = tile_y * layer->getTileSize().y - obj_size.y;
        pc->setVelocity({pc->getVelocity().x, 0.0f});
      } else {
        // 下方两个角点都要检测
        auto width_left = obj_pos.x - tile_x * tile_size.x;
        auto width_right = obj_pos.x + obj_size.x - tile_x_right * tile_size.x;
        auto height_left =
            getTileHeightAtWidth(width_left, tile_type_left, tile_size);
        auto height_right =
            getTileHeightAtWidth(width_right, tile_type_right, tile_size);
        auto height = glm::max(height_left, height_right);
        if (height > 0.0f) {
          if (new_obj_pos.y >
              (tile_y + 1) * layer->getTileSize().y - obj_size.y - height) {
            new_obj_pos.y =
                (tile_y + 1) * layer->getTileSize().y - obj_size.y - height;
            pc->velocity_.y = 0.0f; // 只有向下运动时才需要让y速度归零
          }
        }
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
  // 使用translate 方法,避免直接设置位置,因为碰撞盒可能有偏移量
  // 解释起来是这样的,这个函数计算的时候使用的pos一直都是碰撞盒的pos,但是碰撞盒和实体本身不一定是完全重合的,是有偏移量的,所以不能最后直接用setPosition(new_obj_pos),但是不管偏移量是多少,盒子移动的距离和实体移动的距离肯定是相同的,所以用translate
  tc->translate(new_obj_pos - obj_pos);
}
void PhysicsEngine::resolveSolidObjectCollisions(
    engine::object::GameObject *move_obj,
    engine::object::GameObject *solid_obj) {

  // 进入此函数前,已经检查了各个组件的有效性,因此直接进行计算
  auto *move_tc =
      move_obj->getComponent<engine::component::TransformComponent>();

  auto *move_pc = move_obj->getComponent<engine::component::PhysicsComponent>();
  auto *move_cc =
      move_obj->getComponent<engine::component::ColliderComponent>();

  auto *solid_cc =
      solid_obj->getComponent<engine::component::ColliderComponent>();

  // 这里只能获取期望位置,无法获取当前帧初始位置,因此无法进行轴分离碰撞检测
  auto move_aabb = move_cc->getWorldAABB();
  auto solid_aabb = solid_cc->getWorldAABB();

  // 使用最小平移向量解决碰撞问题, 稳定性不如轴分离的逻辑
  // 简单来讲就是计算重叠区域的宽度和高度,哪个小,就往哪个方向推,把他们推的不重叠
  auto move_center = move_aabb.position + move_aabb.size / 2.0f;
  auto solid_center = solid_aabb.position + solid_aabb.size / 2.0f;

  // 计算两个包围盒重叠部分
  auto overlap = glm::vec2(move_aabb.size / 2.0f + solid_aabb.size / 2.0f) -
                 glm::abs(move_center - solid_center);
  if (overlap.x < 0.1f && overlap.y < 0.1f)
    return; // 重叠部分太小则认为没有碰撞

  if (overlap.x < overlap.y) {
    if (move_center.x < solid_center.x) {
      move_tc->translate(glm::vec2(-overlap.x, 0.0f));
      if (move_pc->getVelocity().x > 0.0f)
        move_pc->velocity_.x = 0.0f; // if判断不可少否则可能出现错误吸附
    } else {
      move_tc->translate(glm::vec2(overlap.x, 0.0f));
      if (move_pc->getVelocity().x < 0.0f)
        move_pc->velocity_.x = 0.0f; // if判断不可少否则可能出现错误吸附
    }
  } else {
    if (move_center.y < solid_center.y) {

      move_tc->translate(glm::vec2(0.0f, -overlap.y));
      if (move_pc->getVelocity().y > 0.0f)
        move_pc->velocity_.y = 0.0f; // if判断不可少否则可能出现错误吸附
    } else {
      move_tc->translate(glm::vec2(0.0f, overlap.y));
      if (move_pc->getVelocity().y < 0.0f)
        move_pc->velocity_.y = 0.0f; // if判断不可少否则可能出现错误吸附
    }
  }
}

float PhysicsEngine::getTileHeightAtWidth(float width,
                                          engine::component::TileType type,
                                          glm::vec2 tile_size) {
  auto rel_x = glm::clamp(width / tile_size.x, 0.0f, 1.0f);
  switch (type) {
  case engine::component::TileType::SLOPE_0_1:
    return rel_x * tile_size.y;
  case engine::component::TileType::SLOPE_0_2:
    return rel_x * tile_size.y * 0.5f;
  case engine::component::TileType::SLOPE_2_1:
    return rel_x * tile_size.y * 0.5f + tile_size.y * 0.5f;
  case engine::component::TileType::SLOPE_1_0:
    return (1.0f - rel_x) * tile_size.y;
  case engine::component::TileType::SLOPE_2_0:
    return (1.0f - rel_x) * tile_size.y * 0.5f;
  case engine::component::TileType::SLOPE_1_2:
    return (1.0f - rel_x) * tile_size.y * 0.5f + tile_size.y * 0.5f;
  default:
    return 0.0f;
  }
}

void PhysicsEngine::applyWorldBounds(engine::component::PhysicsComponent *pc) {
  if (!pc || !world_bounds_)
    return;

  auto *obj = pc->getOwner();
  auto *cc = obj->getComponent<engine::component::ColliderComponent>();
  auto *tc = obj->getComponent<engine::component::TransformComponent>();
  auto world_aabb = cc->getWorldAABB();
  auto obj_pos = world_aabb.position;
  auto obj_size = world_aabb.size;

  if (obj_pos.x < world_bounds_->position.x) {
    pc->velocity_.x = 0.0f;
    obj_pos.x = world_bounds_->position.x;
  }
  if (obj_pos.x + obj_size.x >
      world_bounds_->position.x + world_bounds_->size.x) {
    pc->velocity_.x = 0.0f;
    obj_pos.x = world_bounds_->position.x + world_bounds_->size.x - obj_size.x;
  }
  if (obj_pos.y < world_bounds_->position.y) {
    pc->velocity_.y = 0.0f;
    obj_pos.y = world_bounds_->position.y;
  }

  // 不检测下边界,一般认为掉下去就是死了
  tc->translate(obj_pos - world_aabb.position);
}

} // namespace engine::physics