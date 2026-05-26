#pragma once
#include "../render/sprite.h"
#include "../utils/alignment.h"
#include "component.h"
#include <SDL3/SDL_rect.h>
#include <glm/vec2.hpp>
#include <optional>
#include <string>

namespace engine::resource {
class ResourceManager;
} // namespace engine::resource

namespace engine::component {
class TransformComponent;

/**
 * @brief 管理 GameObject 的视觉表示, 持有一个Sprite对象
 *
 * 协调 Sprite数据和渲染逻辑,并于TransformComponent交互
 */
class SpriteComponent final : public engine::component::Component {
  friend class engine::object::GameObject;

private:
  engine::resource::ResourceManager *resource_manager_ = nullptr;
  TransformComponent *transform_ = nullptr;

  engine::render::Sprite sprite_;
  engine::utils::Alignment alignment_ =
      engine::utils::Alignment::NONE; // 对齐方式
  glm::vec2 sprite_size_ = {0.0f, 0.0f};
  glm::vec2 offset_ = {0.0f, 0.0f};
  bool is_hidden_ = false;

public:
  SpriteComponent(
      const std::string &texture_id,
      engine::resource::ResourceManager &resource_manager,
      engine::utils::Alignment alignment = engine::utils::Alignment::NONE,
      std::optional<SDL_FRect> source_rect_opt = std::nullopt,
      bool is_flipped = false);

  SpriteComponent(
      engine::render::Sprite &&sprite,
      engine::resource::ResourceManager &resource_manager,
      engine::utils::Alignment alignment = engine::utils::Alignment::NONE);

  ~SpriteComponent() override = default;

  SpriteComponent(const SpriteComponent &) = delete;
  SpriteComponent &operator=(const SpriteComponent &) = delete;
  SpriteComponent(const SpriteComponent &&) = delete;
  SpriteComponent &operator=(const SpriteComponent &&) = delete;

  /// @brief 更新偏移量(根据当前的 alignment_ 和 sprite_size_ 计算 offset_)
  void updateOffset();

  // Getters
  const engine::render::Sprite &getSprite() const { return sprite_; }
  const std::string &getTextureID() const { return sprite_.getTextureID(); }
  bool ifFlipped() const { return sprite_.isFlipped(); }
  bool isHidden() const { return is_hidden_; }
  const glm::vec2 &getSpriteSize() const { return sprite_size_; }
  const glm::vec2 &getOffset() const { return offset_; }
  engine::utils::Alignment getAlignment() const { return alignment_; }

  // Setters
  void
  setSpriteByID(const std::string &texture_id,
                const std::optional<SDL_FRect> &source_rec_opt = std::nullopt);
  void setFlipped(bool flipped) { sprite_.setFlipped(flipped); }
  void setHidden(bool hidden) { is_hidden_ = hidden; }
  void setSourceRect(const std::optional<SDL_FRect> &source_rec_opt);
  void setAlignment(engine::utils::Alignment anchor);

private:
  /// @brief 辅助函数,根据sprite_的source_rect_更新sprite_size_
  void updateSpriteSize();

  void init() override;
  void update(float, engine::core::Context &) override {}
  void render(engine::core::Context &context) override;
};

} // namespace engine::component
