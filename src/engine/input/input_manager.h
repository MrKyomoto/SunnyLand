#pragma once
#include <SDL3/SDL_render.h>
#include <glm/vec2.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace engine::core {
class Config;
} // namespace engine::core

namespace engine::input {
using std::string;
using std::unordered_map;
using std::variant;
using std::vector;
enum class ActionState {
  INACTIVE, ///< @brief 动作未激活
  PRESSED_THIS_FRAME,
  HELD_DOWN,
  RELEASED_THIS_FRAME,
};

class InputManager final {
private:
  SDL_Renderer *sdl_renderer_;
  unordered_map<string, vector<string>> actions_to_keyname_map_;
  unordered_map<variant<SDL_Scancode, Uint32>, vector<string>>
      input_to_actions_map_;

  unordered_map<string, ActionState> action_states_;

  bool should_quit_ = false;
  /// @brief 鼠标位置(针对屏幕坐标)
  glm::vec2 mouse_position_;

public:
  /**
   * @brief 构造函数
   * @throws std::runtime_error 如果任意指针为nullptr
   */
  InputManager(SDL_Renderer *sdl_renderer, const engine::core::Config *config);

  void update();

  /// @brief 动作当前是否触发(持续按下或本帧按下)
  bool isActionDown(const string &action_name) const;
  bool isActionPressed(const string &action_name) const;
  bool isActionReleased(const string &action_name) const;

  bool shouldQuit() const;
  void setShouldQuit(bool should_quit);

  /// @brief 获取鼠标位置(屏幕坐标)
  glm::vec2 getMousePosition() const;
  /// @brief 获取鼠标位置(逻辑坐标)
  glm::vec2 getLogicMousePosition() const;

private:
  /// @brief 处理 SDL 事件,将按键转换为动作状态
  void processEvent(const SDL_Event &event);
  /// @brief 根据Config初始化映射表
  void initializeMappings(const engine::core::Config *config);
  /// @brief 辅助更新动作状态
  void updateActionState(const string &action_name, bool is_input_active,
                         bool is_repeat_event);
  SDL_Scancode scancodeFromString(const string &key_name);
  Uint32 mouseButtonUint8FromString(const string &button_name);
};

} // namespace engine::input
