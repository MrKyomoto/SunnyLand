#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <glm/vec2.hpp>
#include <map>

namespace engine::component {
struct TileInfo;
enum class TileType;
}

namespace engine::scene {
using std::string;
class Scene;

class LevelLoader final {
  string map_path_;
  glm::ivec2 map_size_;
  glm::ivec2 tile_size_;
  std::map<int, nlohmann::json> tileset_data_; ///< @brief firstgid

public:
  LevelLoader() = default;

  /// @brief 加载关卡数据到指定的 Scene 对象中
  bool loadLevel(const string &map, Scene &scene);

private:
  void loadImageLayer(const nlohmann::json &image_json, Scene &scene);
  void loadTileLayer(const nlohmann::json &tile_json, Scene &scene);
  void loadObjectLayer(const nlohmann::json &object_json, Scene &scene);

  engine::component::TileType getTileType(const nlohmann::json& tile_json);
  engine::component::TileType getTileTypeByID(const nlohmann::json& tileset_json,int local_id);
  /**
  * @brief 根据全局 ID 获取瓦片信息
  */
  engine::component::TileInfo getTileInfoByGid(int gid);

  void loadTileset(const string& tileset_path,int first_gid);

  /// @brief 解析图片路径，合并地图路径和相对路径，例如：
  /// 1. 地图路径： "assets/maps/level1.tmj"
  /// 2. 相对路径： "../textures/Layers/back.png"
  /// 3. 最终路径： "assets/textures/Layers/back.png"
  string resolvePath(const string& relative_path, const string& file_path);
};

} // namespace engine::scene
