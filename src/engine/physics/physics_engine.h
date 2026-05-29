#pragma once
#include "../utils/math.h"
#include <glm/vec2.hpp>
#include <memory>
#include <optional>
#include <utility> // for std::pair
#include <vector>

namespace engine::component {
class PhysicsComponent;
class TileLayerComponent;
enum class TileType;
} // namespace engine::component

namespace engine::object {
class GameObject;
}

namespace engine::physics {
class PhysicsEngine {
private:
  std::vector<engine::component::PhysicsComponent *> components_;
  std::vector<engine::component::TileLayerComponent *> collision_tile_layers_;
  // 默认值 (px/s^2,相当于100px对应现实里1m)
  glm::vec2 gravity_ = {0.0f, 980.0f};
  float max_speed_ = 500.0f;

  std::optional<engine::utils::Rect> world_bounds_;

  // @brief 储存本帧发声的 GameObject 碰撞对 (每次 update 开始时清空)
  std::vector<
      std::pair<engine::object::GameObject *, engine::object::GameObject *>>
      collision_pairs_;

public:
  PhysicsEngine() = default;

  PhysicsEngine(const PhysicsEngine &) = delete;
  PhysicsEngine &operator=(const PhysicsEngine &) = delete;
  PhysicsEngine(const PhysicsEngine &&) = delete;
  PhysicsEngine &operator=(const PhysicsEngine &&) = delete;

  void registerComponent(engine::component::PhysicsComponent *component);
  void unregisterComponent(engine::component::PhysicsComponent *component);

  void registerCollisionLayer(engine::component::TileLayerComponent *layer);
  void unregisterCollisionLayer(engine::component::TileLayerComponent *layer);

  void update(float delta_time);

  void setGravity(const glm::vec2 &gravity) { gravity_ = gravity; }
  const glm::vec2 &getGravity() const { return gravity_; }
  void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }
  float getMaxSpeed() const { return max_speed_; }

  void setWorldBounds(const engine::utils::Rect &world_bounds) {
    world_bounds_ = world_bounds;
  }
  const std::optional<engine::utils::Rect> &getWorldBounds() const {
    return world_bounds_;
  }

  const std::vector<
      std::pair<engine::object::GameObject *, engine::object::GameObject *>> &
  getCollisionPairs() const {
    return collision_pairs_;
  }

private:
  void checkObjectCollisions();
  void resolveTileCollisions(engine::component::PhysicsComponent *pc,
                             float delta_time);

  // 处理可移动物体与SOLID物体的碰撞
  void resolveSolidObjectCollisions(engine::object::GameObject *move_obj,
                                    engine::object::GameObject *solid_obj);
  void applyWorldBounds(engine::component::PhysicsComponent *pc);

  /**
   * @brief 根据瓦片类型和指定宽度x坐标计算瓦片上对应的y坐标
   * @param width 从瓦片左侧算起的宽度
   * @param type 瓦片类型
   * @param tile_size 瓦片尺寸
   * @return 瓦片上对应高度(从瓦片下侧起算)
   */
  float getTileHeightAtWidth(float width, engine::component::TileType type,
                             glm::vec2 tile_size);
};

} // namespace engine::physics