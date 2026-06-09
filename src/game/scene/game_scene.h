#pragma once
#include "../../engine/scene/scene.h"
#include <glm/vec2.hpp>
#include <memory>

namespace engine::object {
class GameObject;
}
namespace game::scene {

/**
 * @brief 主要的游戏场景，包含玩家，敌人，关卡元素
 */
class GameScene final : public engine::scene::Scene {
  engine::object::GameObject *player_ = nullptr;

public:
  GameScene(std::string name, engine::core::Context &context,
            engine::scene::SceneManager &scene_manager);

  void init() override;
  void update(float delta_time) override;
  void render() override;
  void handleInput() override;
  void clean() override;

private:
  void testCollisionPairs();

  [[nodiscard]] bool initLevel();
  [[nodiscard]] bool initPlayer();
  [[nodiscard]] bool initEnemyAndItem();

  void handleObjectCollisions();
  void PlayerVSEnemyCollision(engine::object::GameObject *player,
                              engine::object::GameObject *enemy);
  void PlayerVSItem(engine::object::GameObject *player,
                    engine::object::GameObject *item);

  /**
   * @brief 创建一个一次性特效对象
   * @param center_pos 特效中心位置
   * @param tag 特效标签,决定特效类型("enemy","item")
   */
  void createEffect(const glm::vec2 &center_pos, const std::string &tag);

  void testHealth();
};

} // namespace game::scene
