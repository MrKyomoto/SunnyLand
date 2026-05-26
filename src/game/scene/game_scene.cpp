#include "game_scene.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/object/game_object.h"
#include "../../engine/render/camera.h"
#include "../../engine/scene/level_loader.h"
#include <SDL3/SDL_rect.h>
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace game::scene {

GameScene::GameScene(std::string name, engine::core::Context &context,
                     engine::scene::SceneManager &scene_manager)
    : engine::scene::Scene(name, context, scene_manager) {
  spdlog::trace("GameScene is constructed");
}
void GameScene::init() {
  // Level Loader通常加载完后即可销毁,因此不存为成员变量
  engine::scene::LevelLoader level_loader;
  level_loader.loadLevel("assets/maps/level1.tmj", *this);

  Scene::init();
  spdlog::trace("GameScene is initialized");
}

void GameScene::update(float delta_time) { Scene::update(delta_time); }
void GameScene::render() { Scene::render(); }
void GameScene::handleInput() {
  Scene::handleInput();
  testCamera();
}
void GameScene::clean() { Scene::clean(); }

void GameScene::testCamera() {
  auto &camera = context_.getCamera();
  auto &input_manager = context_.getInputManager();
  if (input_manager.isActionDown("move_up"))
    camera.move(glm::vec2(0, -1));
  if (input_manager.isActionDown("move_down"))
    camera.move(glm::vec2(0, 1));
  if (input_manager.isActionDown("move_left"))
    camera.move(glm::vec2(-1, 0));
  if (input_manager.isActionDown("move_right"))
    camera.move(glm::vec2(1, 0));
}

} // namespace game::scene
