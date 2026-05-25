#pragma once
#include <nlohmann/json_fwd.hpp> // nolhmann_json提供的前向声明
#include <string>
#include <unordered_map>
#include <vector>

namespace engine::core {
using std::string;
using std::vector;
/**
 * @brief 管理应用程序的配置设置
 *
 * 提供配置项的默认值,并支持从JSON文件加载/保存配置
 * 如果加载失败或文件不存在将使用默认值
 */
class Config final {
public:
  string window_title_ = "SunnyLand";
  int window_width_ = 1280;
  int window_height_ = 720;
  bool window_resizable_ = true;

  // 图形设置
  bool vsync_enabled_ = true; ///< @brief 是否启用垂直同步

  int target_fps_ = 144; ///< @brief 0 表示不限制

  float music_volume_ = 0.5f;
  float sound_volume_ = 0.5f;

  // 存储动作名称到 SDL_Scancode名称列表的映射,
  // 提供一些合理的默认值以防配置文件加载失败或缺少此部分
  std::unordered_map<string, vector<string>> input_mappings_ = {
      {"move_left", {"A", "Left"}}, {"move_right", {"D", "Right"}},
      {"move_up", {"W", "Up"}},     {"move_down", {"S", "Down"}},
      {"jump", {"J", "Space"}},     {"attack", {"K", "MouseLeft"}},
      {"pause", {"P", "Escape"}},
      // 可以添加更多默认动作
  };

public:
  explicit Config(const string &file_path);

  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;
  Config(const Config &&) = delete;
  Config &operator=(const Config &&) = delete;

  bool loadFromFile(const string &file_path);
  [[nodiscard]] bool saveToFile(const string &file_path);

private:
  void fromJson(const nlohmann::json &j);
  nlohmann::ordered_json toJson() const;
};
} // namespace engine::core
