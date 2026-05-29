#include "game_scene.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/transform_component.h"
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
  player_ = findGameObjectByName("player");
  if (!player_) {
    spdlog::error("未找到玩家对象");
    return;
  }

  auto* player_transform = player_->getComponent<engine::component::TransformComponent>();
  if (player_transform) {
    context_.getCamera().setTarget(player_transform);
  }

  auto world_size = main_layer->getComponent<engine::component::TileLayerComponent>()->getWorldSize();
  context_.getCamera().setLimitBounds(engine::utils::Rect(glm::vec2(0.0f),world_size));

  context_.getPhysicsEngine().setWorldBounds(engine::utils::Rect(glm::vec2(0.0f),world_size));

  Scene::init();
  spdlog::trace("GameScene is initialized");
}

void GameScene::update(float delta_time) { Scene::update(delta_time); }
void GameScene::render() { Scene::render(); }
void GameScene::handleInput() {
  Scene::handleInput();
  testPlayer();
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

void GameScene::testPlayer() {
  if (!player_)
    return;
  auto &input_manager = context_.getInputManager();
  auto *pc = player_->getComponent<engine::component::PhysicsComponent>();
  if (!pc)
    return;
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
