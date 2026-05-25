#include "texture_manager.h"
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {

TextureManager::TextureManager(SDL_Renderer *renderer) : renderer_(renderer) {
  if (!renderer_) {
    // 关键错误,无法继续,抛出异常,它将由catch语句捕获(位于GameApp),并进行处理,
    // 而且其实构造函数是不能返回值的(
    throw std::runtime_error("TextureManager 构造失败: 渲染器指针为空");
  }

  spdlog::trace("TextureManager 构造成功");
}

SDL_Texture *TextureManager::loadTexture(const string &file_path) {
  auto it = textures_.find(file_path);
  if (it != textures_.end()) {
    return it->second.get();
  }

  SDL_Texture *raw_texture = IMG_LoadTexture(renderer_, file_path.c_str());

  if (!raw_texture) {
    spdlog::error("加载纹理失败: '{}': '{}'", file_path, SDL_GetError());
    return nullptr;
  }

  textures_.emplace(
      file_path, std::unique_ptr<SDL_Texture, SDLTextureDeleter>(raw_texture));
  spdlog::debug("成功加载并缓存纹理: {}", file_path);

  return raw_texture;
}

SDL_Texture *TextureManager::getTexture(const string &file_path) {
  auto it = textures_.find(file_path);
  if (it != textures_.end()) {
    return it->second.get();
  }

  spdlog::warn("纹理 '{}' 未找到缓存, 尝试加载", file_path);
  return loadTexture(file_path);
}

glm::vec2 TextureManager::getTextureSize(const string &file_path) {
  SDL_Texture *texture = getTexture(file_path);
  if (!texture) {
    spdlog::error("无法获取纹理: {}", file_path);
    return glm::vec2(0);
  }

  glm::vec2 size;
  if (!SDL_GetTextureSize(texture, &size.x, &size.y)) {
    spdlog::error("无法查询纹理尺寸: {}", file_path);
    return glm::vec2(0);
  }
  return size;
}

void TextureManager::unloadTexture(const string &file_path) {
  auto it = textures_.find(file_path);
  if (it != textures_.end()) {
    spdlog::debug("卸载纹理: {}", file_path);
    textures_.erase(it); // unique_ptr 通过自定义删除器处理删除
  } else {
    spdlog::warn("尝试卸载不存在的纹理: {}", file_path);
  }
}

void TextureManager::clearTextures() {
  if (!textures_.empty()) {
    spdlog::debug("正在清除所有缓存的纹理, 总数: {}", textures_.size());
    textures_.clear();
  }
}

} // namespace engine::resource