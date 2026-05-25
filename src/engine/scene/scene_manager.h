#pragma once
#include "scene.h"
#include <memory>
#include <string>
#include <vector>
namespace engine::core {
class Context;
}
namespace engine::scene {
using std::string;
using std::vector;
class Scene;

class SceneManager final {
private:
  engine::core::Context &context_;
  vector<std::unique_ptr<Scene>> scene_stack_;

  enum class PendingAction { None, Push, Pop, Replace };
  PendingAction pending_action_ = PendingAction::None;
  std::unique_ptr<Scene> pending_scene_;

public:
  explicit SceneManager(engine::core::Context &context);
  ~SceneManager();

  SceneManager(SceneManager &&) = delete;
  SceneManager(const SceneManager &) = delete;
  SceneManager &operator=(SceneManager &&) = delete;
  SceneManager &operator=(const SceneManager &) = delete;

  // 延迟切换场景
  void requestPushScene(
      std::unique_ptr<Scene> &&scene); ///< @brief 请求压入一个新场景
  void requestPopScene();             ///< @brief pop current scene
  void requestReplaceScene(
      std::unique_ptr<Scene> &&scene); ///< @brief replace current scene

  // Getters
  Scene *getCurrentScene() const;
  engine::core::Context &getContext() const { return context_; }

  void update(float delta_time);
  void render();
  void handleInput();
  void close();

private:
  /// @brief 处理挂起的场景操作（每轮更新最后调用）
  void processPendingActions();
  void pushScene(std::unique_ptr<Scene> &&scene);
  void popScene();
  void replaceScene(std::unique_ptr<Scene> &&scene);
};
} // namespace engine::scene
