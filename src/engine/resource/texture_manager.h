#pragma once
#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace engine::resource {

using std::string;
/**
 * @brief 管理 SDL_Texture资源的加载,存储和检索
 *
 * 在构造时初始化,使用文件路径作为key,确保纹理只加载一次并正确释放
 * 依赖于一个有效的SDL_Renderer,构造失败会抛出异常
 */
class TextureManager {
  // 友元, private的接口只会暴露给它
  friend class ResourceManager;

private:
  // SDL_Texture 的删除器函数对象,用于智能指针管理
  struct SDLTextureDeleter {
    void operator()(SDL_Texture *texture) const {
      if (texture) {
        SDL_DestroyTexture(texture);
      }
    }
  };

  std::unordered_map<string, std::unique_ptr<SDL_Texture, SDLTextureDeleter>>
      textures_;

  SDL_Renderer *renderer_ = nullptr; // 指向主渲染器的非拥有指针

public:
  explicit TextureManager(SDL_Renderer *renderer);

  TextureManager(const TextureManager &) = delete;
  TextureManager &operator=(const TextureManager &) = delete;
  TextureManager(const TextureManager &&) = delete;
  TextureManager &operator=(const TextureManager &&) = delete;

private:
  SDL_Texture *loadTexture(const string &file_path);
  SDL_Texture *getTexture(const string &file_path);
  glm::vec2 getTextureSize(const string &file_path);
  void unloadTexture(const string &file_path);
  void clearTextures();
};

} // namespace engine::resource
