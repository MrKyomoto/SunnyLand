#include "level_loader.h"
#include "../component/parallax_component.h"
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

void LevelLoader::loadImageLayer(const nlohmann::json &image_json,
                                 Scene &scene) {
  // 获取纹理路径(相对路径,会自动处理'\/'符号)
  const string &image_path = image_json.value("image", "");
  if (image_path.empty()) {
    spdlog::error("Layer '{}' lacks 'image' property",
                  image_json.value("name", "Unnamed"));
    return;
  }

  auto texture_id = resolvePath(image_path);

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

void LevelLoader::loadTileLayer(const nlohmann::json &tile_json, Scene &) {
  // TODO
}

void LevelLoader::loadObjectLayer(const nlohmann::json &object_json, Scene &) {
  // TODO
}

string LevelLoader::resolvePath(string image_path) {
  try {
    // 获取地图文件的父目录(相对于可执行文件)
    auto map_dir = std::filesystem::path(map_path_).parent_path();
    auto final_path = std::filesystem::canonical(map_dir / image_path);
    return final_path.string();
  } catch (const std::exception &e) {
    spdlog::error("Resolving path failed: {}", e.what());
    return image_path;
  }
}

} // namespace engine::scene