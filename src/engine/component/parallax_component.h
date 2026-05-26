#pragma once
#include "../render/sprite.h"
#include "component.h"
#include "transform_component.h"
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <string>

namespace engine::component {
class TransformComponent;

/**
 * @brief 在背景中渲染可滚动纹理的组件，以创建视差效果
 *
 * 该组件根据相机的位置和滚动因子来移动纹理
 */

class ParallaxComponent final : public Component {
  friend class engine::object::GameObject;

private:
  // cache transform component
  TransformComponent *transform_ = nullptr;

  engine::render::Sprite sprite_;
  // == 0 -> stand still; == 1 -> move with camera; < 1 -> move slower than
  // camera
  glm::vec2 scroll_factor_;
  glm::bvec2 repeat_;
  bool is_hidden_ = false;

public:
  ParallaxComponent(const std::string &texture_id,
                    const glm::vec2 &scroll_factor, glm::bvec2 &repeat);

  ParallaxComponent(ParallaxComponent &&) = delete;
  ParallaxComponent(const ParallaxComponent &) = delete;
  ParallaxComponent &operator=(ParallaxComponent &&) = delete;
  ParallaxComponent &operator=(const ParallaxComponent &) = delete;

  // Setter && Getter
  void setSprite(const engine::render::Sprite &sprite) { sprite_ = sprite; }
  void setScrollFactor(const glm::vec2 &factor) { scroll_factor_ = factor; }
  void setRepeat(const glm::bvec2 &repeat) { repeat_ = repeat; }
  void setHidden(bool hidden) { is_hidden_ = hidden; }

  const engine::render::Sprite &getSprite() const { return sprite_; }
  const glm::vec2 &getScrollFactor() const { return scroll_factor_; }
  const glm::bvec2 &getRepeat() const { return repeat_; }
  bool isHidden() const { return is_hidden_; }

protected:
  // update 是必须实现的虚函数，留空
  void update(float, engine::core::Context &) override {}
  void init() override;
  void render(engine::core::Context &context) override;
};

} // namespace engine::component
