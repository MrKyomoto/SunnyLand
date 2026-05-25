#pragma once
#include <SDL3/SDL_rect.h> // for SDL_FRect
#include <optional>        // for std::optional 表示可选的源矩形
#include <string>

namespace engine::render {

/**
 * @brief 表示要绘制的视觉精灵的数据
 *
 * 包含纹理标识符,要绘制的纹理部分(源矩形)以及反转状态
 * 位置,缩放和旋转由外部(i.e.SpriteComponent)标识
 * 渲染由 Renderer 类完成(传入sprite作为参数)
 */
class Sprite final {
private:
  std::string texture_id_;
  std::optional<SDL_FRect> source_rect_;
  bool is_flipped_ = false;

public:
  explicit Sprite(const std::string &texture_id,
                  const std::optional<SDL_FRect> &source_rect = std::nullopt,
                  bool is_flipped = false)
      : texture_id_(texture_id), source_rect_(source_rect),
        is_flipped_(is_flipped) {}

  const std::string &getTextureID() const { return texture_id_; }
  const std::optional<SDL_FRect> &getSourceRect() const { return source_rect_; }
  bool isFlipped() const { return is_flipped_; }

  void setTextureID(const std::string &texture_id) { texture_id_ = texture_id; }
  void setSourceRect(const std::optional<SDL_FRect> &source_rect) {
    source_rect_ = source_rect;
  }
  void setFlipped(bool flipped) { is_flipped_ = flipped; }
};

} // namespace engine::render
