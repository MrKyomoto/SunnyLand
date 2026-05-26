#pragma once
#include "../render/sprite.h"
#include "component.h"
#include <glm/vec2.hpp>
#include <vector>

namespace engine::render {
class Sprite;
}

namespace engine::core {
class Context;
}

namespace engine::component {
/**
 * @brief 定义瓦片类型,用于游戏逻辑(i.e. 碰撞)
 */
enum class TileType {
  EMPTY,  // 空白瓦片
  NORMAL, // 普通瓦片
  SOLID,  // 静止可碰撞瓦片
  //...
};

/**
 * @brief 包含单个瓦片的渲染和逻辑信息
 */
struct TileInfo {
  render::Sprite sprite;
  TileType type;
  TileInfo(render::Sprite s = render::Sprite(), TileType t = TileType::EMPTY)
      : sprite(std::move(s)), type(t) {}
};

class TileLayerComponent final : public Component {
  friend class engine::object::GameObject;

private:
  glm::ivec2 tile_size_; // 瓦片尺寸(像素)
  glm::ivec2 map_size_;  // 地图尺寸(瓦片数量)
  // index = y * map_width_ + x
  std::vector<TileInfo> tiles_;
  ///< @brief
  ///< 瓦片层在世界中的偏移量,瓦片通常不需要缩放以及旋转,因此不引入Transform组件,offset_最好也保持默认的0以免增加不必要的复杂性
  glm::vec2 offset_ = {0.0f, 0.0f};
  bool is_hidden_ = false;

public:
  TileLayerComponent() = default;

  TileLayerComponent(glm::ivec2 tile_size, glm::ivec2 map_size,
                     std::vector<TileInfo> &&tiles);

  /// @brief 根据坐标获取瓦片信息
  /// @param 瓦片坐标需要判断是否有效,无效则返回nullptr
  const TileInfo *getTileInfoAt(glm::ivec2 pos) const;
  // 针对地图坐标大小
  // 无效则返回TileType::EMPTY
  TileType getTileTypeAt(glm::ivec2 pos) const;
  // 针对地图像素大小
  // 无效则返回TileType::EMPTY
  TileType getTileTypeAtWorldPos(const glm::vec2 &world_pos) const;

  // Getters && Setters
  glm::ivec2 getTileSize() const { return tile_size_; }
  glm::ivec2 getMapSize() const { return map_size_; }
  glm::vec2 getWorldSize() const {
    return glm::vec2(map_size_.x * tile_size_.x, map_size_.y * tile_size_.y);
  }
  const std::vector<TileInfo> &getTiles() const { return tiles_; }
  const glm::vec2 &getOffset() const { return offset_; }
  bool isHidden() const { return is_hidden_; }

  void setOffset(const glm::vec2 &offset) { offset_ = offset; }
  void setHidden(bool hidden) { is_hidden_ = hidden; }

protected:
  void init() override;
  void update(float, engine::core::Context &) override {}
  void render(engine::core::Context &context) override;
};

} // namespace engine::component