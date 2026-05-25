#include "renderer.h"
#include "../resource/resource_manager.h"
#include "camera.h"
#include "sprite.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <stdexcept>


namespace engine::render {
Renderer::Renderer(SDL_Renderer *sdl_renderer,
                   engine::resource::ResourceManager *resource_manager)
    : renderer_(sdl_renderer), resource_manager_(resource_manager) {
  spdlog::trace("构造 Renderer...");
  if (!renderer_) {
    throw std::runtime_error("Renderer 构造失败: 提供的SDL_Renderer指针为空");
  }
  if (!resource_manager_) {
    throw std::runtime_error(
        "Renderer 构造失败: 提供的ResourceManager指针为空");
  }

  setDrawColor(0, 0, 0, 255);
  spdlog::trace("Renderer 构造成功");
}
void Renderer::drawSprite(const Camera &camera, const Sprite &sprite,
                          const glm::vec2 &position, const glm::vec2 &scale,
                          double angle) {
  auto texture = resource_manager_->getTexture(sprite.getTextureID());
  if (!texture) {
    spdlog::error("无法为 ID {} 获取纹理", sprite.getTextureID());
    return;
  }

  auto src_rect = getSpriteSrcRect(sprite);
  if (!src_rect.has_value()) {
    spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureID());
    return;
  }

  glm::vec2 position_screen = camera.worldToScreen(position);

  float scaled_w = src_rect.value().w * scale.x;
  float scaled_h = src_rect.value().h * scale.y;
  SDL_FRect dest_rect = {position_screen.x, position_screen.y, scaled_w,
                         scaled_h};

  if (!isRectInViewport(camera, dest_rect)) {

    spdlog::info("精灵超出视口范围: ID: {}", sprite.getTextureID());
    return;
  }

  if (!SDL_RenderTextureRotated(
          renderer_, texture, &src_rect.value(), &dest_rect, angle, NULL,
          sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
    spdlog::error("渲染旋转纹理失败 (ID: {} : {})", sprite.getTextureID(),
                  SDL_GetError());
  }
}
void Renderer::drawParallax(const Camera &camera, const Sprite &sprite,
                            const glm::vec2 &position,
                            const glm::vec2 &scroll_factor,
                            const glm::bvec2 &repeat, const glm::vec2 &scale) {
  auto texture = resource_manager_->getTexture(sprite.getTextureID());
  if (!texture) {
    spdlog::error("无法为 ID {} 获取纹理", sprite.getTextureID());
    return;
  }

  auto src_rect = getSpriteSrcRect(sprite);
  if (!src_rect.has_value()) {
    spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureID());
    return;
  }

  glm::vec2 position_screen =
      camera.worldToScreenWithParallax(position, scroll_factor);

  float scaled_w = src_rect.value().w * scale.x;
  float scaled_h = src_rect.value().h * scale.y;

  glm::vec2 start, stop;
  glm::vec2 viewport_size = camera.getViewPortSize();

  if (repeat.x) {
    start.x = glm::mod(position_screen.x, scaled_w) - scaled_w;
    stop.x = viewport_size.x;
  } else {
    start.x = position_screen.x;
    stop.x = glm::min(position_screen.x + scaled_w, viewport_size.x);
  }

  if (repeat.y) {
    start.y = glm::mod(position_screen.y, scaled_h) - scaled_h;
    stop.y = viewport_size.y;
  } else {
    start.y = position_screen.y;
    stop.y = glm::min(position_screen.y + scaled_h, viewport_size.y);
  }

  for (float y = start.y; y < stop.y; y += scaled_h) {

    for (float x = start.x; x < stop.x; x += scaled_w) {
      SDL_FRect dest_rect = {x, y, scaled_w, scaled_h};
      if (!SDL_RenderTexture(renderer_, texture, nullptr, &dest_rect)) {
        spdlog::error("渲染视差纹理失败(ID: {}: {})", sprite.getTextureID(),
                      SDL_GetError());
        return;
      }
    }
  }
}
void Renderer::drawUISprite(const Sprite &sprite, const glm::vec2 &position,
                            const std::optional<glm::vec2> &size) {
  auto texture = resource_manager_->getTexture(sprite.getTextureID());
  if (!texture) {
    spdlog::error("无法为 ID {} 获取纹理", sprite.getTextureID());
    return;
  }

  auto src_rect = getSpriteSrcRect(sprite);
  if (!src_rect.has_value()) {
    spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureID());
    return;
  }

  SDL_FRect dest_rect;
  dest_rect.x = position.x;
  dest_rect.y = position.y;
  if (size.has_value()) {
    dest_rect.w = size.value().x;
    dest_rect.h = size.value().y;
  } else {
    dest_rect.w = src_rect.value().w;
    dest_rect.h = src_rect.value().h;
  }

  if (!SDL_RenderTexture(renderer_, texture, nullptr, &dest_rect)) {
    spdlog::error("渲染视差纹理失败(ID: {}: {})", sprite.getTextureID(),
                  SDL_GetError());
    return;
  }
}
void Renderer::present() { SDL_RenderPresent(renderer_); }
void Renderer::clearScreen() {
  if (!SDL_RenderClear(renderer_)) {
    spdlog::error("清除渲染器失败: {}", SDL_GetError());
  }
}
void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  if (!SDL_SetRenderDrawColor(renderer_, r, g, b, a)) {
    spdlog::error("设置渲染绘制颜色失败: {}", SDL_GetError());
  }
}
void Renderer::setDrawColorFloat(float r, float g, float b, float a) {
  if (!SDL_SetRenderDrawColorFloat(renderer_, r, g, b, a)) {
    spdlog::error("设置渲染绘制颜色失败: {}", SDL_GetError());
  }
}
std::optional<SDL_FRect> Renderer::getSpriteSrcRect(const Sprite &sprite) {
  SDL_Texture *texture = resource_manager_->getTexture(sprite.getTextureID());
  if (!texture) {
    spdlog::error("无法为 ID {} 获取纹理", sprite.getTextureID());
    return std::nullopt;
  }

  auto src_rect = sprite.getSourceRect();
  if (src_rect.has_value()) {
    if (src_rect.value().w <= 0 || src_rect.value().h <= 0) {
      spdlog::error("源矩形尺寸无效, ID: {}", sprite.getTextureID());
      return std::nullopt;
    }
    return src_rect;
  } else {
    SDL_FRect result = {0, 0, 0, 0};
    if (!SDL_GetTextureSize(texture, &result.w, &result.h)) {
      spdlog::error("无法获取纹理尺寸, ID: {}", sprite.getTextureID());
      return std::nullopt;
    }
    return result;
  }
}
bool Renderer::isRectInViewport(const Camera &camera, const SDL_FRect &rect) {
  glm::vec2 viewport_size = camera.getViewPortSize();
  bool result = rect.x + rect.w >= 0 && rect.x <= viewport_size.x &&
                rect.y + rect.h >= 0 && rect.y <= viewport_size.y;
  return result;
}
} // namespace engine::render