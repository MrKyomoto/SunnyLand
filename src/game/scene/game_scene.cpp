#include "game_scene.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/object/game_object.h"
#include "../../engine/physics/physics_engine.h"
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

  auto *main_layer = findGameObjectByName("main");
  if (main_layer) {
    auto *tile_layer =
        main_layer->getComponent<engine::component::TileLayerComponent>();
    if (tile_layer) {
      context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
      spdlog::info("注册 main 层到物理引擎");
    }
  }

  createTestObject();

  Scene::init();
  spdlog::trace("GameScene is initialized");
}

void GameScene::update(float delta_time) { Scene::update(delta_time); }
void GameScene::render() { Scene::render(); }
void GameScene::handleInput() {
  Scene::handleInput();
  // testCamera();
  testObject();
  testCollisionPairs();
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

void GameScene::createTestObject() {
  spdlog::trace("在 GameScene 中创建 test_object...");
  auto test_object =
      std::make_unique<engine::object::GameObject>("test_object");
  test_object_ = test_object.get();

  test_object_->addComponent<engine::component::TransformComponent>(
      glm::vec2(100.0f, 60.0f));
  test_object_->addComponent<engine::component::SpriteComponent>(
      "assets/textures/Props/big-crate.png", context_.getResourceManager());
  test_object_->addComponent<engine::component::PhysicsComponent>(
      &context_.getPhysicsEngine());
  test_object_->addComponent<engine::component::ColliderComponent>(
      std::make_unique<engine::physics::AABBCollider>(glm::vec2(32.0f,32.0f)));

  addGameObject(std::move(test_object));
  spdlog::trace("test_object 创建并添加到 GameScene中");

  auto test_object2 =
      std::make_unique<engine::object::GameObject>("test_object2");

  test_object2->addComponent<engine::component::TransformComponent>(
      glm::vec2(50.0f, 50.0f));
  test_object2->addComponent<engine::component::SpriteComponent>(
      "assets/textures/Props/big-crate.png", context_.getResourceManager());
  test_object2->addComponent<engine::component::PhysicsComponent>(
      &context_.getPhysicsEngine(), false);
  test_object2->addComponent<engine::component::ColliderComponent>(
      std::make_unique<engine::physics::AABBCollider>(glm::vec2(32.0f, 32.0f)));

  addGameObject(std::move(test_object2));
  spdlog::trace("test_object2 added into GameScene");
}

void GameScene::testObject() {
  if (!test_object_)
    return;
  auto &input_manager = context_.getInputManager();
  auto* pc = test_object_->getComponent<engine::component::PhysicsComponent>();
  if(!pc) return;
  if (input_manager.isActionDown("move_left")) {
    pc->velocity_.x = -100.0f;
  } else {
    pc->velocity_.x *= 0.9f;
  }

  if (input_manager.isActionDown("move_right")) {
    pc->velocity_.x = 100.0f;
  } else {
    pc->velocity_.x *= 0.9f;
  }

  if (input_manager.isActionDown("jump")) {
    pc->velocity_.y = -400.0f;
  }
}

void GameScene::testCollisionPairs() {
  auto collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
  for (auto &pair : collision_pairs) {
    spdlog::info("碰撞对 {} 和 {}", pair.first->getName(),
                 pair.second->getName());
  }
}

} // namespace game::scene
