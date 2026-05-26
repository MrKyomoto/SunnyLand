#pragma once
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace engine::scene {
using std::string;
class Scene;

class LevelLoader final {
  string map_path_;
public:
  LevelLoader() = default;

  /// @brief 加载关卡数据到指定的 Scene 对象中
  bool loadLevel(const string& map, Scene& scene);
private:
  void loadImagerLayer(const nlohmann::json& layer_json,Scene& scene);
  void loadTileLayer(const nlohmann::json& tile_json,Scene& scene);
  void loadObjectLayer(const nlohmann::json& object_json,Scene& scene);

  /// @brief 解析图片路径，合并地图路径和相对路径，例如：
  /// 1. 地图路径： "assets/maps/level1.tmj"
  /// 2. 相对路径： "../textures/Layers/back.png"
  /// 3. 最终路径： "asstes/textures/Layers/back.png"
  string resolvePath(string image_path);
};

} // namespace engine::scene
