#include "parallax_component.h"
#include "../core/context.h"
#include "../object/game_object.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "../render/sprite.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::component {

ParallaxComponent::ParallaxComponent(const std::string &texture_id,
                                     const glm::vec2 &scroll_factor,
                                     const glm::bvec2 &repeat)
    : sprite_(engine::render::Sprite(texture_id)),
      scroll_factor_(scroll_factor), repeat_(repeat) {
  spdlog::trace("ParallaxComponent is constructed, texture id: {}", texture_id);
}
void ParallaxComponent::init() {
  if (!owner_) {
    spdlog::error("ParallaxComponent 在初始化前未设置所有者");
    return;
  }
  transform_ = owner_->getComponent<TransformComponent>();
  if (!transform_) {
    spdlog::warn("GameObject '{}' 上的 ParallaxComponent 需要一个 "
                 "TransformComponent, 但未找到",
                 owner_->getName());
    return;
  }
}

void ParallaxComponent::render(engine::core::Context &context) {
  if (is_hidden_ || !transform_) {
    return;
  }

  // 获取变换信息(考虑偏移量)
  const glm::vec2 &pos = transform_->getPosition();

  context.getRenderer().drawParallax(context.getCamera(), sprite_, pos,
                                     scroll_factor_, repeat_);
}
} // namespace engine::component
