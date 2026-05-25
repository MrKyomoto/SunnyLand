#include "scene_manager.h"
#include "../core/context.h"
#include "scene.h"
#include <spdlog/spdlog.h>
#include <utility>

namespace engine::scene {
SceneManager::SceneManager(engine::core::Context &context) : context_(context) {
  spdlog::trace("SceneManager is constructed");
}
SceneManager::~SceneManager() {
  spdlog::trace("SceneManager is deconstructed");
  close(); // 即使不手动调用close也能确保清理
}

Scene *SceneManager::getCurrentScene() const {
  if (scene_stack_.empty()) {
    return nullptr;
  }
  return scene_stack_.back().get();
}
void SceneManager::update(float delta_time) {
  Scene *current_scene = getCurrentScene();
  if (current_scene) {
    // 只更新栈顶的scene,下层的不更新相当于暂停
    current_scene->update(delta_time);
  }
  processPendingActions();
}
void SceneManager::render() {
  for (const auto &scene : scene_stack_) {
    if (scene) {
      scene->render();
    }
  }
}
void SceneManager::handleInput() {
  Scene *current_scene = getCurrentScene();
  if (current_scene) {
    current_scene->handleInput();
  }
}
void SceneManager::close() {
  spdlog::trace("Closing SceneManager && cleanning scene stack");
  while (!scene_stack_.empty()) {
    if (scene_stack_.back()) {
      spdlog::debug("Cleanning scene '{}'", scene_stack_.back()->getName());
      scene_stack_.back()->clean();
    }
    scene_stack_.pop_back();
  }
}
void SceneManager::requestPopScene() { pending_action_ = PendingAction::Pop; }
void SceneManager::requestPushScene(std::unique_ptr<Scene> &&scene) {
  pending_action_ = PendingAction::Push;
  pending_scene_ = std::move(scene);
}
void SceneManager::requestReplaceScene(std::unique_ptr<Scene> &&scene) {
  pending_action_ = PendingAction::Replace;
  pending_scene_ = std::move(scene);
}
void SceneManager::processPendingActions() {

  switch (pending_action_) {
  case PendingAction::Pop: {
    popScene();
    break;
  }
  case PendingAction::Push: {
    pushScene(std::move(pending_scene_));
    break;
  }
  case PendingAction::Replace: {
    replaceScene(std::move(pending_scene_));
    break;
  }
  case PendingAction::None:
    break;
  default:
    break;
  }
  pending_action_ = PendingAction::None;
}
void SceneManager::pushScene(std::unique_ptr<Scene> &&scene) {
  if (!scene) {
    spdlog::warn("Trying to push EMPTY scene into stack");
    return;
  }
  spdlog::debug("Pushing scene '{}' into stack", scene->getName());

  if (!scene->isInitialized()) {
    scene->init();
  }

  scene_stack_.push_back(std::move(scene));
}
void SceneManager::popScene() {
  if (scene_stack_.empty()) {
    spdlog::warn("Trying to pop scene from EMPTY stack");
    return;
  }
  spdlog::debug("Popping scene '{}' from stack",
                scene_stack_.back()->getName());

  if (scene_stack_.back()) {
    scene_stack_.back()->clean();
  }
  scene_stack_.pop_back();
}
void SceneManager::replaceScene(std::unique_ptr<Scene> &&scene) {
  if (!scene) {
    spdlog::warn("Trying to use EMPTY scene to replace stack");
    return;
  }
  if (!scene->isInitialized()) {
    scene->init();
  }
  // 这里是不是得判断一下scene_stack是否为空呢
  if (scene_stack_.empty()) {
    spdlog::debug("Scene stack is empty, replacing by new scene '{}'",
                  scene->getName());
  } else {
    spdlog::debug("Replacing scene '{}' by new scene '{}'",
                  scene_stack_.back()->getName(), scene->getName());

    while (!scene_stack_.empty()) {
      if (scene_stack_.back()) {
        scene_stack_.back()->clean();
      }
      scene_stack_.pop_back();
    }
  }

  scene_stack_.push_back(std::move(scene));
}
} // namespace engine::scene
