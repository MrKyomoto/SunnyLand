#include "scene.h"
#include "../core/context.h"
#include "../input/input_manager.h"
#include "../object/game_object.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "../physics/physics_engine.h"
#include <algorithm>
#include <memory>
#include <spdlog/spdlog.h>
#include <utility>

namespace engine::scene {

Scene::Scene(const string &scene_name, engine::core::Context &context,
             engine::scene::SceneManager &scene_manager)
    : scene_name_(scene_name), context_(context), scene_manager_(scene_manager),
      is_initialized_(false) {
  spdlog::trace("Scene '{}' is constructed", scene_name_);
}
Scene::~Scene() = default;

void Scene::init() {
  // 子类应该最后调用父类的init方法
  is_initialized_ = true;
  spdlog::trace("Scene '{}' is initialized", scene_name_);
}

void Scene::update(float delta_time) {
  if (!is_initialized_)
    return;

  context_.getPhysicsEngine().update(delta_time);
  context_.getCamera().update(delta_time);

  for (auto it = game_objects_.begin(); it != game_objects_.end();) {
    if (*it && !(*it)->isNeedRemove()) {
      (*it)->update(delta_time, context_);
      it++;
    } else {
      if (*it) {
        (*it)->clean();
      }
      it = game_objects_.erase(it);
    }
  }
  processPendingAdditions();
}
void Scene::render() {
  if (!is_initialized_)
    return;

  for (const auto &obj : game_objects_) {
    if (obj)
      obj->render(context_);
  }
}

void Scene::handleInput() {
  if (!is_initialized_)
    return;
  for (const auto &obj : game_objects_) {
    if (obj)
      obj->handleInput(context_);
  }
}
void Scene::clean() {
  if (!is_initialized_)
    return;
  for (const auto &obj : game_objects_) {
    if (obj)
      obj->clean();
  }
  game_objects_.clear();

  is_initialized_ = false;
  spdlog::trace("Scene '{}' clean finished", scene_name_);
}
void Scene::addGameObject(
    std::unique_ptr<engine::object::GameObject> &&game_object) {
  if (game_object)
    game_objects_.push_back(std::move(game_object));
  else
    spdlog::warn("Trying to add an EMPTY game object to scene '{}'",
                 scene_name_);
}

void Scene::safeAddGameObject(
    std::unique_ptr<engine::object::GameObject> &&game_object) {
  if (game_object)
    pending_additions_.push_back(std::move(game_object));
  else
    spdlog::warn("Trying to add an EMPTY game object to scene '{}'",
                 scene_name_);
}

void Scene::removeGameObject(engine::object::GameObject *game_object_ptr) {
  if (!game_object_ptr) {
    spdlog::warn(
        "Trying to remove an EMPTY game object pointer from scene '{}'",
        scene_name_);
    return;
  }

  // erase-remove
  // 不可用，因为裸指针无法与智能指针进行比较，这里使用remove_if并提供自定义比较方法
  // erase-remove 原理：
  // 实际上是把所有和child相等的n个元素移动到容器末尾然后删除这n个元素
  auto it = std::remove_if(
      game_objects_.begin(), game_objects_.end(),
      [game_object_ptr](const std::unique_ptr<engine::object::GameObject> &p) {
        return p.get() ==
               game_object_ptr; // compare raw pointer with unique_ptr.get()
      });
  if (it != game_objects_.end()) {
    // 因为传入的是指针，所以只可能有一个元素被移除，所以不需要遍历到末尾
    (*it)->clean();
    // remove from it to end()
    game_objects_.erase(it, game_objects_.end());
    spdlog::trace("Remove game object from scene '{}'", scene_name_);
  } else {
    spdlog::warn("Game object pointer NOT FOUND in scene '{}'", scene_name_);
  }
}
void Scene::safeRemoveGameObject(engine::object::GameObject *game_object_ptr) {
  game_object_ptr->setNeedRemove(true);
}

engine::object::GameObject *
Scene::findGameObjectByName(const string &name) const {
  for (const auto &obj : game_objects_) {
    if (obj && obj->getName() == name) {
      return obj.get();
    }
  }
  return nullptr;
}
void Scene::processPendingAdditions() {
  for (auto &ganme_object : pending_additions_) {
    addGameObject(std::move(ganme_object));
  }
  pending_additions_.clear();
}
} // namespace engine::scene
