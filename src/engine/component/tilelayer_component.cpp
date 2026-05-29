#include "tilelayer_component.h"
#include "../core/context.h"
#include "../physics/physics_engine.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "../render/sprite.h"
#include <spdlog/spdlog.h>


namespace engine::component {

TileLayerComponent::TileLayerComponent(glm::ivec2 tile_size,
                                       glm::ivec2 map_size,
                                       std::vector<TileInfo> &&tiles)
    : tile_size_(tile_size), map_size_(map_size), tiles_(std::move(tiles)) {
  if (tiles_.size() != static_cast<size_t>(map_size_.x * map_size_.y)) {
    spdlog::error("TileLayerComponent: "
                  "地图尺寸与提供的瓦片向量大小不匹配,瓦片数据将被清除");
    tiles_.clear();
    map_size_ = {0, 0};
  }
  spdlog::trace("TileLayerComponent is constructed");
}

void TileLayerComponent::init() {
  if (!owner_) {
    spdlog::warn("TileLayerComponent's owner_ 未设置");
    spdlog::trace("TileLayerComponent is initialized");
  }
}
void TileLayerComponent::render(engine::core::Context &context) {
  if (is_hidden_ || tile_size_.x <= 0 || tile_size_.y <= 0) {
    return;
  }

  for (int y = 0; y < map_size_.y; y++) {
    for (int x = 0; x < map_size_.x; x++) {
      size_t index = static_cast<size_t>(y) * map_size_.x + x;
      if (index < tiles_.size() && tiles_[index].type != TileType::EMPTY) {
        const auto &tile_info = tiles_[index];
        glm::vec2 tile_left_top_pos = {
            offset_.x + static_cast<float>(x) * tile_size_.x,
            offset_.y + static_cast<float>(y) * tile_size_.y};

        // 左下角转到左上角,因此只需要修改y的偏移而不需要修改x
        if (static_cast<int>(tile_info.sprite.getSourceRect()->h) !=
            tile_size_.y) {
          // 如果图片大小与瓦片大小不一致,则需要调整
          tile_left_top_pos.y -= (tile_info.sprite.getSourceRect()->h -
                                  static_cast<float>(tile_size_.y));
        }

        context.getRenderer().drawSprite(context.getCamera(), tile_info.sprite,
                                         tile_left_top_pos);
      }
    }
  }
}

const TileInfo *TileLayerComponent::getTileInfoAt(glm::ivec2 pos) const {
  if (pos.x < 0 || pos.x >= map_size_.x || pos.y < 0 || pos.y >= map_size_.y) {
    spdlog::warn("无效的map pos: ({},{})", pos.x, pos.y);
    return nullptr;
  }

  size_t index = static_cast<size_t>(pos.y) * map_size_.x + pos.x;
  if (index < tiles_.size()) {

    return &tiles_[index];
  }
  spdlog::warn("无效的 index: {}", index);
  return nullptr;
}

TileType TileLayerComponent::getTileTypeAt(glm::ivec2 pos) const {
  const TileInfo *info = getTileInfoAt(pos);
  return info ? info->type : TileType::EMPTY;
}

TileType
TileLayerComponent::getTileTypeAtWorldPos(const glm::vec2 &world_pos) const {
  glm::vec2 relative_pos = world_pos - offset_;
  int tile_x = static_cast<int>(std::floor(relative_pos.x / tile_size_.x));
  int tile_y = static_cast<int>(std::floor(relative_pos.y / tile_size_.y));
  return getTileTypeAt(glm::ivec2{tile_x, tile_y});
}
void TileLayerComponent::clean() {
  if (physics_engine_) {
    physics_engine_->unregisterCollisionLayer(this);
  }
}

} // namespace engine::component