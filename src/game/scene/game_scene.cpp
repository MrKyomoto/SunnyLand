#include "game_scene.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/object/game_object.h"
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
  createTestObject();

  Scene::init();
  spdlog::trace("GameScene is initialized");
}

void GameScene::update(float delta_time) { Scene::update(delta_time); }
void GameScene::render() { Scene::render(); }
void GameScene::handleInput() { Scene::handleInput(); }
void GameScene::clean() { Scene::clean(); }
void GameScene::createTestObject() {
  spdlog::trace("Creating test_object in GameScene... ");
  auto test_object =
      std::make_unique<engine::object::GameObject>("test_object");
  test_object->addComponent<engine::component::TransformComponent>(
      glm::vec2(100.0f, 100.0f));
  test_object->addComponent<engine::component::SpriteComponent>(
      "assets/textures/Actors/foxy.png", context_.getResourceManager());

  addGameObject(std::move(test_object));
  spdlog::trace("test_object is created and added into GameScene");
}
} // namespace game::scene
