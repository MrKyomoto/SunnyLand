#include "level_loader.h"
#include "../component/parallax_component.h"
#include "../component/sprite_component.h"
#include "../component/tilelayer_component.h"
#include "../component/transform_component.h"
#include "../core/context.h"
#include "../object/game_object.h"
#include "../render/sprite.h"
#include "../scene/scene.h"
#include <filesystem>
#include <fstream>
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace engine::scene {

bool LevelLoader::loadLevel(const string &map, Scene &scene) {
  map_path_ = map;
  std::ifstream file(map_path_);
  if (!file.is_open()) {
    spdlog::error("Unable to open level file: {}", map_path_);
    return false;
  }

  nlohmann::json json_data;
  try {
    file >> json_data;
  } catch (const nlohmann::json::parse_error &e) {
    spdlog::error("Parsing JSON data failed: {}", e.what());
    return false;
  }

  map_size_ =
      glm::ivec2(json_data.value("width", 0), json_data.value("height", 0));
  tile_size_ = glm::ivec2(json_data.value("tilewidth", 0),
                          json_data.value("tileheight", 0));

  if (json_data.contains("tilesets") && json_data["tilesets"].is_array()) {
    for (const auto &tileset_json : json_data["tilesets"]) {
      if (!tileset_json.contains("source") ||
          !tileset_json["source"].is_string() ||
          !tileset_json.contains("firstgid") ||
          !tileset_json["firstgid"].is_number_integer()) {
        spdlog::error("tilesets 对象中缺少有效 'source' or 'firstgid' 字段");
        continue;
      }
      auto tileset_path = resolvePath(tileset_json["source"], map_path_);
      auto first_gid = tileset_json["firstgid"];
      // 把Tileset存入内存,便于快速读取
      loadTileset(tileset_path, first_gid);
    }
  }

  if (!json_data.contains("layers") || !json_data["layers"].is_array()) {
    spdlog::error("level file '{}' is lack of 'layer' array or its invalid",
                  map_path_);
    return false;
  }
  for (const auto &layer_json : json_data["layers"]) {
    string layer_type = layer_json.value("type", "none");
    if (!layer_json.value("visible", true)) {
      spdlog::info("layer '{}' is invisible, skip it",
                   layer_json.value("name", "Unnamed"));
      continue;
    }

    if (layer_type == "imagelayer") {
      loadImageLayer(layer_json, scene);
    } else if (layer_type == "tilelayer") {
      loadTileLayer(layer_json, scene);
    } else if (layer_type == "objectgroup") {
      loadObjectLayer(layer_json, scene);
    } else {
      spdlog::warn("Unsupported layer type: {}", layer_type);
    }
  }

  spdlog::info("level load successfully: {}", map_path_);
  return true;
}

void LevelLoader::loadTileset(const string &tileset_path, int first_gid) {
  std::ifstream tileset_file(tileset_path);
  if (!tileset_file.is_open()) {
    spdlog::error("Unable to open Tileset file: {}", tileset_path);
    return;
  }

  nlohmann::json ts_json;
  try {
    tileset_file >> ts_json;
  } catch (const nlohmann::json::parse_error &e) {
    spdlog::error("Parsing Tileset JSON file '{}' failed: {} (at byte {})",
                  tileset_path, e.what(), e.byte);
    return;
  }

  ts_json["file_path"] = tileset_path;
  tileset_data_[first_gid] = std::move(ts_json);
  spdlog::info("Tileset file '{}' loaded successfully, firstgid: {}",
               tileset_path, first_gid);
}

void LevelLoader::loadImageLayer(const nlohmann::json &image_json,
                                 Scene &scene) {
  // 获取纹理路径(相对路径,会自动处理'\/'符号)
  const string &image_path = image_json.value("image", "");
  if (image_path.empty()) {
    spdlog::error("Layer '{}' lacks 'image' property",
                  image_json.value("name", "Unnamed"));
    return;
  }

  auto texture_id = resolvePath(image_path, map_path_);

  const glm::vec2 offset = glm::vec2(image_json.value("offsetx", 0.0f),
                                     image_json.value("offsety", 0.0f));

  const glm::vec2 scroll_factor = glm::vec2(
      image_json.value("parallaxx", 1.0f), image_json.value("parallaxy", 1.0f));
  const glm::bvec2 repeat = glm::bvec2(image_json.value("repeatx", false),
                                       image_json.value("repeaty", false));

  const string &image_name = image_json.value("name", "Unnamed");

  auto game_object = std::make_unique<engine::object::GameObject>(image_name);

  game_object->addComponent<engine::component::TransformComponent>(offset);
  game_object->addComponent<engine::component::ParallaxComponent>(
      texture_id, scroll_factor, repeat);

  scene.addGameObject(std::move(game_object));

  spdlog::info("Load Image Layer '{}' successfully", image_name);
}

void LevelLoader::loadTileLayer(const nlohmann::json &tile_json, Scene &scene) {
  if (!tile_json.contains("data") || !tile_json["data"].is_array()) {
    spdlog::error("Layer '{}' lacks 'data' property",
                  tile_json.value("name", "Unnamed"));
    return;
  }

  std::vector<engine::component::TileInfo> tiles;
  tiles.reserve(map_size_.x * map_size_.y);

  const auto &data = tile_json["data"];

  for (const auto &gid : data) {
    tiles.push_back(getTileInfoByGid(gid));
  }

  const string &tile_name = tile_json.value("name", "Unnamed");
  auto game_object = std::make_unique<engine::object::GameObject>(tile_name);
  game_object->addComponent<engine::component::TileLayerComponent>(
      tile_size_, map_size_, std::move(tiles));
  scene.addGameObject(std::move(game_object));
}

void LevelLoader::loadObjectLayer(const nlohmann::json &object_json,
                                  Scene &scene) {
  if (!object_json.contains("objects") || !object_json["objects"].is_array()) {
    spdlog::error("对象图层 '{}' 缺少 'objects' 属性",
                  object_json.value("name", "Unnamed"));
    return;
  }

  const auto &objects = object_json["objects"];
  for (const auto &object : objects) {
    auto gid = object.value("gid", 0);
    if (gid == 0) {
      // TODO: 这是自己绘制的形状, 未来按需处理
    } else {
      auto tile_info = getTileInfoByGid(gid);
      if (tile_info.sprite.getTextureID().empty()) {
        spdlog::error("gid 为 {} 的瓦片没有图像纹理", gid);
        continue;
      }

      auto position =
          glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f));
      auto dst_size =
          glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f));
      // 实际的position需要调整(左下角->左上角)
      position = glm::vec2(position.x, position.y - dst_size.y);

      auto rotation = object.value("rotation", 0.0f);
      auto src_size_opt = tile_info.sprite.getSourceRect();
      if (!src_size_opt) {
        // 正常情况下调用了getTileInfoByGid得到的TileInfo.sprite都设置了源矩形,没有代表某处出错
        spdlog::error("gid 为 {} 的瓦片没有源矩形");
        continue;
      }
      auto src_size = glm::vec2(src_size_opt->w, src_size_opt->h);
      auto scale = dst_size / src_size;

      const string &object_name = object.value("name", "Unnamed");

      auto game_object =
          std::make_unique<engine::object::GameObject>(object_name);
      game_object->addComponent<engine::component::TransformComponent>(
          position, scale, rotation);
      game_object->addComponent<engine::component::SpriteComponent>(
          std::move(tile_info.sprite), scene.getContext().getResourceManager());

      scene.addGameObject(std::move(game_object));
      spdlog::info("加载对象: '{}' 完成", object_name);
    }
  }
}

engine::component::TileInfo LevelLoader::getTileInfoByGid(int gid) {
  if (gid == 0) {
    return engine::component::TileInfo();
  }

  // 1 : tileset.tsj
  // 576: prop.tsj
  // 607: actor.tsj
  // 查找tileset_data_中键大于gid的第一个元素,返回迭代器
  auto tileset_it = tileset_data_.upper_bound(gid);
  if (tileset_it == tileset_data_.begin()) {
    spdlog::error("gid为 {} 的瓦片未找到图块集", gid);
    return engine::component::TileInfo();
  }

  --tileset_it;

  const auto &tileset = tileset_it->second;
  auto local_id = gid - tileset_it->first;
  const string file_path = tileset.value("file_path", "");
  if (file_path.empty()) {
    spdlog::error("Tileset file '{}' lacks 'file_path' property",
                  tileset_it->first);
    return engine::component::TileInfo();
  }

  // 区分两种不同的图块集,单一图片:"image"位于外层,多图片集合:"image"位于tiles数组中的对象中

  if (tileset.contains("image")) {
    auto texture_id =
        resolvePath(tileset["image"].get<std::string>(), file_path);
    // 计算瓦片在图片网格中的坐标
    auto corrdinate_x = local_id % tileset["columns"].get<int>();
    auto corrdinate_y = local_id / tileset["columns"].get<int>();
    SDL_FRect texture_rect = {
        static_cast<float>(corrdinate_x * tile_size_.x),
        static_cast<float>(corrdinate_y * tile_size_.y),
        static_cast<float>(tile_size_.x),
        static_cast<float>(tile_size_.y),
    };
    engine::render::Sprite sprite(texture_id, texture_rect);
    return engine::component::TileInfo(sprite,
                                       engine::component::TileType::NORMAL);
  } else {
    if (!tileset.contains("tiles")) {
      spdlog::error("Tileset file '{}' lacks 'tiles' property",
                    tileset_it->first);
      return engine::component::TileInfo();
    }

    const auto &tiles_json = tileset["tiles"];
    for (const auto &tile_json : tiles_json) {
      auto tile_id = tile_json.value("id", 0);
      if (tile_id == local_id) {
        if (!tile_json.contains("image")) {
          spdlog::error("Tileset file '{}' lacks 'image' property",
                        tileset_it->first, tile_id);
          return engine::component::TileInfo();
        }

        auto texture_id =
            resolvePath(tile_json["image"].get<std::string>(), file_path);
        auto image_width = tile_json.value("imagewidth", 0);
        auto image_height = tile_json.value("imageheight", 0);

        SDL_FRect texture_rect = {
            static_cast<float>(tile_json.value("x", 0)),
            static_cast<float>(tile_json.value("y", 0)),
            static_cast<float>(tile_json.value("width", image_width)),
            static_cast<float>(tile_json.value("height", image_height)),
        };

        engine::render::Sprite sprite(texture_id, texture_rect);
        return engine::component::TileInfo(sprite,
                                           engine::component::TileType::NORMAL);
      }
    }
  }

  // 如果走到这里说明查找失败
  spdlog::error("Tileset '{}' not found tile with gid {}", tileset_it->first,
                gid);
  return engine::component::TileInfo();
}
string LevelLoader::resolvePath(const string &relative_path,
                                const string &file_path) {
  try {
    // 获取地图文件的父目录(相对于可执行文件)
    auto map_dir = std::filesystem::path(file_path).parent_path();
    auto final_path = std::filesystem::canonical(map_dir / relative_path);
    return final_path.string();
  } catch (const std::exception &e) {
    spdlog::error("Resolving path failed: {}", e.what());
    return relative_path;
  }
}

} // namespace engine::scene