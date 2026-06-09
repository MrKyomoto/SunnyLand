#include "game_scene.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/health_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/object/game_object.h"
#include "../../engine/physics/physics_engine.h"
#include "../../engine/render/animation.h"
#include "../../engine/render/camera.h"
#include "../../engine/scene/level_loader.h"
#include "../component/player_component.h"
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

  if (is_initialized_) {
    spdlog::warn("GameScene 已经初始化过了,重复调用init()");
    return;
  }

  spdlog::trace("GameScene 初始化开始...");

  if (!initLevel()) {
    spdlog::error("关卡初始化失败,无法继续");
    context_.getInputManager().setShouldQuit(true);
    return;
  }
  if (!initPlayer()) {
    spdlog::error("玩家初始化失败,无法继续");
    context_.getInputManager().setShouldQuit(true);
    return;
  }
  if (!initEnemyAndItem()) {
    spdlog::error("敌人和道具初始化失败,无法继续");
    context_.getInputManager().setShouldQuit(true);
    return;
  }
  Scene::init();
  spdlog::trace("GameScene is initialized");
}

bool GameScene::initLevel() {
  // Level Loader通常加载完后即可销毁,因此不存为成员变量
  engine::scene::LevelLoader level_loader;
  if (!level_loader.loadLevel("assets/maps/level1.tmj", *this)) {
    spdlog::error("关卡加载失败");
    return false;
  }

  auto *main_layer = findGameObjectByName("main");
  if (!main_layer) {
    spdlog::error("未找到 main 层");
    return false;
  }
  auto *tile_layer =
      main_layer->getComponent<engine::component::TileLayerComponent>();

  if (!tile_layer) {
    spdlog::error("main 层没有 TileLayerComponent 组件");
    return false;
  }
  context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
  spdlog::info("注册 main 层到物理引擎");

  auto world_size =
      main_layer->getComponent<engine::component::TileLayerComponent>()
          ->getWorldSize();
  context_.getCamera().setLimitBounds(
      engine::utils::Rect(glm::vec2(0.0f), world_size));

  context_.getPhysicsEngine().setWorldBounds(
      engine::utils::Rect(glm::vec2(0.0f), world_size));
  return true;
}

bool GameScene::initPlayer() {
  player_ = findGameObjectByName("player");
  if (!player_) {
    spdlog::error("未找到玩家对象");
    return false;
  }

  auto *player_component =
      player_->addComponent<game::component::PlayerComponent>();
  if (!player_component) {
    spdlog::error("无法添加 PlayerComponent到玩家对象");
    return false;
  }

  auto *player_transform =
      player_->getComponent<engine::component::TransformComponent>();
  if (!player_transform) {
    spdlog::error("玩家对象缺少 TransformComponent组件");
    return false;
  }
  context_.getCamera().setTarget(player_transform);

  spdlog::trace("玩家初始化完成");
  return true;
}

bool GameScene::initEnemyAndItem() {
  bool success = true;
  for (auto &game_object : game_objects_) {
    if (game_object->getName() == "eagle") {
      if (auto *ac =
              game_object
                  ->getComponent<engine::component::AnimationComponent>();
          ac) {
        ac->playAnimation("fly");
      } else {
        spdlog::error("Eagle 对象缺少 AnimationComponent,无法播放动画");
        success = false;
      }
      continue;
    }

    if (game_object->getName() == "frog") {
      if (auto *ac =
              game_object
                  ->getComponent<engine::component::AnimationComponent>();
          ac) {
        ac->playAnimation("idle");
      } else {
        spdlog::error("Frog 对象缺少 AnimationComponent,无法播放动画");
        success = false;
      }
      continue;
    }
    if (game_object->getName() == "opossum") {
      if (auto *ac =
              game_object
                  ->getComponent<engine::component::AnimationComponent>();
          ac) {
        ac->playAnimation("walk");
      } else {
        spdlog::error("Opossum 对象缺少 AnimationComponent,无法播放动画");
        success = false;
      }
      continue;
    }
    // 物品有同样的item标签所以可以通过这个获取
    if (game_object->getTag() == "item") {
      if (auto *ac =
              game_object
                  ->getComponent<engine::component::AnimationComponent>();
          ac) {
        ac->playAnimation("idle");
      } else {
        spdlog::error("Item 对象缺少 AnimationComponent,无法播放动画");
        success = false;
      }
      continue;
    }
  }

  return success;
}

void GameScene::update(float delta_time) {
  Scene::update(delta_time);
  handleObjectCollisions();
  handleTileTriggers();
}

void GameScene::render() { Scene::render(); }
void GameScene::handleInput() {
  Scene::handleInput();
  // testCollisionPairs();
  testHealth();
}
void GameScene::clean() { Scene::clean(); }

void GameScene::testCollisionPairs() {
  auto collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
  for (auto &pair : collision_pairs) {
    spdlog::info("碰撞对 {} 和 {}", pair.first->getName(),
                 pair.second->getName());
  }
}

void GameScene::handleObjectCollisions() {
  auto collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
  for (const auto &pair : collision_pairs) {
    auto *obj1 = pair.first;
    auto *obj2 = pair.second;

    if (obj1->getName() == "player" && obj2->getTag() == "enemy") {
      PlayerVSEnemyCollision(obj1, obj2);
    } else if (obj2->getName() == "player" && obj1->getTag() == "enemy") {
      PlayerVSEnemyCollision(obj2, obj1);
    } else if (obj1->getName() == "player" && obj2->getTag() == "item") {
      PlayerVSItem(obj1, obj2);
    } else if (obj2->getName() == "player" && obj1->getTag() == "item") {
      PlayerVSItem(obj2, obj1);
    } else if (obj1->getName() == "player" && obj2->getTag() == "hazard") {
      obj1->getComponent<game::component::PlayerComponent>()->takeDamage(1);
      spdlog::debug("Player {} 收到了 Hazard 对象伤害",obj1->getName());
    } else if (obj2->getName() == "player" && obj1->getTag() == "hazard") {
      obj2->getComponent<game::component::PlayerComponent>()->takeDamage(1);
      spdlog::debug("Player {} 收到了 Hazard 对象伤害",obj2->getName());
    }
  }
}

void GameScene::PlayerVSEnemyCollision(engine::object::GameObject *player,
                                       engine::object::GameObject *enemy) {
  auto player_aabb =
      player->getComponent<engine::component::ColliderComponent>()
          ->getWorldAABB();
  auto enemy_aabb = enemy->getComponent<engine::component::ColliderComponent>()
                        ->getWorldAABB();
  auto player_center = player_aabb.position + player_aabb.size / 2.0f;
  auto enemy_center = enemy_aabb.position + enemy_aabb.size / 2.0f;

  auto overlap = glm::vec2(player_aabb.size / 2.0f + enemy_aabb.size / 2.0f) -
                 glm::abs(player_center - enemy_center);

  // NOTE: 踩头才算有效攻击
  if (overlap.x > overlap.y && player_center.y < enemy_center.y) {
    spdlog::info("Player {} 踩塌了 Enemy {}", player->getName(),
                 enemy->getName());
    auto enemy_health =
        enemy->getComponent<engine::component::HealthComponent>();
    if (!enemy_health) {
      spdlog::error("Enemy {} 缺少 HealthComponent 组件", enemy->getName());
      return;
    }

    enemy_health->takeDamage(1);
    if (!enemy_health->isAlive()) {
      enemy->setNeedRemove(true);
      createEffect(enemy_center, enemy->getTag());
    }

    // 反弹跳跃效果
    player->getComponent<engine::component::PhysicsComponent>()->velocity_.y =
        -300.0f;
  } else {
    // NOTE: 踩踏判断失败,玩家受伤
    player->getComponent<game::component::PlayerComponent>()->takeDamage(1);
  }
}

void GameScene::PlayerVSItem(engine::object::GameObject *player,
                             engine::object::GameObject *item) {
  if (item->getName() == "fruit") {
    player->getComponent<engine::component::HealthComponent>()->heal(1);
  } else if (item->getName() == "gem") {
    // TODO: score++
  }

  item->setNeedRemove(true);
  auto item_aabb = item->getComponent<engine::component::ColliderComponent>()
                       ->getWorldAABB();
  createEffect(item_aabb.position + item_aabb.size / 2.0f, item->getTag());
}

void GameScene::createEffect(const glm::vec2 &center_pos,
                             const std::string &tag) {
  auto effect_obj =
      std::make_unique<engine::object::GameObject>("effect_" + tag);
  effect_obj->addComponent<engine::component::TransformComponent>(center_pos);

  auto animation = std::make_unique<engine::render::Animation>("effect", false);
  if (tag == "enemy") {
    effect_obj->addComponent<engine::component::SpriteComponent>(
        "assets/textures/FX/enemy-death.png", context_.getResourceManager(),
        engine::utils::Alignment::CENTER);

    for (int i = 0; i < 5; i++) {
      animation->addFrame({static_cast<float>(i * 40), 0.0f, 40.0f, 41.0f},
                          0.1f);
    }
  } else if (tag == "item") {
    effect_obj->addComponent<engine::component::SpriteComponent>(
        "assets/textures/FX/item-feedback.png", context_.getResourceManager(),
        engine::utils::Alignment::CENTER);

    for (int i = 0; i < 4; i++) {
      animation->addFrame({static_cast<float>(i * 32), 0.0f, 32.0f, 32.0f},
                          0.1f);
    }
  } else {
    return;
  }

  auto *animation_component =
      effect_obj->addComponent<engine::component::AnimationComponent>();
  animation_component->addAnimation(std::move(animation));
  animation_component->setOneShotRemoval(true);
  animation_component->playAnimation("effect");

  safeAddGameObject(std::move(effect_obj));
}

void GameScene::handleTileTriggers() {
  const auto &tile_trigger_events =
      context_.getPhysicsEngine().getTileTriggerEvents();
  for (const auto &event : tile_trigger_events) {
    auto *obj = event.first;
    auto tile_type = event.second;
    if (tile_type == engine::component::TileType::HAZARD) {
      if (obj->getName() == "player") {
        obj->getComponent<game::component::PlayerComponent>()->takeDamage(1);
      }
    }
  }
}

void GameScene::testHealth() {
  auto input_manager = context_.getInputManager();
  if (input_manager.isActionPressed("attack")) {
    player_->getComponent<game::component::PlayerComponent>()->takeDamage(1);
  }
}

} // namespace game::scene
