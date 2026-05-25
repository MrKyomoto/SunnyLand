#pragma once
#include <functional> // std::hash
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility> // std::pair


#include <SDL3_ttf/SDL_ttf.h>

namespace engine::resource {
using FontKey = std::pair<std::string, int>;
using std::string;

// FontKey的自定义哈希函数, 用于std::unordered_map
struct FontKeyHash {
  std::size_t operator()(const FontKey &key) const {
    std::hash<std::string> string_hasher;
    std::hash<int> int_hasher;
    return string_hasher(key.first) ^ int_hasher(key.second);
  }
};

/**
 * @brief 管理 SDL_ttf 字体资源 (TTF_Font)
 *
 * 提供字体的加载和缓存功能,通过文件路径和点大小来标识
 * 构造失败会抛出异常,仅供ResourceManager内部使用
 */
class FontManager {
  friend class ResourceManager;

private:
  struct SDLFontDeleter {
    void operator()(TTF_Font *font) const {
      if (font) {
        TTF_CloseFont(font);
      }
    }
  };

  // 字体存储 FontKey -> TTF_Font
  // unordered map的键需要能转换为哈希值,对于基础数据类型系统会自动转换
  // 但是对于自定义类型无法自动转换,需要提供自定义哈希函数(第三个模板参数)
  std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>,
                     FontKeyHash>
      fonts_;

public:
  FontManager();
  ~FontManager();

  FontManager(const FontManager &) = delete;
  FontManager &operator=(const FontManager &) = delete;
  FontManager(const FontManager &&) = delete;
  FontManager &operator=(const FontManager &&) = delete;

private:
  TTF_Font *loadFont(const string &file_path, int point_size);
  TTF_Font *getFont(const string &file_path, int point_size);
  void unloadFont(const string &file_path, int point_size);
  void clearFonts();
};

} // namespace engine::resource