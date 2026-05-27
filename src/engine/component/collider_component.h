#pragma once
#include "../physics/collider.h"
#include "../utils/alignment.h"
#include "../utils/math.h"
#include "component.h"
#include <memory>

namespace engine::component {
class TransformComponent;
}

namespace engine::component {
class ColliderComponent final : public Component {
  friend class engine::object::GameObject;

private:
  TransformComponent *transform_ = nullptr;
  std::unique_ptr<engine::physics::Collider> collider_;
  // 碰撞器(最小包围盒)的左上角相对于变换原点的偏移量
  glm::vec2 offset_ = {0.0f, 0.0f};
  engine::utils::Alignment alignment_ = engine::utils::Alignment::NONE;

  bool is_trigger_ = false; // 是否为触发器(仅检测碰撞,不产生物理响应)
  bool is_active_ = true;   // 是否激活

public:
  /// @brief
  /// 这里存的是智能指针collider,与SpriteComponent不同,注意这里需要转移所有权,两种方法都是可以的,只是这里采用了智能指针
  explicit ColliderComponent(
      std::unique_ptr<engine::physics::Collider> collider,
      engine::utils::Alignment alignment = engine::utils::Alignment::NONE,
      bool is_trigger = false, bool is_active = true);

  void updateOffset();

  TransformComponent *getTransform() const { return transform_; }
  const engine::physics::Collider *getCollider() const {
    return collider_.get();
  }
  const glm::vec2 &getOffset() const { return offset_; }
  engine::utils::Alignment getAlignment() const { return alignment_; }
  /// @brief 获取世界坐标系啊的最小轴对齐包装盒
  engine::utils::Rect getWorldAABB() const;
  bool isTrigger() const { return is_trigger_; }
  bool isActive() const { return is_active_; }

  /// @brief 设置新的对齐方式并重新计算偏移量
  void setAlignment(engine::utils::Alignment anchor);
  void setOffset(const glm::vec2& offset){offset_ = offset;}
  void setTrigger(bool is_trigger){is_trigger_ = is_trigger;}
  void setActive(bool is_active){is_active_ = is_active;}

private:
  void init() override;
  void update(float,engine::core::Context&) override {}
};

} // namespace engine::component