#pragma once
#include "../../engine/scene/scene.h"
#include <memory>

namespace engine::object {
class GameObject;
}
namespace game::scene {

/**
 * @brief 主要的游戏场景，包含玩家，敌人，关卡元素
 */
class GameScene final : public engine::scene::Scene {
public:
  GameScene(std::string name, engine::core::Context &context,
            engine::scene::SceneManager &scene_manager);

  void init() override;
  void update(float delta_time) override;
  void render() override;
  void handleInput() override;
  void clean() override;

private:
  void createTestObject();
};
} // namespace game::scene
