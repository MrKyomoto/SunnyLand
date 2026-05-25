#pragma once
#include <memory>
#include <string>
#include <vector>

namespace engine::core {
class Context;
}
namespace engine::render {
class Renderer;
class Camera;
} // namespace engine::render
namespace engine::input {
class InputManager;
}
namespace engine::object {
class GameObject;
}
namespace engine::scene {
using std::string;
using std::vector;
class SceneManager;
class Scene {
protected:
  string scene_name_;
  engine::core::Context &context_;             ///< @brief 隐式，构造时传入
  engine::scene::SceneManager &scene_manager_; ///< @brief 构造时传入
  /// @brief 非当前场景很可能未被删除，因此需要初始化标志避免重复初始化
  bool is_initialized_ = false;
  vector<std::unique_ptr<engine::object::GameObject>>
      game_objects_; ///< @brief 场景中的游戏对象
  vector<std::unique_ptr<engine::object::GameObject>>
      pending_additions_; ///< @brief 待添加的游戏对象（延迟添加）

public:
  Scene(const string &scene_name, engine::core::Context &context,
        engine::scene::SceneManager &scene_manager);
  ///< @brief
  ///< 基类必须声明虚析构函数才能让派生类析构函数被正确调用，析构函数定义必须写在cpp中，不然需要引入GameObject头文件
  virtual ~Scene();

  Scene(Scene &&) = delete;
  Scene(const Scene &) = delete;
  Scene &operator=(Scene &&) = delete;
  Scene &operator=(const Scene &) = delete;

  virtual void init();
  virtual void update(float delta_time);
  virtual void render();
  virtual void handleInput();
  virtual void clean();

  /// @brief
  /// 直接向游戏场景添加一个游戏对象（初始化时可用，游戏进行中不安全），&&表示右值引用，与std::move配合使用避免拷贝
  virtual void
  addGameObject(std::unique_ptr<engine::object::GameObject> &&game_object);
  virtual void
  safeAddGameObject(std::unique_ptr<engine::object::GameObject> &&game_object);
  /// @brief 一般不使用但保留逻辑
  virtual void removeGameObject(engine::object::GameObject *game_object_ptr);
  virtual void
  safeRemoveGameObject(engine::object::GameObject *game_object_ptr);

  const vector<std::unique_ptr<engine::object::GameObject>> &
  getGameObjects() const {
    return game_objects_;
  }

  engine::object::GameObject *findGameObjectByName(const string &name) const;

  // Getters && Setters
  void setName(const string &name) { scene_name_ = name; }
  const string &getName() const { return scene_name_; }
  void setInitialized(bool initialized) { is_initialized_ = initialized; }
  bool isInitialized() const { return is_initialized_; }

  engine::core::Context &getContext() const { return context_; }
  engine::scene::SceneManager &getSceneManager() const {
    return scene_manager_;
  }

protected:
  void processPendingAdditions();
};
} // namespace engine::scene
