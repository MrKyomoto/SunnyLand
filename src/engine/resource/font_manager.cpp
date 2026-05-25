#include "font_manager.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {
FontManager::FontManager() {
  if (!TTF_WasInit() && !TTF_Init()) {
    throw std::runtime_error("FontManager 构造失败: TTF_Init 失败" +
                             string(SDL_GetError()));
  }
  spdlog::trace("FontManager 构造成功");
}
FontManager::~FontManager() {
  clearFonts();
  TTF_Quit();
  spdlog::trace("FontManager 析构成功");
}
TTF_Font *FontManager::loadFont(const string &file_path, int point_size) {
  if (point_size <= 0) {
    spdlog::error("无法加载字体 '{}': 无效的点大小 {}", file_path, point_size);
    return nullptr;
  }

  FontKey key = {file_path, point_size};
  auto it = fonts_.find(key);
  if (it != fonts_.end()) {
    return it->second.get();
  }

  spdlog::debug("正在加载字体: {} ({}pt)", file_path, point_size);
  TTF_Font *raw_font =
      TTF_OpenFont(file_path.c_str(), static_cast<float>(point_size));
  if (!raw_font) {
    spdlog::error("加载字体 '{}' ({}pt) 失败: {}", file_path, point_size,
                  SDL_GetError());
    return nullptr;
  }

  fonts_.emplace(key, std::unique_ptr<TTF_Font, SDLFontDeleter>(raw_font));
  spdlog::debug("成功加载并缓存字体: {} ({}pt)", file_path, point_size);
  return raw_font;
}
TTF_Font *FontManager::getFont(const string &file_path, int point_size) {
  FontKey key = {file_path, point_size};
  auto it = fonts_.find(key);
  if (it != fonts_.end()) {
    return it->second.get();
  }

  spdlog::warn("字体 '{}' ({}pt) 不在缓存中,尝试加载", file_path, point_size);
  return loadFont(file_path, point_size);
}
void FontManager::unloadFont(const string &file_path, int point_size) {
  FontKey key = {file_path, point_size};
  auto it = fonts_.find(key);
  if (it != fonts_.end()) {
    spdlog::debug("卸载字体: {} ({}pt)", file_path, point_size);
    fonts_.erase(it);
  } else {
    spdlog::warn("尝试卸载不存在字体: {} ({}pt)", file_path, point_size);
  }
}
void FontManager::clearFonts() {
  if (!fonts_.empty()) {
    spdlog::debug("正在清理所有缓存的字体, 数量: {}", fonts_.size());
    fonts_.clear();
  }
}
} // namespace engine::resource